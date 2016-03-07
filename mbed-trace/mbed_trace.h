/*
 * Copyright (c) 2015 ARM Limited. All rights reserved.
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

/**
 * \file mbed_trace.h
 * Trace interface for MbedOS applications.
 * This file provide simple but flexible way to handle software traces.
 * Trace library are abstract layer, which use stdout (printf) by default, 
 * but outputs can be easily redirect to custom function, for example to 
 * store traces to memory or other interfaces.
 *
 *  usage example:
 * \code(main.c:)
 *      #include "mbed_trace.h"
 *      #define TRACE_GROUP  "main"
 *
 *      int main(void){
 *          mbed_trace_init();   // initialize trace library
 *          tr_debug("this is debug msg");  //print debug message to stdout: "[DBG]
 *          tr_err("this is error msg");
 *          tr_warn("this is warning msg");
 *          tr_info("this is info msg");
 *          return 0;
 *      }
 * \endcode
 * Activate with compiler flag: YOTTA_CFG_MBED_TRACE
 * Configure trace line buffer size with compiler flag: YOTTA_CFG_MBED_TRACE_LINE_LENGTH. Default length: 1024.
 *
 */
#ifndef MBED_TRACE_H_
#define MBED_TRACE_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef YOTTA_CFG
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#else
#include "ns_types.h"
#endif

#ifndef YOTTA_CFG_MBED_TRACE_FEA_IPV6
#define YOTTA_CFG_MBED_TRACE_FEA_IPV6 1
#endif

/** 3 upper bits are trace modes related,
    and 5 lower bits are trace level configuration */

/** Config mask */
#define TRACE_MASK_CONFIG         0xE0
/** Trace level mask */
#define TRACE_MASK_LEVEL          0x1F

/** plain trace data instead of "headers" */
#define TRACE_MODE_PLAIN          0x80
/** color mode */
#define TRACE_MODE_COLOR          0x40
/** Use print CR before trace line */
#define TRACE_CARRIAGE_RETURN     0x20

/** used to activate all trace levels */
#define TRACE_ACTIVE_LEVEL_ALL    0x1F
/** print all traces same as above */
#define TRACE_ACTIVE_LEVEL_DEBUG  0x1f
/** print info,warn and error traces */
#define TRACE_ACTIVE_LEVEL_INFO   0x0f
/** print warn and error traces */
#define TRACE_ACTIVE_LEVEL_WARN   0x07
/** print only error trace */
#define TRACE_ACTIVE_LEVEL_ERROR  0x03
/** print only cmd line data */
#define TRACE_ACTIVE_LEVEL_CMD    0x01
/** trace nothing  */
#define TRACE_ACTIVE_LEVEL_NONE   0x00

/** this print is some deep information for debug purpose */
#define TRACE_LEVEL_DEBUG         0x10
/** Info print, for general purpose prints */
#define TRACE_LEVEL_INFO          0x08
/** warning prints, which shouldn't causes any huge problems */
#define TRACE_LEVEL_WARN          0x04
/** Error prints, which causes probably problems, e.g. out of mem. */
#define TRACE_LEVEL_ERROR         0x02
/** special level for cmdline. Behaviours like "plain mode" */
#define TRACE_LEVEL_CMD           0x01

//usage macros:
#define tr_info(...)            mbed_tracef(TRACE_LEVEL_INFO,    TRACE_GROUP, __VA_ARGS__)   //!< Print info message
#define tr_debug(...)           mbed_tracef(TRACE_LEVEL_DEBUG,   TRACE_GROUP, __VA_ARGS__)   //!< Print debug message
#define tr_warning(...)         mbed_tracef(TRACE_LEVEL_WARN,    TRACE_GROUP, __VA_ARGS__)   //!< Print warning message
#define tr_warn(...)            mbed_tracef(TRACE_LEVEL_WARN,    TRACE_GROUP, __VA_ARGS__)   //!< Alternative warning message
#define tr_error(...)           mbed_tracef(TRACE_LEVEL_ERROR,   TRACE_GROUP, __VA_ARGS__)   //!< Print Error Message
#define tr_err(...)             mbed_tracef(TRACE_LEVEL_ERROR,   TRACE_GROUP, __VA_ARGS__)   //!< Alternative error message
#define tr_cmdline(...)         mbed_tracef(TRACE_LEVEL_CMD,     TRACE_GROUP, __VA_ARGS__)   //!< Special print for cmdline. See more from TRACE_LEVEL_CMD -level

/** Possible to skip all traces in compile time */
#if defined(YOTTA_CFG_MBED_TRACE) || defined(HAVE_DEBUG) || (defined(YOTTA_CFG) && !defined(NDEBUG))

#if defined(__GNUC__) || defined(__CC_ARM)
/**
 * Initialize trace functionality
 * @return 0 when all success, otherwise non zero
 */
int mbed_trace_init( void );
/**
 * Free trace memory
 */
void mbed_trace_free( void );
/**
 * Resize buffers (line / tmp ) sizes
 * @param lineLength    new maximum length for trace line (0 = do no resize)
 * @param tmpLength     new maximum length for trace tmp buffer (used for trace_array, etc) (0 = do no resize)
 */
void mbed_trace_buffer_sizes(int lineLength, int tmpLength);
/**
 *  Set trace configurations
 *  Possible parameters:
 *
 *   TRACE_MODE_COLOR
 *   TRACE_MODE_PLAIN (this exclude color mode)
 *   TRACE_CARRIAGE_RETURN (print CR before trace line)
 *
 *   TRACE_ACTIVE_LEVEL_ALL - to activate all trace levels
 *   or TRACE_ACTIVE_LEVEL_DEBUG (alternative)
 *   TRACE_ACTIVE_LEVEL_INFO
 *   TRACE_ACTIVE_LEVEL_WARN
 *   TRACE_ACTIVE_LEVEL_ERROR
 *   TRACE_ACTIVE_LEVEL_CMD
 *   TRACE_LEVEL_NONE - to deactivate all traces
 *
 * @param config  Byte size Bit-mask. Bits are descripted above.
 * usage e.g.
 * @code
 *  mbed_trace_config_set( TRACE_ACTIVE_LEVEL_ALL|TRACE_MODE_COLOR );
 * @endcode
 */
void mbed_trace_config_set(uint8_t config);
/** get trace configurations 
 * @return trace configuration byte
 */
uint8_t mbed_trace_config_get(void);
/**
 * Set trace prefix function
 * pref_f -function return string with null terminated
 * Can be used for e.g. time string
 * e.g.
 *   char* trace_time(){ return "rtc-time-in-string"; }
 *   mbed_trace_prefix_function_set( &trace_time );
 */
void mbed_trace_prefix_function_set( char* (*pref_f)(size_t) );
/**
 * Set trace suffix function
 * suffix -function return string with null terminated
 * Can be used for e.g. time string
 * e.g.
 *   char* trace_suffix(){ return " END"; }
 *   mbed_trace_suffix_function_set( &trace_suffix );
 */
void mbed_trace_suffix_function_set(char* (*suffix_f)(void) );
/**
 * Set trace print function
 * By default, trace module print using printf() function,
 * but with this you can write own print function,
 * for e.g. to other IO device.
 */
void mbed_trace_print_function_set( void (*print_f)(const char*) );
/**
 * Set trace print function for tr_cmdline()
 */
void mbed_trace_cmdprint_function_set( void (*printf)(const char*) );
/**
 * When trace group contains text in filters,
 * trace print will be ignored.
 * e.g.: 
 *  mbed_trace_exclude_filters_set("mygr");
 *  mbed_tracef(TRACE_ACTIVE_LEVEL_DEBUG, "ougr", "This is not printed");
 */
void mbed_trace_exclude_filters_set(char* filters);
/** get trace exclude filters
 */
const char* mbed_trace_exclude_filters_get(void);
/**
 * When trace group contains text in filter,
 * trace will be printed.
 * e.g.:
 *  set_trace_include_filters("mygr");
 *  mbed_tracef(TRACE_ACTIVE_LEVEL_DEBUG, "mygr", "Hi There");
 *  mbed_tracef(TRACE_ACTIVE_LEVEL_DEBUG, "grp2", "This is not printed");
 */
void mbed_trace_include_filters_set(char* filters);
/** get trace include filters
 */
const char* mbed_trace_include_filters_get(void);
/**
 * General trace function
 * This should be used every time when user want to print out something important thing
 * Usage e.g.
 *   mbed_tracef( TRACE_LEVEL_INFO, "mygr", "Hello world!");
 *
 * @param dlevel debug level
 * @param grp    trace group
 * @param fmt    trace format (like printf)
 * @param ...    variable arguments related to fmt
 */
void mbed_tracef(uint8_t dlevel, const char* grp, const char *fmt, ...) __attribute__ ((__format__(__printf__, 3, 4)));
/**
 *  Get last trace from buffer
 */
const char* mbed_trace_last(void);
#if YOTTA_CFG_MBED_TRACE_FEA_IPV6 == 1
/**
 * mbed_tracef helping function for convert ipv6
 * table to human readable string.
 * usage e.g.
 * char ipv6[16] = {...}; // ! array length is 16 bytes !
 * mbed_tracef(TRACE_LEVEL_INFO, "mygr", "ipv6 addr: %s", mbed_trace_ipv6(ipv6));
 *
 * @param add_ptr  IPv6 Address pointer
 * @return temporary buffer where ipv6 is in string format
 */
char* mbed_trace_ipv6(const void *addr_ptr);
/**
 * mbed_tracef helping function for print ipv6 prefix
 * usage e.g.
 * char ipv6[16] = {...}; // ! array length is 16 bytes !
 * mbed_tracef(TRACE_LEVEL_INFO, "mygr", "ipv6 addr: %s", mbed_trace_ipv6_prefix(ipv6, 4));
 *
 * @param prefix        IPv6 Address pointer
 * @param prefix_len    prefix length
 * @return temporary buffer where ipv6 is in string format
 */
char* mbed_trace_ipv6_prefix(const uint8_t *prefix, uint8_t prefix_len);
#endif
/**
 * mbed_tracef helping function for convert hex-array to string.
 * usage e.g.
 *  char myarr[] = {0x10, 0x20};
 *  mbed_tracef(TRACE_LEVEL_INFO, "mygr", "arr: %s", mbed_trace_array(myarr, 2));
 *
 * @param buf  hex array pointer
 * @param len  buffer length
 * @return temporary buffer where string copied
 * if array as string not fit to temp buffer, this function write '*' as last character, 
 * which indicate that buffer is too small for array.
 */
char* mbed_trace_array(const uint8_t* buf, uint16_t len);

#else // defined(__GNUC__) || defined(__CC_ARM)
int  mbed_trace_init( void );
void mbed_trace_free( void );
void mbed_trace_config_set(uint8_t config);
void mbed_trace_prefix_function_set( char* (*pref_f)(size_t) );
void mbed_trace_suffix_function_set(char* (*suffix_f)(void) );
void mbed_trace_print_function_set( void (*print_f)(const char*) );
void mbed_trace_cmdprint_function_set( void (*printf)(const char*) );
void mbed_trace_exclude_filters_set(char* filters);
const char* mbed_trace_exclude_filters_get(void);
void mbed_trace_include_filters_set(char* filters);
const char* mbed_trace_include_filters_get(void);
void mbed_tracef(uint8_t dlevel, const char* grp, const char *fmt, ...);
const char* mbed_trace_last(void);
char* mbed_trace_array(const uint8_t* buf, uint16_t len);
#if YOTTA_CFG_MBED_TRACE_FEA_IPV6 == 1
char* mbed_trace_ipv6(const void *addr_ptr);
char* mbed_trace_ipv6_prefix(const uint8_t *prefix, uint8_t prefix_len);       
#endif // YOTTA_CFG_MBED_TRACE_FEA_IPV6

#endif // defined(__GNUC__) || defined(__CC_ARM)


#else // YOTTA_CFG_MBED_TRACE

// trace functionality not supported
#define mbed_trace_init(...)                ((void) 0)
#define mbed_trace_free(...)                ((void) 0)
#define mbed_trace_config_set(...)          ((void) 0)
#define mbed_trace_prefix_function_set(...) ((void) 0)
#define mbed_trace_suffix_function_set(...) ((void) 0)
#define mbed_trace_print_function_set(...)  ((void) 0)
#define mbed_trace_cmdprint_function_set(...)  ((void) 0)
#define mbed_trace_exclude_filters_get(...) ((void) 0)
#define mbed_trace_include_filters_set(...) ((void) 0)
#define mbed_trace_include_filters_get(...) ((void) 0)

#define mbed_trace_last(...)                ((void) 0)
#define mbed_tracef(...)                    ((void) 0)
#define mbed_trace_ipv6(...)                ((void) 0)
#define mbed_trace_array(...)               ((void) 0)
#define mbed_trace_ipv6_prefix(...)         ((void) 0)

#endif //YOTTA_CFG_MBED_TRACE

#ifdef __cplusplus
}
#endif

#endif /* MBED_TRACE_H_ */
