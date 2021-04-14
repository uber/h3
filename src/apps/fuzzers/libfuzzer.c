/*
 * Copyright 2021 Uber Technologies, Inc.
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

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "h3api.h"
#include "utility.h"

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    char *new_str = (char *)malloc(size+1);
    if (new_str == NULL){
        return 0;
    }
    memcpy(new_str, data, size);
    new_str[size] = '\0';

    
    H3Index h3;
    h3 = H3_EXPORT(stringToH3)(new_str);
    
    H3Index input[] = {h3, h3};
    int inputSize = sizeof(input) / sizeof(H3Index);

    // fuzz compactCells
    H3Index* compacted = calloc(inputSize, sizeof(H3Index));
    int err = compactCells(input, compacted, inputSize);
    
    // fuzz uncompactCells
    int compactedCount = 0;
    for (int i = 0; i < inputSize; i++) {
        if (compacted[i] != 0) {
            compactedCount++;
        }
    }
    if(uncompactedSize<10) {
        int uncompactRes = 10;
        int uncompactedSize =
            maxUncompactCellsSize(compacted, inputSize, uncompactRes);
        
        H3Index* uncompacted = calloc(uncompactedSize, sizeof(H3Index));
        int err2 = uncompactCells(compacted, compactedCount, uncompacted,
                              uncompactedSize, uncompactRes);
        free(uncompacted);
    }
    
    // fuzz h3NeighborRotations 
    int rotations = 0;
    h3NeighborRotations(h3, CENTER_DIGIT, &rotations);

    free(compacted);    
    free(new_str);
    return 0;
}
