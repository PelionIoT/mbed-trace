/*
 * Copyright (c) 2016 ARM Limited. All rights reserved.
 */
#include "common_functions_stub.h"
#include <stdio.h>

#include <string.h>

uint8_t *bitcopy(uint8_t *restrict dst, const uint8_t *restrict src, uint_fast8_t bits)
{
    memcpy(dst, src, bits/8);
    return dst+bits/8;
}
