/*
 * Copyright (c) 2016 ARM Limited. All rights reserved.
 */
#ifndef MBED_TRACE_TEST_STUBS_IP6STRING_H_
#define MBED_TRACE_TEST_STUBS_IP6STRING_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

extern const char *ip6tos_output_string;
extern uint8_t ip6tos_input_array[16];

void ip6tos(const void *ip6addr, char *p);

#ifdef __cplusplus
}
#endif
#endif /* MBED_TRACE_TEST_STUBS_IP6STRING_H_ */
