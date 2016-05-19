/*
 * Copyright (c) 2014-2015 ARM Limited. All rights reserved.
 */
#ifndef MBED_TRACE_TEST_STUBS_COMMON_FUNCTIONS_H_
#define MBED_TRACE_TEST_STUBS_COMMON_FUNCTIONS_H_

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t *bitcopy(uint8_t *restrict dst, const uint8_t *restrict src, uint_fast8_t bits);

#ifdef __cplusplus
}
#endif
#endif /* MBED_TRACE_TEST_STUBS_COMMON_FUNCTIONS_H_ */
