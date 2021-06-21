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
#ifndef MBED_TRACE_TEST_STUBS_IP6STRING_H_
#define MBED_TRACE_TEST_STUBS_IP6STRING_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

typedef struct {
    const char *output_string;
    uint8_t input_array[16];
} ip6tos_stub_def_t;

extern ip6tos_stub_def_t ip6tos_stub;

#ifdef __cplusplus
}
#endif
#endif /* MBED_TRACE_TEST_STUBS_IP6STRING_H_ */
