# RFC: Standardize Return Codes

* **Authors**: Isaac Brodsky (@isaacbrodsky)
* **Date**: June 19, 2020
* **Status**: Draft

## Overview

This is a proposal for changing H3 function signatures (the type of the value
returned by the function) to indicate error cases.

## Motivation

H3 does it's best to be robust to system failures or unexpected inputs, but
some times these cannot be recovered from. When that happens, the application
calling H3 needs to be aware of the problem so it can be handled and the
user alerted to the issue.

One of the main impediments with this happening currently is the use of `void` as
the return type for a number of H3 functions. This precludes the ability to use
the return value for indicating an error. In many cases, the outputs of the function
cannot indicate an error case versus a case with no output.

This proposal is to change H3 functions that can experience internal errors
or invalid inputs to be able to indicate this to the calling program, in a
consistent and useful way.

## Prior Art

Reading materials to reference:

* [SQLite](https://sqlite.org/rescode.html)
* [SDL](https://wiki.libsdl.org/SDL_GetError)
* [libpng](http://www.libpng.org/pub/png/libpng-1.4.0-manual.pdf)

## Approaches

### Return code

### Return code + "GetError"

### setjmp/longjmp

## Proposal
