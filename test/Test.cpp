// ----------------------------------------------------------------------------
// Copyright 2014-2021 Pelion.
//
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------------------------------------------------------
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "gtest/gtest.h"

#define MBED_CONF_MBED_TRACE_ENABLE 1
#define MBED_CONF_MBED_TRACE_FEA_IPV6 1

#include "mbed-trace/mbed_trace.h"
#include "ip6tos_stub.h"

static int mutex_wait_count = 0;
static int mutex_release_count = 0;
static bool check_mutex_lock_status = true;
void my_mutex_wait()
{
    mutex_wait_count++;
}
void my_mutex_release()
{
    mutex_release_count++;
}

char buf[1024];
#include <stdio.h>
void myprint(const char *str)
{
    if (check_mutex_lock_status) {
        ASSERT_TRUE((mutex_wait_count - mutex_release_count) > 0);
    }
    strcpy(buf, str);
}

class trace : public testing::Test
{
    void SetUp(void)
    {
        mbed_trace_init();
        mbed_trace_config_set(TRACE_MODE_PLAIN | TRACE_ACTIVE_LEVEL_ALL);
        mbed_trace_print_function_set(myprint);
        mbed_trace_mutex_wait_function_set(my_mutex_wait);
        mbed_trace_mutex_release_function_set(my_mutex_release);
    }

    void TearDown(void)
    {
        ASSERT_TRUE(mutex_wait_count == mutex_release_count); // Check the mutex count with every test
        mbed_trace_free();
    }
};

/* Unity test code starts */
TEST_F(trace, MutexNotSet)
{
    mbed_trace_mutex_wait_function_set(0);
    mbed_trace_mutex_release_function_set(0);
    int mutex_call_count_at_entry = mutex_wait_count;
    check_mutex_lock_status = false;

    char expectedStr[] = "Hello hello!";
    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "Hello hello!");
    ASSERT_STREQ(expectedStr, buf);

    ASSERT_TRUE(mutex_call_count_at_entry == mutex_wait_count);
    ASSERT_TRUE(mutex_call_count_at_entry == mutex_release_count);

    mbed_trace_mutex_wait_function_set(my_mutex_wait);
    mbed_trace_mutex_release_function_set(my_mutex_release);
    check_mutex_lock_status = true;
}

TEST_F(trace, Array)
{
    unsigned char longStr[200] = {0x66};
    for (int i = 0; i < 200; i++) {
        longStr[i] = 0x66;
    }
    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "%s", mbed_trace_array(longStr, 200));
}

TEST_F(trace, Null0Array)
{
    static const unsigned char array[2] = { 0x23, 0x45 };
    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "%s", mbed_trace_array(array, 2));
    ASSERT_STREQ("23:45", buf);
    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "%s", mbed_trace_array(array, 0));
    ASSERT_STREQ("", buf);
    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "%s", mbed_trace_array(NULL, 0));
    ASSERT_STREQ("", buf);
    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "%s", mbed_trace_array(NULL, 2));
    ASSERT_STREQ("<null>", buf);
}

TEST_F(trace, LongString)
{
    char longStr[1000] = {0x36};
    for (int i = 0; i < 999; i++) {
        longStr[i] = 0x36;
    }
    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "%s", longStr);
}

TEST_F(trace, TooLong)
{
#define TOO_LONG_SIZE 9400
#define TRACE_LINE_SIZE 1024
    char longStr[TOO_LONG_SIZE] = {0};
    for (int i = 0; i < TOO_LONG_SIZE; i++) {
        longStr[i] = 0x36;
    }

    mbed_trace_config_set(TRACE_ACTIVE_LEVEL_ALL);
    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "%s", longStr);

    char shouldStr[TRACE_LINE_SIZE] = "[DBG ][mygr]: ";
    for (int i = 14; i < TRACE_LINE_SIZE; i++) {
        shouldStr[i] = 0x36;
    }
    shouldStr[TRACE_LINE_SIZE - 1] = 0;
    ASSERT_STREQ(shouldStr, buf);
}

TEST_F(trace, BufferResize)
{
    uint8_t arr[20] = {0};
    memset(arr, '0', 20);

    mbed_trace_buffer_sizes(0, 10);
    ASSERT_STREQ("30:30:30*", mbed_trace_array(arr, 20));
    mbed_trace_buffer_sizes(0, 15);
    ASSERT_STREQ("30:30:30:30*", mbed_trace_array(arr, 20));
    mbed_trace_buffer_sizes(0, 15);
    ASSERT_STREQ("30:30:30:30", mbed_trace_array(arr, 4));

    const char *expectedStr = "0123456789";
    mbed_trace_buffer_sizes(11, 0);
    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "01234567890123456789");
    ASSERT_STREQ(expectedStr, buf);
    expectedStr = "012345678901234";
    mbed_trace_buffer_sizes(16, 0);
    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "01234567890123456789");
    ASSERT_STREQ(expectedStr, buf);
    expectedStr = "012345678901234";
    mbed_trace_buffer_sizes(16, 0);
    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "012345678901234");
    ASSERT_STREQ(expectedStr, buf);
}

TEST_F(trace, PreInitConfiguration)
{
    uint8_t arr[20] = {0};
    memset(arr, '0', 20);

    mbed_trace_free();
    mbed_trace_config_set(TRACE_MODE_PLAIN | TRACE_ACTIVE_LEVEL_ALL);
    mbed_trace_print_function_set(myprint);
    mbed_trace_buffer_sizes(11, 10);
    mbed_trace_mutex_wait_function_set(my_mutex_wait);
    mbed_trace_mutex_release_function_set(my_mutex_release);
    mbed_trace_init();

    ASSERT_STREQ("30:30:30*", mbed_trace_array(arr, 20));

    const char *expectedStr = "0123456789";
    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "01234567890123456789");
    ASSERT_STREQ(expectedStr, buf);
}

#if YOTTA_CFG_MBED_TRACE_FEA_IPV6 == 1
ip6tos_stub_def_t ip6tos_stub; // extern variable

TEST_F(trace, ipv6)
{
    uint8_t prefix[] = { 0x14, 0x6e, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00 };
    int prefix_len = 64;

    char expected_str1[] = "146e:a00::/64";
    ip6tos_stub.output_string = "146e:a00::/64";
    char *str = mbed_trace_ipv6_prefix(prefix, prefix_len);
    ASSERT_TRUE(memcmp(ip6tos_stub.input_array, prefix, 8) == 0);
    ASSERT_STREQ(expected_str1, str);
    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "flush buffers and locks");

    char expected_str2[] = "::/0";
    ip6tos_stub.output_string = "::/0";
    str = mbed_trace_ipv6_prefix(NULL, 0);
    ASSERT_STREQ(expected_str2, str);
    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "flush buffers and locks");

    char expected_str3[] = "<err>";
    str = mbed_trace_ipv6_prefix(NULL, 1);
    ASSERT_STREQ(expected_str3, str);
    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "flush buffers and locks");

    char expected_str4[] = "<err>";
    str = mbed_trace_ipv6_prefix(prefix, 200);
    ASSERT_STREQ(expected_str4, str);
    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "flush buffers and locks");

    char expected_str5[] = "";
    ip6tos_stub.output_string = "0123456789012345678901234567890123456789";
    str = mbed_trace_ipv6_prefix(prefix, 64); // Fill the tmp_data buffer
    str = mbed_trace_ipv6_prefix(prefix, 64);
    str = mbed_trace_ipv6_prefix(prefix, 64);
    str = mbed_trace_ipv6_prefix(prefix, 64);
    ASSERT_STREQ(expected_str5, str);
    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "flush buffers and locks");
}

TEST_F(trace, active_level_all_ipv6)
{
    mbed_trace_config_set(TRACE_ACTIVE_LEVEL_ALL);

    uint8_t arr[] = { 0x20, 0x01, 0xd, 0xb8, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1 };
    ip6tos_stub.output_string = "2001:db8::1:0:0:1";
    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "my addr: %s", mbed_trace_ipv6(arr));
    ASSERT_TRUE(memcmp(ip6tos_stub.input_array, arr, 16) == 0);
    ASSERT_STREQ("[DBG ][mygr]: my addr: 2001:db8::1:0:0:1", buf);
}
#endif //YOTTA_CFG_MBED_TRACE_FEA_IPV6

TEST_F(trace, config_change)
{
    mbed_trace_config_set(TRACE_MODE_COLOR | TRACE_ACTIVE_LEVEL_ALL);
    ASSERT_TRUE(mbed_trace_config_get() == TRACE_MODE_COLOR | TRACE_ACTIVE_LEVEL_ALL);
    mbed_trace_config_set(TRACE_MODE_PLAIN | TRACE_ACTIVE_LEVEL_NONE);
    ASSERT_TRUE(mbed_trace_config_get() == TRACE_MODE_PLAIN | TRACE_ACTIVE_LEVEL_NONE);
    mbed_trace_config_set(TRACE_MODE_PLAIN | TRACE_ACTIVE_LEVEL_ALL);
    ASSERT_TRUE(mbed_trace_config_get() == TRACE_MODE_PLAIN | TRACE_ACTIVE_LEVEL_ALL);
}

TEST_F(trace, active_level_all_color)
{
    mbed_trace_config_set(TRACE_MODE_COLOR | TRACE_ACTIVE_LEVEL_ALL);
    // unknown debug level
    mbed_tracef(TRACE_LEVEL_DEBUG + 1, "mygr", "hep");
    ASSERT_STREQ("              hep", buf);
    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "hello");
    ASSERT_STREQ("\x1b[90m[DBG ][mygr]: hello\x1b[0m", buf);
    mbed_tracef(TRACE_LEVEL_INFO, "mygr", "to one");
    ASSERT_STREQ("\x1b[39m[INFO][mygr]: to one\x1b[0m", buf);
    mbed_tracef(TRACE_LEVEL_WARN, "mygr", "and all");
    ASSERT_STREQ("\x1b[33m[WARN][mygr]: and all\x1b[0m", buf);
    mbed_tracef(TRACE_LEVEL_ERROR, "mygr", "even you");
    ASSERT_STREQ("\x1b[31m[ERR ][mygr]: even you\x1b[0m", buf);
}

TEST_F(trace, change_levels)
{
    mbed_trace_config_set(TRACE_ACTIVE_LEVEL_DEBUG);
    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "hep");
    ASSERT_STREQ("[DBG ][mygr]: hep", buf);

    mbed_trace_config_set(TRACE_ACTIVE_LEVEL_DEBUG | TRACE_MODE_PLAIN);
    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "hep");
    ASSERT_STREQ("hep", buf);

    mbed_trace_config_set(TRACE_ACTIVE_LEVEL_DEBUG | TRACE_MODE_COLOR);
    mbed_tracef(TRACE_LEVEL_ERROR, "mygr", "hep");
    ASSERT_STREQ("\x1b[31m[ERR ][mygr]: hep\x1b[0m", buf);

}

TEST_F(trace, active_level_debug)
{
    mbed_trace_config_set(TRACE_ACTIVE_LEVEL_DEBUG);

    // unknown debug level
    mbed_tracef(TRACE_LEVEL_DEBUG + 1, "mygr", "hep");
    ASSERT_STREQ("              hep", buf);

    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "hep");
    ASSERT_STREQ("[DBG ][mygr]: hep", buf);

    mbed_tracef(TRACE_LEVEL_INFO, "mygr", "test");
    ASSERT_STREQ("[INFO][mygr]: test", buf);

    mbed_tracef(TRACE_LEVEL_WARN, "mygr", "hups");
    ASSERT_STREQ("[WARN][mygr]: hups", buf);

    mbed_tracef(TRACE_LEVEL_ERROR, "mygr", "o'ou");
    ASSERT_STREQ("[ERR ][mygr]: o'ou", buf);
}

TEST_F(trace, active_level_info)
{
    buf[0] = 0;
    mbed_trace_config_set(TRACE_ACTIVE_LEVEL_INFO);

    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "hep");
    ASSERT_STREQ("", mbed_trace_last());

    mbed_tracef(TRACE_LEVEL_INFO, "mygr", "test");
    ASSERT_STREQ("[INFO][mygr]: test", buf);

    mbed_tracef(TRACE_LEVEL_WARN, "mygr", "hups");
    ASSERT_STREQ("[WARN][mygr]: hups", buf);

    mbed_tracef(TRACE_LEVEL_ERROR, "mygr", "o'ou");
    ASSERT_STREQ("[ERR ][mygr]: o'ou", buf);
}

TEST_F(trace, active_level_warn)
{
    mbed_trace_config_set(TRACE_ACTIVE_LEVEL_WARN);

    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "hep");
    ASSERT_STREQ("", mbed_trace_last());

    mbed_tracef(TRACE_LEVEL_INFO, "mygr", "test");
    ASSERT_STREQ("", mbed_trace_last());

    mbed_tracef(TRACE_LEVEL_WARN, "mygr", "hups");
    ASSERT_STREQ("[WARN][mygr]: hups", buf);

    mbed_tracef(TRACE_LEVEL_ERROR, "mygr", "o'ou");
    ASSERT_STREQ("[ERR ][mygr]: o'ou", buf);
}

TEST_F(trace, active_level_error)
{
    mbed_trace_config_set(TRACE_ACTIVE_LEVEL_ERROR);

    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "hep");
    ASSERT_STREQ("", mbed_trace_last());

    mbed_tracef(TRACE_LEVEL_INFO, "mygr", "test");
    ASSERT_STREQ("", mbed_trace_last());

    mbed_tracef(TRACE_LEVEL_WARN, "mygr", "hups");
    ASSERT_STREQ("", mbed_trace_last());

    mbed_tracef(TRACE_LEVEL_ERROR, "mygr", "o'ou");
    ASSERT_STREQ("[ERR ][mygr]: o'ou", buf);
}
TEST_F(trace, active_level_none)
{
    mbed_trace_config_set(TRACE_ACTIVE_LEVEL_NONE);

    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "hep");
    ASSERT_STREQ("", mbed_trace_last());

    mbed_tracef(TRACE_LEVEL_INFO, "mygr", "test");
    ASSERT_STREQ("", mbed_trace_last());

    mbed_tracef(TRACE_LEVEL_WARN, "mygr", "hups");
    ASSERT_STREQ("", mbed_trace_last());

    mbed_tracef(TRACE_LEVEL_ERROR, "mygr", "o'ou");
    ASSERT_STREQ("", mbed_trace_last());
}

TEST_F(trace, active_level_all_1)
{
    mbed_trace_config_set(TRACE_ACTIVE_LEVEL_ALL);
    mbed_trace_exclude_filters_set((char *)"mygr");

    mbed_tracef(TRACE_LEVEL_DEBUG, "mygu", "hep");
    ASSERT_STREQ("[DBG ][mygu]: hep", buf);

    mbed_tracef(TRACE_LEVEL_INFO, "mygr", "test");
    ASSERT_STREQ("", mbed_trace_last());
}
TEST_F(trace, active_level_all_2)
{
    mbed_trace_config_set(TRACE_ACTIVE_LEVEL_ALL);
    mbed_trace_exclude_filters_set((char *)"mygr,mygu");

    mbed_tracef(TRACE_LEVEL_DEBUG, "mygu", "hep");
    ASSERT_STREQ("", mbed_trace_last());

    mbed_tracef(TRACE_LEVEL_INFO, "mygr", "test");
    ASSERT_STREQ("", mbed_trace_last());
}
TEST_F(trace, active_level_all_3)
{
    mbed_trace_config_set(TRACE_ACTIVE_LEVEL_ALL);
    mbed_trace_include_filters_set((char *)"mygr");

    mbed_tracef(TRACE_LEVEL_DEBUG, "mygu", "hep");
    ASSERT_STREQ("", mbed_trace_last());

    mbed_tracef(TRACE_LEVEL_INFO, "mygr", "test");
    ASSERT_STREQ("[INFO][mygr]: test", buf);
}

TEST_F(trace, active_level_all_array)
{
    mbed_trace_config_set(TRACE_ACTIVE_LEVEL_ALL);

    uint8_t arr[] = {0x01, 0x02, 0x03};
    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "my addr: %s", mbed_trace_array(arr, 3));
    ASSERT_STREQ("[DBG ][mygr]: my addr: 01:02:03", buf);
}


size_t time_length;
char trace_prefix_str[] = "[<TIME>]";
char *trace_prefix(size_t length)
{
    time_length = length;
    return trace_prefix_str;
}
TEST_F(trace, prefix)
{
    mbed_trace_config_set(TRACE_ACTIVE_LEVEL_ALL);
    mbed_trace_prefix_function_set(&trace_prefix);
    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "test %d %d", 1, 2);
    ASSERT_STREQ("[<TIME>][DBG ][mygr]: test 1 2", buf);
    //TEST_ASSERT_EQUAL_INT(4, time_length);

    mbed_trace_config_set(TRACE_ACTIVE_LEVEL_ALL | TRACE_MODE_PLAIN);
    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "test");
    ASSERT_STREQ("test", buf);

    mbed_trace_config_set(TRACE_ACTIVE_LEVEL_ALL | TRACE_MODE_COLOR);
    mbed_tracef(TRACE_LEVEL_ERROR, "mygr", "test");
    ASSERT_STREQ("\x1b[31m[<TIME>][ERR ][mygr]: test\x1b[0m", buf);
}
char trace_suffix_str[] = "[END]";
char *trace_suffix()
{
    return trace_suffix_str;
}
TEST_F(trace, suffix)
{
    mbed_trace_config_set(TRACE_ACTIVE_LEVEL_ALL);
    mbed_trace_prefix_function_set(&trace_prefix);
    mbed_trace_suffix_function_set(&trace_suffix);
    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "test");
    ASSERT_STREQ("[<TIME>][DBG ][mygr]: test[END]", buf);
}
TEST_F(trace, formatting)
{
    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "hello %d %d %.1f", 12, 13, 5.5);
    ASSERT_STREQ("hello 12 13 5.5", buf);

    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "hello %d %d %d %d %d %d", 12, 13, 5, 6, 8, 9);
    ASSERT_STREQ("hello 12 13 5 6 8 9", buf);

    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "HOH %d HAH %d %d %d %d %d", 12, 13, 5, 6, 8, 9);
    ASSERT_STREQ("HOH 12 HAH 13 5 6 8 9", buf);
}
TEST_F(trace, filters_control)
{
    mbed_trace_include_filters_set((char *)"hello");
    ASSERT_STREQ("hello", mbed_trace_include_filters_get());

    mbed_trace_include_filters_set(0);
    ASSERT_STREQ("", mbed_trace_include_filters_get());

    mbed_trace_exclude_filters_set((char *)"hello");
    ASSERT_STREQ("hello", mbed_trace_exclude_filters_get());

    mbed_trace_exclude_filters_set(0);
    ASSERT_STREQ("", mbed_trace_exclude_filters_get());
}
TEST_F(trace, cmd_printer)
{
    buf[0] = 0;
    mbed_trace_config_set(TRACE_ACTIVE_LEVEL_ALL);
    mbed_tracef(TRACE_LEVEL_CMD, "mygr", "default printer");
    ASSERT_STREQ("default printer", buf);

    mbed_trace_cmdprint_function_set(myprint);
    mbed_tracef(TRACE_LEVEL_CMD, "mygr", "custom printer");
    ASSERT_STREQ("\n", buf); // because there is two print calls, second one add line feeds
}
TEST_F(trace, no_printer)
{
    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "hello");
    ASSERT_STREQ("hello", buf);

    mbed_trace_print_function_set(NULL);
    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "this shoudnt be printed because printer is missing");
    ASSERT_STREQ("hello", buf);
}
TEST_F(trace, uninitialized)
{
    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "hello");
    ASSERT_STREQ("hello", buf);

    mbed_trace_free();

    mbed_tracef(TRACE_LEVEL_DEBUG, "mygr", "this shoudnt be printed because mtrace is not initialized");
    ASSERT_STREQ("hello", buf);
}

