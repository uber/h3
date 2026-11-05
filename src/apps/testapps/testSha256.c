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
/** @file
 * @brief Known-answer tests for applib's SHA-256 (FIPS 180-4 examples and
 * NIST CAVS vectors), which the conformance suite's manifest and digests
 * depend on.
 */

#include <string.h>

#include "sha256.h"
#include "test.h"

static void assertHex(const char *message, size_t len, const char *want) {
    char hex[65];
    sha256Hex((const uint8_t *)message, len, hex);
    t_assert(strcmp(hex, want) == 0, "one-call digest");
}

SUITE(sha256) {
    TEST(empty) {
        assertHex(
            "", 0,
            "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    }

    TEST(abc) {
        assertHex(
            "abc", 3,
            "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    }

    TEST(twoBlocks) {
        // 56 bytes: the padding spills into a second block.
        assertHex(
            "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", 56,
            "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");
    }

    TEST(millionA) {
        Sha256 ctx;
        sha256Init(&ctx);
        char chunk[1000];
        memset(chunk, 'a', sizeof chunk);
        for (int i = 0; i < 1000; i++) {
            sha256Update(&ctx, (const uint8_t *)chunk, sizeof chunk);
        }
        char hex[65];
        sha256Final(&ctx, hex);
        t_assert(strcmp(hex,
                        "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d3"
                        "9ccc7112cd0") == 0,
                 "streamed digest of one million 'a'");
    }

    TEST(streamingMatchesOneCall) {
        // Feeding the same bytes in uneven pieces must not change the digest.
        const char *text =
            "The quick brown fox jumps over the lazy dog; the quick brown fox "
            "jumps over the lazy dog again, past a block boundary.";
        size_t len = strlen(text);
        char whole[65];
        sha256Hex((const uint8_t *)text, len, whole);
        for (size_t split = 0; split <= len; split += 7) {
            Sha256 ctx;
            sha256Init(&ctx);
            sha256Update(&ctx, (const uint8_t *)text, split);
            sha256Update(&ctx, (const uint8_t *)text + split, len - split);
            char hex[65];
            sha256Final(&ctx, hex);
            t_assert(strcmp(hex, whole) == 0, "split digest");
        }
    }
}
