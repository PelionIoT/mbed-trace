/*
 * Copyright (c) 2014-2015 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>

#ifdef MBED_CONF_MBED_TRACE_ENABLE
#undef MBED_CONF_MBED_TRACE_ENABLE
#endif
#define MBED_CONF_MBED_TRACE_ENABLE 1
#ifndef MBED_CONF_MBED_TRACE_FEA_IPV6
#define MBED_CONF_MBED_TRACE_FEA_IPV6 1
#endif

#include "mbed-trace/mbed_trace.h"


#if defined(YOTTA_CFG_MBED_TRACE_MEM)
#define MBED_TRACE_MEM_INCLUDE      YOTTA_CFG_MBED_TRACE_MEM_INCLUDE
#define MBED_TRACE_MEM_ALLOC        YOTTA_CFG_MBED_TRACE_MEM_ALLOC
#define MBED_TRACE_MEM_FREE         YOTTA_CFG_MBED_TRACE_MEM_FREE
#else /* YOTTA_CFG_MEMLIB */
// Default options
#ifndef MBED_TRACE_MEM_INCLUDE
#define MBED_TRACE_MEM_INCLUDE   <stdlib.h>
#endif
#include MBED_TRACE_MEM_INCLUDE
#ifndef MBED_TRACE_MEM_ALLOC
#define MBED_TRACE_MEM_ALLOC malloc
#endif
#ifndef MBED_TRACE_MEM_FREE
#define MBED_TRACE_MEM_FREE  free
#endif
#endif /* YOTTA_CFG_MEMLIB */

#define VT100_COLOR_ERROR "\x1b[31m"
#define VT100_COLOR_WARN  "\x1b[33m"
#define VT100_COLOR_INFO  "\x1b[39m"
#define VT100_COLOR_DEBUG "\x1b[90m"

/** default max trace line size in bytes */
#ifdef MBED_TRACE_LINE_LENGTH
#define DEFAULT_TRACE_LINE_LENGTH         MBED_TRACE_LINE_LENGTH
#elif defined YOTTA_CFG_MBED_TRACE_LINE_LENGTH
#warning YOTTA_CFG_MBED_TRACE_LINE_LENGTH is deprecated and will be removed in the future! Use MBED_TRACE_LINE_LENGTH instead.
#define DEFAULT_TRACE_LINE_LENGTH         YOTTA_CFG_MBED_TRACE_LINE_LENGTH
#else
#define DEFAULT_TRACE_LINE_LENGTH         1024
#endif

/** default max temporary buffer size in bytes, used in
    trace_ipv6, trace_ipv6_prefix and trace_array */
#ifdef MBED_TRACE_TMP_LINE_LENGTH
#define DEFAULT_TRACE_TMP_LINE_LEN        MBED_TRACE_TMP_LINE_LENGTH
#elif defined YOTTA_CFG_MBED_TRACE_TMP_LINE_LEN
#warning The YOTTA_CFG_MBED_TRACE_TMP_LINE_LEN flag is deprecated and will be removed in the future! Use MBED_TRACE_TMP_LINE_LENGTH instead.
#define DEFAULT_TRACE_TMP_LINE_LEN        YOTTA_CFG_MBED_TRACE_TMP_LINE_LEN
#elif defined YOTTA_CFG_MTRACE_TMP_LINE_LEN
#warning The YOTTA_CFG_MTRACE_TMP_LINE_LEN flag is deprecated and will be removed in the future! Use MBED_TRACE_TMP_LINE_LENGTH instead.
#define DEFAULT_TRACE_TMP_LINE_LEN        YOTTA_CFG_MTRACE_TMP_LINE_LEN
#else
#define DEFAULT_TRACE_TMP_LINE_LEN        128
#endif

/** default max filters (include/exclude) length in bytes */
#ifdef MBED_TRACE_FILTER_LENGTH
#define DEFAULT_TRACE_FILTER_LENGTH       MBED_TRACE_FILTER_LENGTH
#else
#define DEFAULT_TRACE_FILTER_LENGTH       24
#endif

/** default trace configuration bitmask */
#ifdef MBED_TRACE_CONFIG
#define DEFAULT_TRACE_CONFIG              MBED_TRACE_CONFIG
#else
#define DEFAULT_TRACE_CONFIG              TRACE_MODE_COLOR | TRACE_ACTIVE_LEVEL_ALL | TRACE_CARRIAGE_RETURN
#endif

/** default print function, just redirect str to printf */
static void mbed_trace_realloc( char **buffer, int *length_ptr, int new_length);
static void mbed_trace_default_print(const char *str);
static void mbed_trace_reset_tmp(void);

typedef struct trace_s {
    /** trace configuration bits */
    uint8_t trace_config;
    /** exclude filters list, related group name */
    char *filters_exclude;
    /** include filters list, related group name */
    char *filters_include;
    /** Filters length */
    int filters_length;
    /** trace line */
    char *line;
    /** trace line length */
    int line_length;
    /** temporary data */
    char *tmp_data;
    /** temporary data array length */
    int tmp_data_length;
    /** temporary data pointer */
    char *tmp_data_ptr;

    /** prefix function, which can be used to put time to the trace line */
    char *(*prefix_f)(size_t);
    /** suffix function, which can be used to some string to the end of trace line */
    char *(*suffix_f)(void);
    /** print out function. Can be redirect to flash for example. */
    void (*printf)(const char *);
    /** print out function for TRACE_LEVEL_CMD */
    void (*cmd_printf)(const char *);
    /** mutex wait function which can be called to lock against a mutex. */
    void (*mutex_wait_f)(void);
    /** mutex release function which must be used to release the mutex locked by mutex_wait_f. */
    void (*mutex_release_f)(void);
    /** number of times the mutex has been locked */
    int mutex_lock_count;
} trace_t;

static trace_t m_trace = {
    .trace_config = DEFAULT_TRACE_CONFIG,
    .filters_exclude = 0,
    .filters_include = 0,
    .filters_length = DEFAULT_TRACE_FILTER_LENGTH,
    .line = 0,
    .line_length = DEFAULT_TRACE_LINE_LENGTH,
    .tmp_data = 0,
    .tmp_data_length = DEFAULT_TRACE_TMP_LINE_LEN,
    .prefix_f = 0,
    .suffix_f = 0,
    .printf  = mbed_trace_default_print,
    .cmd_printf = 0,
    .mutex_wait_f = 0,
    .mutex_release_f = 0,
    .mutex_lock_count = 0
};




/************************************************************************************************
 * adding ipv6 API
 ************************************************************************************************/
#if ((MBED_CONF_MBED_TRACE_FEA_IPV6 == 1) && defined(MBEDTRACE_USE_STD_LIBS))

#define IPV6_TO_STR ip6toString
#define IPV6_PREFIX_TO STR ip6_prefix_toString

uint8_t *bitCpy(uint8_t *restrict dst, const uint8_t *restrict src, uint_fast8_t bits)
{
    uint_fast8_t bytes = bits / 8;
    bits %= 8;

    if (bytes) {
        dst = (uint8_t *) memcpy(dst, src, bytes) + bytes;
        src += bytes;
    }

    if (bits) {
        uint_fast8_t split_bit = context_split_mask(bits);
        *dst = (*src & split_bit) | (*dst & ~ split_bit);
    }

    return dst;
}


static uint_fast8_t ip6toString(const void *ip6addr, char *p)
{
    char *p_orig = p;
    uint_fast8_t zero_start = 255, zero_len = 1;
    const uint8_t *addr = ip6addr;
    uint_fast16_t part;

    /* Follow RFC 5952 - pre-scan for longest run of zeros */
    for (uint_fast8_t n = 0; n < 8; n++) {
        part = *addr++;
        part = (part << 8) | *addr++;
        if (part != 0) {
            continue;
        }

        /* We're at the start of a run of zeros - scan to non-zero (or end) */
        uint_fast8_t n0 = n;
        for (n = n0 + 1; n < 8; n++) {
            part = *addr++;
            part = (part << 8) | *addr++;
            if (part != 0) {
                break;
            }
        }

        /* Now n0->initial zero of run, n->after final zero in run. Is this the
         * longest run yet? If equal, we stick with the previous one - RFC 5952
         * S4.2.3. Note that zero_len being initialised to 1 stops us
         * shortening a 1-part run (S4.2.2.)
         */
        if (n - n0 > zero_len) {
            zero_start = n0;
            zero_len = n - n0;
        }

        /* Continue scan for initial zeros from part n+1 - we've already
         * consumed part n, and know it's non-zero. */
    }

    /* Now go back and print, jumping over any zero run */
    addr = ip6addr;
    for (uint_fast8_t n = 0; n < 8;) {
        if (n == zero_start) {
            if (n == 0) {
                *p++ = ':';
            }
            *p++ = ':';
            addr += 2 * zero_len;
            n += zero_len;
            continue;
        }

        part = *addr++;
        part = (part << 8) | *addr++;
        n++;

        p += sprintf(p, "%"PRIxFAST16, part);

        /* One iteration writes "part:" rather than ":part", and has the
         * explicit check for n == 8 below, to allow easy extension for
         * IPv4-in-IPv6-type addresses ("xxxx::xxxx:a.b.c.d"): we'd just
         * run the same loop for 6 parts, and output would then finish with the
         * required : or ::, ready for "a.b.c.d" to be tacked on.
         */
        if (n != 8) {
            *p++ = ':';
        }
    }
    *p = '\0';

    // Return length of generated string, excluding the terminating null character
    return p - p_orig;
}

static uint_fast8_t ip6_prefix_toString(const void *prefix, uint_fast8_t prefix_len, char *p)
{
    char *wptr = p;
    uint8_t addr[16] = {0};

    if (prefix_len > 128) {
        return 0;
    }

    // Generate prefix part of the string
    bitCpy(addr, prefix, prefix_len);
    wptr += IPV6_TO_STR(addr, wptr);
    // Add the prefix length part of the string
    wptr += sprintf(wptr, "/%"PRIuFAST8, prefix_len);

    // Return total length of generated string
    return wptr - p;
}
#else
#define IPV6_TO_STR ip6tos
#define IPV6_PREFIX_TO_STR ip6_prefix_tos
#endif


int mbed_trace_init(void)
{
    if (m_trace.line == NULL) {
        m_trace.line = MBED_TRACE_MEM_ALLOC(m_trace.line_length);
    }

    if (m_trace.tmp_data == NULL) {
        m_trace.tmp_data = MBED_TRACE_MEM_ALLOC(m_trace.tmp_data_length);
    }
    m_trace.tmp_data_ptr = m_trace.tmp_data;

    if (m_trace.filters_exclude == NULL) {
        m_trace.filters_exclude = MBED_TRACE_MEM_ALLOC(m_trace.filters_length);
    }
    if (m_trace.filters_include == NULL) {
        m_trace.filters_include = MBED_TRACE_MEM_ALLOC(m_trace.filters_length);
    }

    if (m_trace.line == NULL ||
            m_trace.tmp_data == NULL ||
            m_trace.filters_exclude == NULL  ||
            m_trace.filters_include == NULL) {
        //memory allocation fail
        mbed_trace_free();
        return -1;
    }
    memset(m_trace.tmp_data, 0, m_trace.tmp_data_length);
    memset(m_trace.filters_exclude, 0, m_trace.filters_length);
    memset(m_trace.filters_include, 0, m_trace.filters_length);
    memset(m_trace.line, 0, m_trace.line_length);

    return 0;
}
void mbed_trace_free(void)
{
    // release memory
    MBED_TRACE_MEM_FREE(m_trace.line);
    MBED_TRACE_MEM_FREE(m_trace.tmp_data);
    MBED_TRACE_MEM_FREE(m_trace.filters_exclude);
    MBED_TRACE_MEM_FREE(m_trace.filters_include);

    // reset to default values
    m_trace.trace_config = DEFAULT_TRACE_CONFIG;
    m_trace.filters_exclude = 0;
    m_trace.filters_include = 0;
    m_trace.filters_length = DEFAULT_TRACE_FILTER_LENGTH;
    m_trace.line = 0;
    m_trace.line_length = DEFAULT_TRACE_LINE_LENGTH;
    m_trace.tmp_data = 0;
    m_trace.tmp_data_length = DEFAULT_TRACE_TMP_LINE_LEN;
    m_trace.prefix_f = 0;
    m_trace.suffix_f = 0;
    m_trace.printf  = mbed_trace_default_print;
    m_trace.cmd_printf = 0;
    m_trace.mutex_wait_f = 0;
    m_trace.mutex_release_f = 0;
    m_trace.mutex_lock_count = 0;
}
static void mbed_trace_realloc( char **buffer, int *length_ptr, int new_length)
{
    MBED_TRACE_MEM_FREE(*buffer);
    *buffer  = MBED_TRACE_MEM_ALLOC(new_length);
    *length_ptr = new_length;
}
void mbed_trace_buffer_sizes(int lineLength, int tmpLength)
{
    if( lineLength > 0 ) {
        mbed_trace_realloc( &(m_trace.line), &m_trace.line_length, lineLength );
    }
    if( tmpLength > 0 ) {
        mbed_trace_realloc( &(m_trace.tmp_data), &m_trace.tmp_data_length, tmpLength);
        mbed_trace_reset_tmp();
    }
}
void mbed_trace_config_set(uint8_t config)
{
    m_trace.trace_config = config;
}
uint8_t mbed_trace_config_get(void)
{
    return m_trace.trace_config;
}
void mbed_trace_prefix_function_set(char *(*pref_f)(size_t))
{
    m_trace.prefix_f = pref_f;
}
void mbed_trace_suffix_function_set(char *(*suffix_f)(void))
{
    m_trace.suffix_f = suffix_f;
}
void mbed_trace_print_function_set(void (*printf)(const char *))
{
    m_trace.printf = printf;
}
void mbed_trace_cmdprint_function_set(void (*printf)(const char *))
{
    m_trace.cmd_printf = printf;
}
void mbed_trace_mutex_wait_function_set(void (*mutex_wait_f)(void))
{
    m_trace.mutex_wait_f = mutex_wait_f;
}
void mbed_trace_mutex_release_function_set(void (*mutex_release_f)(void))
{
    m_trace.mutex_release_f = mutex_release_f;
}
void mbed_trace_exclude_filters_set(char *filters)
{
    if (filters) {
        (void)strncpy(m_trace.filters_exclude, filters, m_trace.filters_length);
    } else {
        m_trace.filters_exclude[0] = 0;
    }
}
const char *mbed_trace_exclude_filters_get(void)
{
    return m_trace.filters_exclude;
}
const char *mbed_trace_include_filters_get(void)
{
    return m_trace.filters_include;
}
void mbed_trace_include_filters_set(char *filters)
{
    if (filters) {
        (void)strncpy(m_trace.filters_include, filters, m_trace.filters_length);
    } else {
        m_trace.filters_include[0] = 0;
    }
}
static int8_t mbed_trace_skip(int8_t dlevel, const char *grp)
{
    if (dlevel >= 0 && grp != 0) {
        // filter debug prints only when dlevel is >0 and grp is given

        /// @TODO this could be much better..
        if (m_trace.filters_exclude[0] != '\0' &&
                strstr(m_trace.filters_exclude, grp) != 0) {
            //grp was in exclude list
            return 1;
        }
        if (m_trace.filters_include[0] != '\0' &&
                strstr(m_trace.filters_include, grp) == 0) {
            //grp was in include list
            return 1;
        }
    }
    return 0;
}
static void mbed_trace_default_print(const char *str)
{
    puts(str);
}
void mbed_tracef(uint8_t dlevel, const char *grp, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    mbed_vtracef(dlevel, grp, fmt, ap);
    va_end(ap);
}
void mbed_vtracef(uint8_t dlevel, const char* grp, const char *fmt, va_list ap)
{
    if ( m_trace.mutex_wait_f ) {
        m_trace.mutex_wait_f();
        m_trace.mutex_lock_count++;
    }

    if (NULL == m_trace.line) {
        goto end;
    }

    m_trace.line[0] = 0; //by default trace is empty

    if (mbed_trace_skip(dlevel, grp) || fmt == 0 || grp == 0 || !m_trace.printf) {
        //return tmp data pointer back to the beginning
        mbed_trace_reset_tmp();
        goto end;
    }
    if ((m_trace.trace_config & TRACE_MASK_LEVEL) &  dlevel) {
        bool color = (m_trace.trace_config & TRACE_MODE_COLOR) != 0;
        bool plain = (m_trace.trace_config & TRACE_MODE_PLAIN) != 0;
        bool cr    = (m_trace.trace_config & TRACE_CARRIAGE_RETURN) != 0;

        int retval = 0, bLeft = m_trace.line_length;
        char *ptr = m_trace.line;
        if (plain == true || dlevel == TRACE_LEVEL_CMD) {
            //add trace data
            retval = vsnprintf(ptr, bLeft, fmt, ap);
            if (dlevel == TRACE_LEVEL_CMD && m_trace.cmd_printf) {
                m_trace.cmd_printf(m_trace.line);
                m_trace.cmd_printf("\n");
            } else {
                //print out whole data
                m_trace.printf(m_trace.line);
            }
        } else {
            if (color) {
                if (cr) {
                    retval = snprintf(ptr, bLeft, "\r\x1b[2K");
                    if (retval >= bLeft) {
                        retval = 0;
                    }
                    if (retval > 0) {
                        ptr += retval;
                        bLeft -= retval;
                    }
                }
                if (bLeft > 0) {
                    //include color in ANSI/VT100 escape code
                    switch (dlevel) {
                        case (TRACE_LEVEL_ERROR):
                            retval = snprintf(ptr, bLeft, "%s", VT100_COLOR_ERROR);
                            break;
                        case (TRACE_LEVEL_WARN):
                            retval = snprintf(ptr, bLeft, "%s", VT100_COLOR_WARN);
                            break;
                        case (TRACE_LEVEL_INFO):
                            retval = snprintf(ptr, bLeft, "%s", VT100_COLOR_INFO);
                            break;
                        case (TRACE_LEVEL_DEBUG):
                            retval = snprintf(ptr, bLeft, "%s", VT100_COLOR_DEBUG);
                            break;
                        default:
                            color = 0; //avoid unneeded color-terminate code
                            retval = 0;
                            break;
                    }
                    if (retval >= bLeft) {
                        retval = 0;
                    }
                    if (retval > 0 && color) {
                        ptr += retval;
                        bLeft -= retval;
                    }
                }

            }
            if (bLeft > 0 && m_trace.prefix_f) {
                //find out length of body
                size_t sz = 0;
                va_list ap2;
                va_copy(ap2, ap);
                sz = vsnprintf(NULL, 0, fmt, ap2) + retval + (retval ? 4 : 0);
                va_end(ap2);
                //add prefix string
                retval = snprintf(ptr, bLeft, "%s", m_trace.prefix_f(sz));
                if (retval >= bLeft) {
                    retval = 0;
                }
                if (retval > 0) {
                    ptr += retval;
                    bLeft -= retval;
                }
            }
            if (bLeft > 0) {
                //add group tag
                switch (dlevel) {
                    case (TRACE_LEVEL_ERROR):
                        retval = snprintf(ptr, bLeft, "[ERR ][%-4s]: ", grp);
                        break;
                    case (TRACE_LEVEL_WARN):
                        retval = snprintf(ptr, bLeft, "[WARN][%-4s]: ", grp);
                        break;
                    case (TRACE_LEVEL_INFO):
                        retval = snprintf(ptr, bLeft, "[INFO][%-4s]: ", grp);
                        break;
                    case (TRACE_LEVEL_DEBUG):
                        retval = snprintf(ptr, bLeft, "[DBG ][%-4s]: ", grp);
                        break;
                    default:
                        retval = snprintf(ptr, bLeft, "              ");
                        break;
                }
                if (retval >= bLeft) {
                    retval = 0;
                }
                if (retval > 0) {
                    ptr += retval;
                    bLeft -= retval;
                }
            }
            if (retval > 0 && bLeft > 0) {
                //add trace text
                retval = vsnprintf(ptr, bLeft, fmt, ap);
                if (retval >= bLeft) {
                    retval = 0;
                }
                if (retval > 0) {
                    ptr += retval;
                    bLeft -= retval;
                }
            }

            if (retval > 0 && bLeft > 0  && m_trace.suffix_f) {
                //add suffix string
                retval = snprintf(ptr, bLeft, "%s", m_trace.suffix_f());
                if (retval >= bLeft) {
                    retval = 0;
                }
                if (retval > 0) {
                    ptr += retval;
                    bLeft -= retval;
                }
            }

            if (retval > 0 && bLeft > 0  && color) {
                //add zero color VT100 when color mode
                retval = snprintf(ptr, bLeft, "\x1b[0m");
                if (retval >= bLeft) {
                    retval = 0;
                }
                if (retval > 0) {
                    // not used anymore
                    //ptr += retval;
                    //bLeft -= retval;
                }
            }
            //print out whole data
            m_trace.printf(m_trace.line);
        }
        //return tmp data pointer back to the beginning
        mbed_trace_reset_tmp();
    }

end:
    if ( m_trace.mutex_release_f ) {
        // Store the mutex lock count to temp variable so that it won't get
        // clobbered during last loop iteration when mutex gets released
        int count = m_trace.mutex_lock_count;
        m_trace.mutex_lock_count = 0;
        // Since the helper functions (eg. mbed_trace_array) are used like this:
        //   mbed_tracef(TRACE_LEVEL_INFO, "grp", "%s", mbed_trace_array(some_array))
        // The helper function MUST acquire the mutex if it modifies any buffers. However
        // it CANNOT unlock the mutex because that would allow another thread to acquire
        // the mutex after helper function unlocks it and before mbed_tracef acquires it
        // for itself. This means that here we have to unlock the mutex as many times
        // as it was acquired by trace function and any possible helper functions.
        do {
            m_trace.mutex_release_f();
        } while (--count > 0);
    }
}
static void mbed_trace_reset_tmp(void)
{
    m_trace.tmp_data_ptr = m_trace.tmp_data;
}
const char *mbed_trace_last(void)
{
    return m_trace.line;
}
/* Helping functions */
#define tmp_data_left()  m_trace.tmp_data_length-(m_trace.tmp_data_ptr-m_trace.tmp_data)
#if MBED_CONF_MBED_TRACE_FEA_IPV6 == 1
char *mbed_trace_ipv6(const void *addr_ptr)
{
    /** Acquire mutex. It is released before returning from mbed_vtracef. */
    if ( m_trace.mutex_wait_f ) {
        m_trace.mutex_wait_f();
        m_trace.mutex_lock_count++;
    }
    char *str = m_trace.tmp_data_ptr;
    if (str == NULL) {
        return "";
    }
    if (tmp_data_left() < 41) {
        return "";
    }
    if (addr_ptr == NULL) {
        return "<null>";
    }
    str[0] = 0;
    m_trace.tmp_data_ptr += IPV6_TO_STR(addr_ptr, str) + 1;
    return str;
}
char *mbed_trace_ipv6_prefix(const uint8_t *prefix, uint8_t prefix_len)
{
    /** Acquire mutex. It is released before returning from mbed_vtracef. */
    if ( m_trace.mutex_wait_f ) {
        m_trace.mutex_wait_f();
        m_trace.mutex_lock_count++;
    }
    char *str = m_trace.tmp_data_ptr;
    if (str == NULL) {
        return "";
    }
    if (tmp_data_left() < 45) {
        return "";
    }

    if ((prefix_len != 0 && prefix == NULL) || prefix_len > 128) {
        return "<err>";
    }

    m_trace.tmp_data_ptr += IPV6_PREFIX_TO_STR(prefix, prefix_len, str) + 1;
    return str;
}
#endif //MBED_CONF_MBED_TRACE_FEA_IPV6
char *mbed_trace_array(const uint8_t *buf, uint16_t len)
{
    /** Acquire mutex. It is released before returning from mbed_vtracef. */
    if ( m_trace.mutex_wait_f ) {
        m_trace.mutex_wait_f();
        m_trace.mutex_lock_count++;
    }
    int i, bLeft = tmp_data_left();
    char *str, *wptr;
    str = m_trace.tmp_data_ptr;
    if (len == 0 || str == NULL || bLeft == 0) {
        return "";
    }
    if (buf == NULL) {
        return "<null>";
    }
    wptr = str;
    wptr[0] = 0;
    const uint8_t *ptr = buf;
    char overflow = 0;
    for (i = 0; i < len; i++) {
        if (bLeft <= 3) {
            overflow = 1;
            break;
        }
        int retval = snprintf(wptr, bLeft, "%02x:", *ptr++);
        if (retval <= 0 || retval > bLeft) {
            break;
        }
        bLeft -= retval;
        wptr += retval;
    }
    if (wptr > str) {
        if( overflow ) {
            // replace last character as 'star',
            // which indicate buffer len is not enough
            *(wptr - 1) = '*';
        } else {
            //null to replace last ':' character
            *(wptr - 1) = 0;
        }
    }
    m_trace.tmp_data_ptr = wptr;
    return str;
}

