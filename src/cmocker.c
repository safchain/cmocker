/*
 * Copyright 2015 Sylvain Afchain <safchain@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <assert.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>

#include "cmocker.h"

#if HAVE_CONFIG_H
#include "config.h"
#endif

struct hmap {
  struct hnode *nodes;
  unsigned int hsize;
};

struct hnode {
  char *key;
  void *value;
  struct hnode *next;
};

struct hmap_iterator {
  struct hmap *hmap;
  unsigned int index;
  struct hnode *current;
};

struct list {
  struct list_node *nodes;
  struct list_node *last;
};

struct list_node {
  struct list_node *prev;
  struct list_node *next;
};

struct list_iterator {
  struct list_node *current;
};

struct returned {
  void *value;
  int type;
};

struct called {
  int calls;
  struct list *list;
};

static struct hmap *returns = NULL;
static struct hmap *calls = NULL;

#ifdef HAVE_LIBPTHREAD
static pthread_mutex_t call_mutex;
#endif

static struct list *list_alloc()
{
  struct list *list;

  list = calloc(1, sizeof(struct list));
  assert(list != NULL);

  return list;
}

static void list_push(struct list *list, void *value, unsigned int size)
{
  struct list_node *node_ptr;

  node_ptr = malloc(sizeof(struct list_node) + size);
  assert(node_ptr != NULL);

  node_ptr->next = NULL;

  if (list->nodes == NULL) {
    list->nodes = node_ptr;
    node_ptr->prev = NULL;
  } else {
    list->last->next = node_ptr;
    node_ptr->prev = list->last;
  }
  list->last = node_ptr;

  memcpy((char *) node_ptr + sizeof(struct list_node), value, size);
}

static void *list_get(struct list *list, unsigned int index)
{
  struct list_node *node_ptr = list->nodes;
  unsigned int i = 0;

  while (node_ptr != NULL) {
    if (i++ == index) {
      return ((char *) node_ptr + sizeof(struct list_node));
    }
    node_ptr = node_ptr->next;
  }

  return NULL;
}

static void list_reset(struct list *list)
{
  struct list_node *next_ptr;

  while (list->nodes != NULL) {
    next_ptr = list->nodes->next;
    free(list->nodes);
    list->nodes = next_ptr;
  }
  list->last = NULL;
}

static void list_free(struct list *list)
{
  list_reset(list);
  free(list);
}

static void _free0(void **ptr)
{
  free(*ptr);
  *ptr = NULL;
}

static struct hmap *hmap_alloc(unsigned int size)
{
  struct hmap *hmap;

  hmap = malloc(sizeof(struct hmap));
  assert(hmap != NULL);

  hmap->nodes = calloc(size, sizeof(struct hnode));
  assert(hmap->nodes != NULL);

  hmap->hsize = size;

  return hmap;
}

static unsigned int hmap_get_key(struct hmap *hmap, const char *key)
{
  unsigned int hash = 0;
  const char *c = key;

  while(*c != '\0') {
    hash += *c++;
    hash += (hash << 10);
    hash ^= (hash >> 6);
  }

  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);

  return hash % hmap->hsize;
}

static struct hnode *hmap_put(struct hmap *hmap, const char *key,
                              const void *value, unsigned int size)
{
  struct hnode *hnode_ptr = hmap->nodes;
  unsigned int index = 0;

  index = hmap_get_key(hmap, key);

  hnode_ptr = (struct hnode *) (hnode_ptr + index);
  do {
    if (hnode_ptr->key == NULL) {
      hnode_ptr->key = strdup(key);
      assert(hnode_ptr->key != NULL);

      hnode_ptr->value = malloc(size);
      assert(hnode_ptr != NULL);

      memcpy(hnode_ptr->value, value, size);

      break;
    }

    if (strcmp(key, hnode_ptr->key) == 0) {
      free(hnode_ptr->value);

      hnode_ptr->value = malloc(size);
      assert(hnode_ptr != NULL);

      memcpy(hnode_ptr->value, value, size);

      break;
    }

    if (hnode_ptr->next == NULL) {
      hnode_ptr->next = calloc(1, sizeof(struct hnode));
      assert(hnode_ptr->next != NULL);
    }

    hnode_ptr = hnode_ptr->next;
  } while (hnode_ptr != NULL);

  return hnode_ptr;
}

static struct hnode *hmap_get_node(struct hmap *hmap, const char *key)
{
  struct hnode *hnode_ptr = hmap->nodes;
  unsigned int index = 0;

  index = hmap_get_key(hmap, key);

  hnode_ptr = (struct hnode *) (hnode_ptr + index);
  do {
    if (hnode_ptr->key != NULL) {
      if (strcmp(key, hnode_ptr->key) == 0) {
        return hnode_ptr;
      }
    }

    hnode_ptr = hnode_ptr->next;
  } while (hnode_ptr != NULL);

  return NULL;
}


static void *hmap_get(struct hmap *hmap, const char *key)
{
  struct hnode *hnode_ptr = hmap_get_node(hmap, key);
  if (hnode_ptr != NULL) {
    return hnode_ptr->value;
  }

  return NULL;
}

static void hmap_reset(struct hmap *hmap)
{
  struct hnode *hnodes_ptr;
  struct hnode *hnode_ptr;
  struct hnode *next_ptr;
  unsigned int size = hmap->hsize;
  unsigned int i;

  hnodes_ptr = hmap->nodes;

  for (i = 0; i != size; i++) {
    hnode_ptr = ((struct hnode *) hnodes_ptr) + i;

    if (hnode_ptr->key == NULL) {
      continue;
    }

    _free0((void *) &hnode_ptr->key);
    free(hnode_ptr->value);

    next_ptr = hnode_ptr->next;
    hnode_ptr->next = NULL;

    hnode_ptr = next_ptr;
    while (hnode_ptr != NULL) {
      free(hnode_ptr->key);
      free(hnode_ptr->value);

      next_ptr = hnode_ptr->next;
      free(hnode_ptr);

      hnode_ptr = next_ptr;
    }
  }
}

static void hmap_init_iterator(struct hmap *hmap,
                               struct hmap_iterator *iterator)
{
  iterator->hmap = hmap;
  iterator->index = 0;
  iterator->current = NULL;
}

static struct hnode *hmap_iterate(struct hmap_iterator *iterator)
{
  struct hnode *hnodes_ptr;
  struct hnode *hnode_ptr;
  unsigned int size;

  size = iterator->hmap->hsize;
  hnodes_ptr = iterator->hmap->nodes;
  while (iterator->index != size) {
    if (iterator->current != NULL) {
      hnode_ptr = iterator->current;
      iterator->current = NULL;
    } else {
      hnode_ptr = (struct hnode *) (hnodes_ptr + iterator->index++);
    }

    if (hnode_ptr->key != NULL) {
      iterator->current = hnode_ptr->next;
      return hnode_ptr;
    }
  }

  return NULL;
}

static void hmap_free_node(struct hnode *hnode)
{
  _free0((void *) &hnode->key);
  free(hnode->value);
}

static int hmap_del(struct hmap *hmap, const char *key)
{
  struct hnode *hnode_ptr = hmap_get_node(hmap, key);
  if (hnode_ptr != NULL) {
    hmap_free_node(hnode_ptr);
    return 1;
  }

  return 0;
}

static void hmap_free(struct hmap *hmap)
{
  hmap_reset(hmap);
  free(hmap->nodes);
  free(hmap);
}

void mock_init()
{
  returns = hmap_alloc(32);
  assert(returns != NULL);

  calls = hmap_alloc(32);
  assert(calls != NULL);

#ifdef HAVE_LIBPTHREAD
  pthread_mutex_init(&call_mutex, NULL);
#endif
}

void mock_destroy()
{
  hmap_free(returns);
  returns = NULL;

  mock_reset_calls();

  hmap_free(calls);
  calls = NULL;

#ifdef HAVE_LIBPTHREAD
  pthread_mutex_destroy(&call_mutex);
#endif
}

void mock_will_return(const char *fnc, void *value, int type)
{
  struct returned *r = hmap_get(returns, fnc);

  if (r == NULL) {
    r = (struct returned *)malloc(sizeof(struct returned));
    assert(r != NULL);
  }

  r->value = value;
  r->type = type;

  hmap_put(returns, fnc, r, sizeof(struct returned));

  free(r);
}

void *mock_returns(const char *fnc, ...)
{
  struct returned *r = hmap_get(returns, fnc);
  void *value;
  va_list pa;

  if (r == NULL) {
    return NULL;
  }
  value = r->value;

  if (r->type == MOCK_RETURNED_ONCE) {
    hmap_del(returns, fnc);
  } else if (r->type == MOCK_RETURNED_FNC) {
    va_start(pa, fnc);
    value = ((void*(*)(va_list *pa))r->value)(&pa);
    va_end(pa);

    return value;
  }

  return value;
}

void mock_called_with(const char *fnc, void *ptr)
{
  struct called *called;
  int rc;

#ifdef HAVE_LIBPTHREAD
  rc = pthread_mutex_lock(&call_mutex);
  assert(rc == 0);
#endif

  called = (struct called *)hmap_get(calls, fnc);
  if (called == NULL) {
    called = (struct called *)calloc(1, sizeof(struct called));
    assert(called != NULL);

    called->list = list_alloc();
    assert(called->list != NULL);

    called->calls = 1;
    hmap_put(calls, fnc, called, sizeof(struct called));
  }
  called->calls++;

  if (ptr != NULL) {
    list_push(called->list, &ptr, sizeof(void *));
  }

#ifdef HAVE_LIBPTHREAD
  rc = pthread_mutex_unlock(&call_mutex);
  assert(rc == 0);
#endif
}

void mock_called(const char *fnc)
{
  mock_called_with(fnc, NULL);
}

void mock_reset_calls_for(const char *fnc)
{
  struct called *called;
  int rc;

#ifdef HAVE_LIBPTHREAD
  rc = pthread_mutex_lock(&call_mutex);
  assert(rc == 0);
#endif

  called = (struct called *)hmap_get(calls, fnc);
  if (called != NULL) {
    list_free(called->list);
  }
  hmap_del(calls, fnc);

#ifdef HAVE_LIBPTHREAD
  rc = pthread_mutex_unlock(&call_mutex);
  assert(rc == 0);
#endif
}

int mock_calls(const char *fnc)
{
  struct called *called;
  int rc, c = 0;

#ifdef HAVE_LIBPTHREAD
  rc = pthread_mutex_lock(&call_mutex);
  assert(rc == 0);
#endif

  called = (struct called *)hmap_get(calls, fnc);
  if (called != NULL) {
    c = called->calls;
  }

#ifdef HAVE_LIBPTHREAD
  rc = pthread_mutex_unlock(&call_mutex);
  assert(rc == 0);
#endif

  return c;
}

void *mock_call(const char *fnc, int at)
{
  struct called *called;
  void **ptr;
  int rc;

#ifdef HAVE_LIBPTHREAD
  rc = pthread_mutex_lock(&call_mutex);
  assert(rc == 0);
#endif

  called = (struct called *)hmap_get(calls, fnc);
  if (called == NULL) {
     goto not_found;
  }

  ptr = list_get(called->list, at);
  if (ptr == NULL) {
      goto not_found;
  }

#ifdef HAVE_LIBPTHREAD
  rc = pthread_mutex_unlock(&call_mutex);
  assert(rc == 0);
#endif

  return *ptr;

not_found:
#ifdef HAVE_LIBPTHREAD
  rc = pthread_mutex_unlock(&call_mutex);
  assert(rc == 0);
#endif

  return NULL;
}

void mock_reset_calls()
{
  struct hmap_iterator iterator;
  struct hnode *hnode;

  hmap_init_iterator(calls, &iterator);
  while((hnode = hmap_iterate(&iterator)) != NULL) {
    list_free(((struct called *)hnode->value)->list);
    hmap_free_node(hnode);
  }
}

int mock_wait_for_call_num_higher_than(const char *fnc, int limit, int timeout)
{
  time_t start, now;
  int called;

  start = now = time(NULL);
  while(start + timeout > now) {
    called = mock_calls(fnc);
    if (called > limit) {
      return 1;
    }
    usleep(100);
    now = time(NULL);
  }

  return 0;
}

int mock_wait_to_be_called(const char *fnc, int timeout)
{
  return mock_wait_for_call_num_higher_than(fnc, 0, timeout);
}
