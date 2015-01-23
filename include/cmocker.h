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
