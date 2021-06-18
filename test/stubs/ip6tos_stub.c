// ----------------------------------------------------------------------------
// Copyright 2016-2021 Pelion.
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
#include "ip6string.h"
#include <string.h>
#include <stdio.h>
#include "common_functions.h"

#include "ip6tos_stub.h"

ip6tos_stub_def_t ip6tos_stub; // extern variable

static inline uint8_t context_split_mask(uint_fast8_t split_value)
{
    return (uint8_t) - (0x100u >> split_value);
}

uint8_t *bitcopy(uint8_t *restrict dst, const uint8_t *restrict src, uint_fast8_t bits)
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

uint8_t ip6tos(const void *ip6addr, char *p)
{
    memcpy(ip6tos_stub.input_array, ip6addr, 16);
    strcpy(p, ip6tos_stub.output_string);
    return strlen(p);
}

uint_fast8_t ip6_prefix_tos(const void *prefix, uint_fast8_t prefix_len, char *p)
{
    bitcopy(ip6tos_stub.input_array, prefix, prefix_len);
    strcpy(p, ip6tos_stub.output_string);
    return strlen(p);
}
