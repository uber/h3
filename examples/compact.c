/*
 * Copyright 2018 Uber Technologies, Inc.
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
/**
 * Example program that compacts a set of indexes, and then uncompacts
 * to the original set.
 */

#include <assert.h>
#include <h3/h3api.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    H3Index input[] = {
        // All with the same parent index
        0x8a2a1072b587fffL, 0x8a2a1072b5b7fffL, 0x8a2a1072b597fffL,
        0x8a2a1072b59ffffL, 0x8a2a1072b58ffffL, 0x8a2a1072b5affffL,
        0x8a2a1072b5a7fffL,
        // These don't have the same parent indexas above.
        0x8a2a1070c96ffffL, 0x8a2a1072b4b7fffL, 0x8a2a1072b4a7fffL};
    int inputSize = sizeof(input) / sizeof(H3Index);

    H3Index* compacted = calloc(inputSize, sizeof(H3Index));
    // neighboring is the input set to be compacted
    int err = compact(input, compacted, inputSize);
    assert(err == 0);

    int count = 0;
    printf("Compacted:\n");
    for (int i = 0; i < inputSize; i++) {
        if (compacted[i] != 0) {
            printf("%llx\n", compacted[i]);
            count++;
        }
    }
    printf("Compacted to %d indexes.\n", count);

    int uncompactRes = 10;
    int uncompactedSize = maxUncompactSize(compacted, inputSize, uncompactRes);
    H3Index* uncompacted = calloc(uncompactedSize, sizeof(H3Index));
    int err2 =
        uncompact(compacted, count, uncompacted, uncompactedSize, uncompactRes);
    assert(err2 == 0);

    printf("Uncompacted:\n");
    for (int i = 0; i < uncompactedSize; i++) {
        if (uncompacted[i] != 0) {
            printf("%llx\n", uncompacted[i]);
        }
    }

    free(compacted);
    free(uncompacted);
}
