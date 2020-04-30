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
/** @file args.h
 * @brief   Miscellaneous functions and constants.
 */

#ifndef ARGS_H
#define ARGS_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include "utility.h"

/** Maximum number of names an argument may have. */
#define NUM_ARG_NAMES 2

/**
 * An argument accepted by on the command line of an H3 application. Specifies
 * how the argument is presented, parsed, and where parsed values are stored.
 */
typedef struct {
    /**
     * Both short and long names of the argument. A name may be null, but the
     * first name must be non-null.
     */
    const char* const names[NUM_ARG_NAMES];

    /**
     * If true, this argument must be specified. If the argument is not
     * specified, argument parsing will fail.
     */
    const bool required;

    /**
     * Scan format for the argument, which will be passed to sscanf. May be null
     * to indicate the argument does not take a value.
     */
    const char* const scanFormat;

    /**
     * Name to present the value as when printing help.
     */
    const char* const valueName;

    /**
     * Value will be placed here if the argument is present and scanFormat is
     * not null.
     */
    void* const value;

    /**
     * Will be set to true if the argument is present. Should be false when
     * passed in to parseArgs.
     */
    bool found;

    /**
     * Help text for this argument.
     */
    const char* const helpText;
} Arg;

// prototypes

int parseArgs(int argc, char* argv[], int numArgs, Arg* args[],
              const Arg* helpArg, const char* helpText);
void printHelp(FILE* out, const char* programName, const char* helpText,
               int numArgs, Arg* args[], const char* errorMessage,
               const char* errorDetails);

int _parseArgsList(int argc, char* argv[], int numArgs, Arg* args[],
                   const Arg* helpArg, const char** errorMessage,
                   const char** errorDetail);

// common arguments

#define ARG_HELP \
    { .names = {"-h", "--help"}, .helpText = "Show this help message." }
#define DEFINE_INDEX_ARG(varName, argName) \
    H3Index varName = 0;                   \
    Arg argName = {                        \
        .names = {"-i", "--index"},        \
        .scanFormat = "%" PRIx64,          \
        .valueName = "index",              \
        .value = &varName,                 \
        .helpText =                        \
            "Index, or not specified to read indexes from standard input."}
#define ARG_KML \
    { .names = {"-k", "--kml"}, .helpText = "Print output in KML format." }
#define DEFINE_KML_NAME_ARG(varName, argName)      \
    char varName[BUFF_SIZE] = {0};                 \
    Arg argName = {                                \
        .names = {"--kn", "--kml-name"},           \
        .scanFormat = "%255c", /* BUFF_SIZE - 1 */ \
        .valueName = "name",                       \
        .value = &varName,                         \
        .helpText = "Text for the KML name tag, if --kml is specified."}
#define DEFINE_KML_DESC_ARG(varName, argName)      \
    char varName[BUFF_SIZE] = {0};                 \
    Arg argName = {                                \
        .names = {"--kd", "--kml-description"},    \
        .scanFormat = "%255c", /* BUFF_SIZE - 1 */ \
        .valueName = "description",                \
        .value = &varName,                         \
        .helpText =                                \
            "Text for the KML description tag, if --kml is specified."}

#endif
