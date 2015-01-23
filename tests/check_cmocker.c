/*
 * Copyright (C) 2015 Sylvain Afchain
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if
 * not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <check.h>
#include <stdarg.h>

#include <cmocker.h>

START_TEST(test_called)
{
  mock_called("test1");
  ck_assert_int_eq(1, mock_calls("test1"));

  mock_called("test1");
  ck_assert_int_eq(2, mock_calls("test1"));

  mock_called("test2");
  ck_assert_int_eq(1, mock_calls("test2"));

  ck_assert(0 == mock_calls("test3"));

  mock_reset_calls_for("test2");
  ck_assert_int_eq(0, mock_calls("test2"));
  ck_assert_int_eq(2, mock_calls("test1"));

  mock_reset_calls();
  ck_assert_int_eq(0, mock_calls("test2"));
  ck_assert_int_eq(0, mock_calls("test1"));
}
END_TEST

START_TEST(test_called_with)
{
  int arg1 = 44, arg2 = 45, exp_arg;

  mock_called_with("test1", &arg1);
  ck_assert_int_eq(1, mock_calls("test1"));
  exp_arg = *(int *)mock_call("test1", 0);
  ck_assert_int_eq(44, exp_arg);

  mock_called_with("test1", &arg2);
  ck_assert_int_eq(2, mock_calls("test1"));
  exp_arg = *(int *)mock_call("test1", 1);
  ck_assert_int_eq(45, exp_arg);

  ck_assert(NULL == mock_call("test1", 2));
}
END_TEST

START_TEST(test_returns)
{
  int i = 44, exp_i;
  char *s = "test", *exp_s;

  mock_will_return("test1", &i, MOCK_RETURNED_ONCE);

  ck_assert(NULL == mock_returns("test2"));

  exp_i = *((int *)mock_returns("test1"));
  ck_assert_int_eq(44, exp_i);

  ck_assert(NULL == mock_returns("test1"));

  mock_will_return("test1", &i, MOCK_RETURNED_ALWAYS);
  exp_i = *((int *)mock_returns("test1"));
  ck_assert_int_eq(44, exp_i);

  exp_i = *((int *)mock_returns("test1"));
  ck_assert_int_eq(44, exp_i);

  mock_will_return("test2", s, MOCK_RETURNED_ONCE);
  exp_s = (char *)mock_returns("test2");
  ck_assert_str_eq(s, exp_s);
}
END_TEST

static void *returns_fnc(va_list *pa)
{
  static int i = 0;

  if (i == 0) {
    i = 44;
  } else {
    i++;
  }

  return &i;
}

START_TEST(test_returns_fnc)
{
  int exp_i;

  mock_will_return("test1", returns_fnc, MOCK_RETURNED_FNC);

  exp_i = *((int *)mock_returns("test1"));
  ck_assert_int_eq(44, exp_i);

  exp_i = *((int *)mock_returns("test1"));
  ck_assert_int_eq(45, exp_i);

  exp_i = *((int *)mock_returns("test1"));
  ck_assert_int_eq(46, exp_i);
}
END_TEST

static void *returns_fnc_pa(va_list *pa)
{
  static int i = 44;

  i += va_arg(*pa, int);

  return &i;
}

START_TEST(test_returns_fnc_pa)
{
  int exp_i;

  mock_will_return("test1", returns_fnc_pa, MOCK_RETURNED_FNC);

  exp_i = *((int *)mock_returns("test1", 2));
  ck_assert_int_eq(46, exp_i);

  exp_i = *((int *)mock_returns("test1", 3));
  ck_assert_int_eq(49, exp_i);
}
END_TEST

void test_setup()
{
  mock_init();
}

void test_teardown()
{
  mock_destroy();
}

Suite *cmocker_suite(void)
{
  Suite *s;
  TCase *tc_cmocker;

  s = suite_create("cmocker");
  tc_cmocker = tcase_create("cmocker");

  tcase_add_checked_fixture(tc_cmocker, test_setup, test_teardown);

  tcase_add_test(tc_cmocker, test_called);
  tcase_add_test(tc_cmocker, test_called_with);
  tcase_add_test(tc_cmocker, test_returns);
  tcase_add_test(tc_cmocker, test_returns_fnc);
  tcase_add_test(tc_cmocker, test_returns_fnc_pa);

  suite_add_tcase(s, tc_cmocker);

  return s;
}

int main(void)
{
  Suite *s;
  SRunner *sr;
  int number_failed;
  int rc;

  sr = srunner_create(cmocker_suite());
  srunner_run_all(sr, CK_VERBOSE);

  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
