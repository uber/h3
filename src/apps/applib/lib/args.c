/*
 * Copyright 2019 Uber Technologies, Inc.
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
/** @file args.c
 * @brief   Miscellaneous useful functions.
 */

#include "args.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "h3api.h"

/*
 * Return codes from parseArgs.
 */

#define PARSE_ARGS_SUCCESS 0
#define PARSE_ARGS_HELP 1
#define PARSE_ARGS_REPEATED_ARGUMENT 2
#define PARSE_ARGS_MISSING_VALUE 3
#define PARSE_ARGS_FAILED_PARSE 4
#define PARSE_ARGS_UNKNOWN_ARGUMENT 5
#define PARSE_ARGS_MISSING_REQUIRED 6

/**
 * Parse command line arguments and prints help, if needed.
 *
 * Uses the provided arguments to populate argument values and records in the
 * argument if it is found.
 *
 * Returns non-zero if all required arguments are not present, an argument fails
 * to parse, is missing its associated value, or arguments are specified more
 * than once.
 *
 * Help is printed to stdout if a argument with isHelp = true is found, and help
 * is printed to stderr if argument parsing fails.
 *
 * @param argc argc from main
 * @param argv argv from main
 * @param numArgs Number of elements in the args array
 * @param args Pointer to each argument to parse
 * @param helpArg Pointer to the argument for "--help"
 * @param helpText Explanatory text for this program printed with help
 * @return 0 if argument parsing succeeded, otherwise non-0. If help is printed,
 * return value is non-0.
 */
int parseArgs(int argc, char* argv[], int numArgs, Arg* args[],
              const Arg* helpArg, const char* helpText) {
    const char* errorMessage = NULL;
    const char* errorDetails = NULL;

    int failed = _parseArgsList(argc, argv, numArgs, args, helpArg,
                                &errorMessage, &errorDetails);

    if (failed || helpArg->found) {
        printHelp(helpArg->found ? stdout : stderr, argv[0], helpText, numArgs,
                  args, errorMessage, errorDetails);
        return failed != PARSE_ARGS_SUCCESS ? failed : PARSE_ARGS_HELP;
    }
    return PARSE_ARGS_SUCCESS;
}

/**
 * Parse command line arguments.
 *
 * Uses the provided arguments to populate argument values.
 *
 * Returns non-zero if all required arguments are not present, an argument fails
 * to parse, is missing its associated value, or arguments are specified more
 * than once.
 *
 * @param argc argc from main
 * @param argv argv from main
 * @param numArgs Number of elements in the args array
 * @param args Pointer to each argument to parse.
 * @param helpArg Pointer to the argument for "--help" that suppresses checking
 * for required arguments.
 * @param errorMessage Error message to display, if returning non-zero.
 * @param errorDetail Additional error details, if returning non-zero. May be
 * null, and may be a pointer from `argv` or `args`.
 * @return 0 if argument parsing succeeded, otherwise non-0.
 */
int _parseArgsList(int argc, char* argv[], int numArgs, Arg* args[],
                   const Arg* helpArg, const char** errorMessage,
                   const char** errorDetail) {
    // Whether help was found and required arguments do not need to be checked
    bool foundHelp = false;

    for (int i = 1; i < argc; i++) {
        bool foundMatch = false;

        for (int j = 0; j < numArgs; j++) {
            // Test this argument, which may have multiple names, for whether it
            // matches. argName will be set to the name used for this argument
            // if it matches.
            const char* argName = NULL;
            for (int k = 0; k < NUM_ARG_NAMES; k++) {
                if (args[j]->names[k] == NULL) continue;

                if (strcmp(argv[i], args[j]->names[k]) == 0) {
                    argName = args[j]->names[k];
                    break;
                }
            }
            // argName unchanged from NULL indicates this didn't match, try the
            // next argument.
            if (argName == NULL) continue;

            if (args[j]->found) {
                *errorMessage = "Argument specified multiple times";
                *errorDetail = argName;
                return PARSE_ARGS_REPEATED_ARGUMENT;
            }

            if (args[j]->scanFormat != NULL) {
                // Argument has a value, need to advance one and read the value.
                i++;
                if (i >= argc) {
                    *errorMessage = "Argument value not present";
                    *errorDetail = argName;
                    return PARSE_ARGS_MISSING_VALUE;
                }

                if (!sscanf(argv[i], args[j]->scanFormat, args[j]->value)) {
                    *errorMessage = "Failed to parse argument";
                    *errorDetail = argName;
                    return PARSE_ARGS_FAILED_PARSE;
                }
            }

            if (args[j] == helpArg) {
                foundHelp = true;
            }

            args[j]->found = true;
            foundMatch = true;
            break;
        }

        if (!foundMatch) {
            *errorMessage = "Unknown argument";
            // Don't set errorDetail, since the input could be unprintable.
            return PARSE_ARGS_UNKNOWN_ARGUMENT;
        }
    }

    // Check for missing required arguments.
    if (!foundHelp) {
        for (int i = 0; i < numArgs; i++) {
            if (args[i]->required && !args[i]->found) {
                *errorMessage = "Required argument missing";
                *errorDetail = args[i]->names[0];
                return PARSE_ARGS_MISSING_REQUIRED;
            }
        }
    }

    return PARSE_ARGS_SUCCESS;
}

/**
 * Print a help message.
 *
 * @param out Stream to print to, e.g. stdout
 * @param programName Program name, such as from argv[0]
 * @param helpText Explanation of what the program does
 * @param numArgs Number of arguments to print help for
 * @param args Pointer to arguments to print help for
 * @param errorMessage Error message, or null
 * @param errorDetails Additional error detail message, or null
 */
void printHelp(FILE* out, const char* programName, const char* helpText,
               int numArgs, Arg* args[], const char* errorMessage,
               const char* errorDetails) {
    if (errorMessage != NULL) {
        fprintf(out, "%s: %s", programName, errorMessage);
        if (errorDetails != NULL) {
            fprintf(out, ": %s", errorDetails);
        }
        fprintf(out, "\n");
    }
    fprintf(out, "%s: %s\n", programName, helpText);
    fprintf(out, "H3 %d.%d.%d\n\n", H3_VERSION_MAJOR, H3_VERSION_MINOR,
            H3_VERSION_PATCH);

    for (int i = 0; i < numArgs; i++) {
        fprintf(out, "\t");
        for (int j = 0; j < NUM_ARG_NAMES; j++) {
            if (args[i]->names[j] == NULL) continue;
            if (j != 0) fprintf(out, ", ");
            fprintf(out, "%s", args[i]->names[j]);
        }
        if (args[i]->scanFormat != NULL) {
            fprintf(out, " <%s>", args[i]->valueName);
        }
        fprintf(out, "\t");
        if (args[i]->required) {
            fprintf(out, "Required. ");
        }
        fprintf(out, "%s\n", args[i]->helpText);
    }
}
