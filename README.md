[![Coverage Status](https://coveralls.io/repos/safchain/cmocker/badge.svg?branch=master)](https://coveralls.io/r/safchain/cmocker?branch=master)
[![Build Status](https://travis-ci.org/safchain/cmocker.png)](https://travis-ci.org/safchain/cmocker)

cmocker
=======

cmocker is a little C library to help writing unit test especially to implement mock object/function. It can be use with almost all the unit test frameworks. It can be used as a library or just by using the unique C file. There is no external dependencies and it is thread safe.

Installation
----------------

    ./configure
    make
    make check
    sudo make install


API
----

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

Examples (with check)
------------------------------

For this first example let's implement a mock function that check if it has been called. For this example let's assume that you want to check if the do_something function is called when calling the call_something function. The call_something could be like following:

    void call_something()
    {
        ...
        do_something();
        ...
    }

Let's write the test...

    void do_something()
    {
      mock_called("do_something");
    }

    START_TEST(test_called)
    {
      call_something();
      ck_assert_int_eq(1, mock_calls("do_something"));
      call_something();
      ck_assert_int_eq(2, mock_calls("do_something"));
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

    Suite *test_suite(void)
    {
      Suite *s;
      TCase *tc;

      s = suite_create("tests");
      tc = tcase_create("tests");
      tcase_add_checked_fixture(tc, test_setup, test_teardown);
      tcase_add_test(tc, test_called);
      suite_add_tcase(s, tc);

      return s;
    }

    int main(void)
    {
      Suite *s;
      SRunner *sr;
      int number_failed;
      int rc;

      sr = srunner_create(test_suite());
      srunner_run_all(sr, CK_VERBOSE);

      number_failed = srunner_ntests_failed(sr);
      srunner_free(sr);

      return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
    }

We can reset the call counter at any time by using the mock_reset_calls or the mock_reset_calls_for functions.

    START_TEST(test_called)
    {
      call_something();
      ck_assert_int_eq(1, mock_calls("do_something"));

      mock_reset_calls_for("do_something");

      call_something();
      ck_assert_int_eq(1, mock_calls("do_something"));
    }
    END_TEST

We can also add an extra parameter as pointer when specifying that a function has been called in order to add a king of context.

    int v1 = 44, v2 = 45, calls = 0;
    void do_something()
    {
      if (calls++ == 0) {
        mock_called_with("do_something", &v1);
      } else {
        mock_called_with("do_something", &v2);
      }
    }

    START_TEST(test_called)
    {
      int exp_arg;

      call_something();
      call_something();

      /* last arg is the index of the call */
      exp_arg = *(int *)mock_call("do_something", 0);
      ck_assert_int_eq(44, exp_arg);
      exp_arg = *(int *)mock_call("do_something", 1);
      ck_assert_int_eq(45, exp_arg);
    }

Now let's assume we want to specify what will be the value returned by the mocked function. So our prototype would be like as following :

    void call_something()
    {
        ...
        if (do_something()) {
          do_somethingelse();
        }
    }

So let's see how to manage the returned values.

    void do_somethingelse()
    {
      mock_called("do_somethingelse");
    }

    int do_something()
    {
      return *((int *)mock_returns("do_something"));
    }

    START_TEST(test_returns)
    {
      int i = 1;

      mock_will_return("do_something", &i, MOCK_RETURNED_ONCE);
      ck_assert_int_eq(1, mock_calls("do_somethingelse"));

      i = 0;
      mock_will_return("do_something", &i, MOCK_RETURNED_ONCE);
      ck_assert_int_eq(0 mock_calls("do_somethingelse"));
    }
    END_TEST

If we want the a mocked function wil always return the same value we just have to convert the last parameter from MOCK_RETURNED_ONCE to MOCK_RETURNED_ALWAYS.

A function can be used to generate the values that will be returned.

    int i;
    void *return_fnc(va_list *pa)
    {
      i = 44;
      return &i;
    }

    START_TEST(test_returns)
    {
      mock_will_return("do_something", return_fnc,  MOCK_RETURNED_FNC);
      ck_assert_int_eq(1, mock_calls("do_somethingelse"));
    }
    END_TEST

We can use/get the parameters passed from the mocked to the generator. Let's modify the prototype to see that.

    void call_something()
    {
        ...
        if (do_something(2) == 46) {
          do_somethingelse();
        }
    }

So the test will be :

    void do_somethingelse()
    {
      mock_called("do_somethingelse");
    }

    int do_something(int value)
    {
      return *((int *)mock_returns("do_something"), value);
    }

    int i = 44;
    void *return_fnc(va_list *pa)
    {
      i += va_arg(*pa, int);
      return &i;
    }

    START_TEST(test_returns)
    {
      mock_will_return("do_something", return_fnc,  MOCK_RETURNED_FNC);
      ck_assert_int_eq(1, mock_calls("do_somethingelse"));
    }

And that's it !

LICENSE
-----------

Copyright 2015, Sylvain Afchain <safchain@gmail.com>

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
