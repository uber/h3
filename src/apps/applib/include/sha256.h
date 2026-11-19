/*
 * Copyright 2026 Uber Technologies, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/** @file sha256.h
 * @brief SHA-256 (FIPS 180-4), used to hash conformance suite files and
 * digest streams.
 */

#ifndef SHA256_H
#define SHA256_H

#include <stddef.h>
#include <stdint.h>

/* Sha256 is a streaming SHA-256 state: the chaining value, the bytes not yet
 * forming a block, and the total length. */
typedef struct {
    uint32_t state[8];
    uint8_t buffer[64];
    size_t buffered;
    uint64_t total;
} Sha256;

/** Starts a digest. */
void sha256Init(Sha256 *ctx);
/** Absorbs len bytes of data. */
void sha256Update(Sha256 *ctx, const uint8_t *data, size_t len);
/** Finishes the digest and writes it as 64 lowercase hex characters plus a
 * terminating NUL. The context must not be reused afterwards. */
void sha256Final(Sha256 *ctx, char hex[65]);
/** Digests data in one call; see sha256Final for the output. */
void sha256Hex(const uint8_t *data, size_t len, char hex[65]);

#endif
