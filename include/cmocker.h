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

#ifndef MOCK_H_
#define MOCK_H_

enum {
  MOCK_RETURNED_ONCE = 1,
  MOCK_RETURNED_ALWAYS,
  MOCK_RETURNED_FNC
};

void mock_init();
void mock_destroy();
void mock_will_return(const char *fnc, void *value, int type);
void *mock_returns(const char *fnc, ...);
void mock_called(const char *fnc);
void mock_called_with(const char *fnc, void *ptr);
int mock_calls(const char *fnc);
int mock_wait_for_call_num_higher_than(const char *fnc, int limit, int timeout);
int mock_wait_to_be_called(const char *fnc, int timeout);
void mock_reset_calls();
void mock_reset_calls_for(const char *fnc);
void *mock_call(const char *fnc, int at);

#endif
