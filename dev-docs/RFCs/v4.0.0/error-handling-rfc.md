# RFC: Standardize Error Handling

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

* [Cython](https://cython.readthedocs.io/en/latest/src/userguide/language_basics.html#error-return-values)
* [SQLite](https://sqlite.org/rescode.html)
* [SDL](https://wiki.libsdl.org/SDL_GetError)
* [libpng](http://www.libpng.org/pub/png/libpng-1.4.0-manual.pdf)

## Approaches

### Return code

Sample:
```
H3Error err;
H3Index result;

err = geoToH3(lat, lon, res, &result);
if (err) {
    fprintf(stderr, "Error: %d", err);
}
```

Sample with errors discarded:
```
H3Index result;

geoToH3(lat, lon, res, &result);
```

Placing error information in the return of function calls requires that functions return two pieces of information - the error status, and the requested data. This approach does make it clear that a status code is returned and that may make it more usual that a consumer of the API does check the error status.

### Return code + "unsafe version"

Sample:
```
H3Error err;
H3Index result;

err = geoToH3(lat, lon, res, &result);
if (err) {
    fprintf(stderr, "Error: %d", err);
}
```

Sample with errors discarded:
```
H3Index result;

result = unsafe_geoToH3(lat, lon, res);
```

This approach is the same as using return codes, but offers an "unsafe" version of some functions where there is less or no error information returned. This is intended to be used in performance critical cases where the inputs are known to be suitable ahead of time, or the caller would not do anything with the error information anyways.

### Return code + "GetError"

Sample:
```
H3Error err;
H3Index result;

err = geoToH3(lat, lon, res, &result);
if (err) {
    const char* msg = h3GetError();
    fprintf(stderr, "Error: %d (%s)", err, msg);
}
```

This is an extension to the return code approach where the library provides additional information through a "GetError" style function. This is often used to provide an English language name for the error such as "I/O Error", "Latitude out of range", "Success", etc.

Unless the error information returned from the function contains enough information, this approach can only return additional information about the last error, and has similar thread safety concerns as _GetError only_, below.

### GetError only

Sample:
```
H3Error err;
H3Index result;

result = geoToH3(lat, lon, res);
if (err = h3GetError()) {
    fprintf(stderr, "Error: %d", err);
}
```

Sample with errors discarded:
```
H3Index result;

result = geoToH3(lat, lon, res);
```

In this approach, the data requested is returned from the function, and the consumer is responsible for knowing when to check for errors, and how to handle them. There are a few drawbacks:

1. Consumers must reference documentation (not the function signature) to know when to check for errors.
2. [C11 has thread local storage](https://en.wikipedia.org/wiki/Thread-local_storage#C_and_C++), but this may complicate build processes to support pre-C11 compilers.
3. Unless sufficient information is provided about the error (via a return code, above), GetError can only return information about the most recent error.

### Error parameter by reference

Sample:
```
H3Error err;
H3Index result;

result = geoToH3(lat, lon, res, &err);
if (err) {
    fprintf(stderr, "Error: %d", err);    
}
```

Sample with errors discarded:
```
H3Index result;
result = geoToH3(lat, lon, res, NULL);
```

This is an inversion of the return code approach, where the error status is the reference parameter. This offers an advantage that a caller could choose to disregard the error status (for example by passing NULL)

### setjmp/longjmp

Sample:
```
H3Index result;
jmp_buf env;
int err;

if (err = setjmp(env)) {
    fprintf(stderr, "Error: %d", err);
} else {
    result = geoToH3(lat, lon, res, env);
}
```

It will be very difficult to implement error handling using `setjmp`. There are several main problems:

1. H3 is designed for its API to be bound to other languages, but other languages generally do not support `setjmp`.
2. `setjmp` requires passing an environment reference anyways, which could also carry error information.

## Proposal

### Consistency

It would be good to use a single pattern for error handling, so that callers do not need to reference documentation, or determine how to handle different types of error reporting in their environment.

