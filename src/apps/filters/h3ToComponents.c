/*
 * Copyright 2016-2017 Uber Technologies, Inc.
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
 * @brief stdin/stdout filter that converts from integer H3 indexes to
 * components.
 *
 *  usage: `h3ToComponents`
 *
 *  The program reads H3 indexes from stdin until EOF and outputs the
 *  corresponding component strings to stdout.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "baseCells.h"
#include "h3Index.h"
#include "utility.h"

/**
 * Converts an integer H3 digit to the corresponding character.
 *
 * @param d The integer H3 digit (0-7).
 * @return The corresponding character string (or 'x' if invalid).
 */
char resDigitToChar(int d) {
    if (d < 0 || d > 7) {
        return 'x';
    }

    return '0' + d;
}

void doCell(H3Index h, int verboseMode) {
    H3IndexFat hf;
    h3ToH3Fat(h, &hf);
    if (verboseMode == 0) {
        if (hf.mode == H3_HEXAGON_MODE) {
            printf("%d:%d:%d:", hf.mode, hf.res, hf.baseCell);
            for (int i = 0; i < hf.res; i++) {
                printf("%c", resDigitToChar(hf.index[i]));
            }
            printf("\n");
        } else if (hf.mode == H3_UNIEDGE_MODE) {
            printf("%d:%d:%d:%d:", hf.mode, H3_GET_RESERVED_BITS(h), hf.res,
                   hf.baseCell);
            for (int i = 0; i < hf.res; i++) {
                printf("%c", resDigitToChar(hf.index[i]));
            }
            printf("\n");
        } else {
            printf("INVALID INDEX\n");
        }
    } else {
        char* modes[] = {
            "RESERVED",             // 0
            "Hexagon",              // 1
            "Unidirectional Edge",  // 2
            "INVALID",              // 3
            "INVALID",              // 4
            "INVALID",              // 5
            "INVALID",              // 6
            "INVALID",              // 7
            "INVALID",              // 8
            "INVALID",              // 9
            "INVALID",              // 10
            "INVALID",              // 11
            "INVALID",              // 12
            "INVALID",              // 13
            "INVALID",              // 14
            "INVALID",              // 15
        };
        printf("╔════════════╗\n");
        char hStr[BUFF_SIZE];
        H3_EXPORT(h3ToString)(h, hStr, BUFF_SIZE);
        printf("║ H3Index    ║ %s\n", hStr);
        printf("╠════════════╣\n");
        printf("║ Mode       ║ %s (%i)\n", modes[hf.mode], hf.mode);
        printf("║ Resolution ║ %i\n", hf.res);
        if (hf.mode == H3_UNIEDGE_MODE) {
            printf("║ Edge       ║ %i\n", H3_GET_RESERVED_BITS(h));
        }
        printf("║ Base Cell  ║ %i\n", hf.baseCell);
        for (int i = 0; i < hf.res; i++) {
            printf("║%3i Child   ║ %c\n", i + 1, resDigitToChar(hf.index[i]));
        }
        printf("╚════════════╝\n\n");
    }
}

int main(int argc, char* argv[]) {
    // check command line args
    int verboseMode = 0;
    if (argc > 1) {
        if (strncmp("--verbose", argv[1], 9) == 0) {
            verboseMode = 1;
        } else {
            fprintf(stderr, "usage: %s [--verbose]\n", argv[0]);
            exit(1);
        }
    }

    // process the indexes on stdin
    char buff[BUFF_SIZE];
    while (1) {
        // get an index from stdin
        if (!fgets(buff, BUFF_SIZE, stdin)) {
            if (feof(stdin))
                break;
            else
                error("reading H3 index from stdin");
        }

        H3Index h3 = H3_EXPORT(stringToH3)(buff);
        doCell(h3, verboseMode);
    }
}
