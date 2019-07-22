# Copyright (c) 2019 ARM Limited
# SPDX-License-Identifier: Apache-2.0
valgrind --leak-check=yes --error-exitcode=1 ./app
