/*
 * Copyright 2016-2019 Uber Technologies, Inc.
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
/** @file utility.h
 * @brief   Miscellaneous functions and constants.
 */

#ifndef UTILITY_H
#define UTILITY_H

#include <stdio.h>
#include "coordijk.h"
#include "h3api.h"

#define BUFF_SIZE 256

/** Macro: Get the size of a fixed-size array */
#define ARRAY_SIZE(x) sizeof(x) / sizeof(x[0])

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

void error(const char* msg);
void h3Print(H3Index h);    // prints as integer
void h3Println(H3Index h);  // prints as integer

void coordIjkPrint(const CoordIJK* c);

void geoToStringRads(const GeoCoord* p, char* str);
void geoToStringDegs(const GeoCoord* p, char* str);
void geoToStringDegsNoFmt(const GeoCoord* p, char* str);

void geoPrint(const GeoCoord* p);
void geoPrintln(const GeoCoord* p);
void geoPrintNoFmt(const GeoCoord* p);
void geoPrintlnNoFmt(const GeoCoord* p);
void geoBoundaryPrint(const GeoBoundary* b);
void geoBoundaryPrintln(const GeoBoundary* b);
int readBoundary(FILE* f, GeoBoundary* b);

void randomGeo(GeoCoord* p);

void iterateAllIndexesAtRes(int res, void (*callback)(H3Index));
void iterateAllIndexesAtResPartial(int res, void (*callback)(H3Index),
                                   int maxBaseCell);

int _parseArgsList(int argc, char* argv[], int numArgs, Arg* args[],
                   const Arg* helpArg, const char** errorMessage,
                   const char** errorDetail);

#endif
