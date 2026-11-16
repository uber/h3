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
 * @brief Tests command-line argument parsing.
 */

#include <string.h>

#include "args.h"
#include "test.h"

SUITE(args) {
    TEST(invalidNumericValues) {
        char *values[] = {"", " ", "\t\n", "abc"};
        for (int i = 0; i < 4; i++) {
            int resolution = 7;
            Arg resolutionArg = {.names = {"-r", "--resolution"},
                                 .required = true,
                                 .scanFormat = "%d",
                                 .value = &resolution};
            Arg helpArg = ARG_HELP;
            Arg *args[] = {&resolutionArg, &helpArg};
            char *argv[] = {"testArgs", "-r", values[i]};
            const char *errorMessage = NULL;
            const char *errorDetail = NULL;

            t_assert(_parseArgsList(3, argv, 2, args, &helpArg, &errorMessage,
                                    &errorDetail) != 0,
                     "invalid numeric value fails parsing");
            t_assert(errorMessage != NULL &&
                         strcmp(errorMessage, "Failed to parse argument") == 0,
                     "reports a parse error");
            t_assert(errorDetail != NULL && strcmp(errorDetail, "-r") == 0,
                     "identifies the failed argument");
            t_assert(!resolutionArg.found, "does not mark the argument found");
            t_assert(resolution == 7, "does not overwrite the default value");
        }
    }

    TEST(validNumericValues) {
        char *values[] = {"0", " 2", "-1"};
        int expected[] = {0, 2, -1};
        for (int i = 0; i < 3; i++) {
            int resolution = 7;
            Arg resolutionArg = {.names = {"-r", "--resolution"},
                                 .required = true,
                                 .scanFormat = "%d",
                                 .value = &resolution};
            Arg helpArg = ARG_HELP;
            Arg *args[] = {&resolutionArg, &helpArg};
            char *argv[] = {"testArgs", "-r", values[i]};
            const char *errorMessage = NULL;
            const char *errorDetail = NULL;

            t_assert(_parseArgsList(3, argv, 2, args, &helpArg, &errorMessage,
                                    &errorDetail) == 0,
                     "numeric value parses successfully");
            t_assert(resolutionArg.found, "marks the argument found");
            t_assert(resolution == expected[i], "stores the parsed value");
        }
    }

    TEST(floatingPointValues) {
        char *values[] = {"", " ", "abc", "20.25"};
        for (int i = 0; i < 4; i++) {
            double latitude = 0;
            Arg latitudeArg = {.names = {"--lat"},
                               .required = true,
                               .scanFormat = "%lf",
                               .value = &latitude};
            Arg helpArg = ARG_HELP;
            Arg *args[] = {&latitudeArg, &helpArg};
            char *argv[] = {"testArgs", "--lat", values[i]};
            const char *errorMessage = NULL;
            const char *errorDetail = NULL;

            int result = _parseArgsList(3, argv, 2, args, &helpArg, &errorMessage,
                                        &errorDetail);
            if (i < 3) {
                t_assert(result != 0, "invalid floating-point value fails");
                t_assert(!latitudeArg.found, "does not mark the argument found");
            } else {
                t_assert(result == 0, "valid floating-point value parses");
                t_assert(latitude == 20.25, "stores the parsed value");
            }
        }
    }

    TEST(characterValues) {
        char *values[] = {"", " ", "name"};
        for (int i = 0; i < 3; i++) {
            char name[16] = {0};
            Arg nameArg = {.names = {"--name"},
                           .scanFormat = "%15c",
                           .value = name};
            Arg helpArg = ARG_HELP;
            Arg *args[] = {&nameArg, &helpArg};
            char *argv[] = {"testArgs", "--name", values[i]};
            const char *errorMessage = NULL;
            const char *errorDetail = NULL;

            int result = _parseArgsList(3, argv, 2, args, &helpArg, &errorMessage,
                                        &errorDetail);
            if (i == 0) {
                t_assert(result != 0, "empty character value fails");
                t_assert(!nameArg.found, "does not mark the argument found");
            } else {
                t_assert(result == 0, "nonempty character value parses");
                t_assert(strcmp(name, values[i]) == 0, "preserves whitespace");
            }
        }
    }
}
