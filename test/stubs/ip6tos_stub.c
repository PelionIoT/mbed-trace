/*
 * Copyright (c) 2014-2015 ARM Limited. All rights reserved.
 */
#include "ip6string_stub.h"
#include <string.h>
#include <stdio.h>

const char *ip6tos_output_string;
extern uint8_t ip6tos_input_array[16];

void ip6tos(const void *ip6addr, char *p)
{
    memcpy(ip6tos_input_array, ip6addr, 16);
    strcpy(p, ip6tos_output_string);
}
