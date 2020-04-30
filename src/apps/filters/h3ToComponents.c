/*
 * Copyright 2016-2017, 2019 Uber Technologies, Inc.
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
 *  See `h3ToComponents --help` for usage.
 *
 *  The program reads H3 indexes from stdin until EOF and outputs the
 *  corresponding component strings to stdout.
 */

#include <inttypes.h>

#include "args.h"
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

void doCell(H3Index h, bool verboseMode) {
    int h3Mode = H3_GET_MODE(h);
    int h3Res = H3_GET_RESOLUTION(h);
    int h3BaseCell = H3_GET_BASE_CELL(h);
    if (verboseMode) {
        const char* modes[] = {
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
        printf("║ Mode       ║ %s (%i)\n", modes[h3Mode], h3Mode);
        printf("║ Resolution ║ %i\n", h3Res);
        if (h3Mode == H3_UNIEDGE_MODE) {
            printf("║ Edge       ║ %i\n", H3_GET_RESERVED_BITS(h));
        }
        printf("║ Base Cell  ║ %i\n", h3BaseCell);
        for (int i = 1; i <= h3Res; i++) {
            printf("║%3i Child   ║ %c\n", i,
                   resDigitToChar(H3_GET_INDEX_DIGIT(h, i)));
        }
        printf("╚════════════╝\n\n");
    } else {
        if (h3Mode == H3_HEXAGON_MODE) {
            printf("%d:%d:%d:", h3Mode, h3Res, h3BaseCell);
            for (int i = 1; i <= h3Res; i++) {
                printf("%c", resDigitToChar(H3_GET_INDEX_DIGIT(h, i)));
            }
            printf("\n");
        } else if (h3Mode == H3_UNIEDGE_MODE) {
            printf("%d:%d:%d:%d:", h3Mode, H3_GET_RESERVED_BITS(h), h3Res,
                   h3BaseCell);
            for (int i = 1; i <= h3Res; i++) {
                printf("%c", resDigitToChar(H3_GET_INDEX_DIGIT(h, i)));
            }
            printf("\n");
        } else {
            printf("INVALID INDEX\n");
        }
    }
}

int main(int argc, char* argv[]) {
    Arg helpArg = ARG_HELP;
    Arg verboseArg = {.names = {"-v", "--verbose"},
                      .helpText = "Verbose output mode."};
    DEFINE_INDEX_ARG(index, indexArg);
    const int numArgs = 3;
    Arg* args[] = {&helpArg, &verboseArg, &indexArg};

    if (parseArgs(argc, argv, numArgs, args, &helpArg,
                  "Converts H3 indexes to component parts")) {
        return helpArg.found ? 0 : 1;
    }

    if (indexArg.found) {
        doCell(index, verboseArg.found);
    } else {
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
            doCell(h3, verboseArg.found);
        }
    }
}
