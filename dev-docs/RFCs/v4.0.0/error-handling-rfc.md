# RFC: Standardize Error Handling

* **Authors**: Isaac Brodsky (@isaacbrodsky)
* **Date**: June 19, 2020
* **Status**: Accepted

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

err = latLngToCell(lat, lng, res, &result);
if (err) {
    fprintf(stderr, "Error: %d", err);
}
```

Sample with errors discarded:
```
H3Index result;

latLngToCell(lat, lng, res, &result);
```

Placing error information in the return of function calls requires that functions return two pieces of information - the error status, and the requested data. This approach does make it clear that a status code is returned and that may make it more usual that a consumer of the API does check the error status.

### Return code + "unsafe version"

Sample:
```
H3Error err;
H3Index result;

err = latLngToCell(lat, lng, res, &result);
if (err) {
    fprintf(stderr, "Error: %d", err);
}
```

Sample with errors discarded:
```
H3Index result;

result = unsafe_latLngToCell(lat, lng, res);
```

This approach is the same as using return codes, but offers an "unsafe" version of some functions where there is less or no error information returned. This is intended to be used in performance critical cases where the inputs are known to be suitable ahead of time, or the caller would not do anything with the error information anyways.

### Return code + "GetError"

Sample:
```
H3Error err;
H3Index result;

err = latLngToCell(lat, lng, res, &result);
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

result = latLngToCell(lat, lng, res);
if (err = h3GetError()) {
    fprintf(stderr, "Error: %d", err);
}
```

Sample with errors discarded:
```
H3Index result;

result = latLngToCell(lat, lng, res);
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

result = latLngToCell(lat, lng, res, &err);
if (err) {
    fprintf(stderr, "Error: %d", err);    
}
```

Sample with errors discarded:
```
H3Index result;
result = latLngToCell(lat, lng, res, NULL);
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
    result = latLngToCell(lat, lng, res, env);
}
```

It will be very difficult to implement error handling using `setjmp`. There are several main problems:

1. H3 is designed for its API to be bound to other languages, but other languages generally do not support `setjmp`.
2. `setjmp` requires passing an environment reference anyways, which could also carry error information.

## Proposal

### Consistency

It would be good to use a single pattern for error handling, so that callers do not need to reference documentation, or determine how to handle different types of error reporting in their environment.

### Return Code

It is proposed that errors will be indicated by the return code of the function. Any public function that can return an
error (for example, due to domain issues in its input) will return an error code.

The signature for `latLngToCell` will look like:
```
H3Error latLngToCell(double lat, double lng, int res, H3Index *result);
```

### Is Valid Functions

There are two types of functions returning booleans in the library:

* Validation functions, where the return code can be replaced with an error code. Example: `h3IsValid`
* Property inspection functions, where the return code would be ambiguous between invalid and the property being false. Example: `h3IsResClassIII`

### H3Error type

(Discussion: It may be nice to have an enum for this type.)

This section will explicitly be part of the public API for semantic versioning of the H3 library.

The proposal is to use the 32-bit option, below.

#### 64-bit option

The error type will be an integer, with the following properties:

* `H3Error` will be an integer type of 64 bits, i.e. `uint64_t`.
* `H3Error` with value 0 indicates success (no error).
* No `H3Error` value will set the most significant bit.
* No `H3Error` value will set the bits that correspond with the **Mode** bit field in an `H3Index`.

64 bits is unlikely to be needed by `H3Error`, but allows for explicitly mixing error codes and resulting indexes if desired by the application.

The C definition of the type will be:

```
typedef uint64_t H3Error;
```

#### 32-bit option

The error type will be an integer, with the following properties:

* `H3Error` will be an integer type of 32 bits, i.e. `uint32_t`.
* `H3Error` with value 0 indicates success (no error).
* No `H3Error` value will set the most significant bit.
* As a result of these properties, no `H3Error` value will set the bits that correspond with the **Mode** bit field in an `H3Index`.

32 bit return codes with the high bit never set allows for mixing error codes and resulting indexes if desired by the application, after copying the error codes into the result buffer.

The C definition of the type will be:

```
typedef uint32_t H3Error;
```

### H3Error values

Error codes, other than the success code, can either be sequentially numbered, or can have additional structure encoded (for example, bit 2 being set could always indicate an argument domain error.)

SQLite's approach to this is to define broad error categories (in 8 bits), and then allow functions to provide additional error information in the remaining bits.

#### Table of error codes

| Value | Name                 | Description
| ----- | -------------------- | -----------
| 0     | E_SUCCESS            | Success (no error)
| 1     | E_FAILED             | The operation failed but a more specific error is not available
| 2     | E_DOMAIN             | Argument was outside of acceptable range (when a more specific error code is not available)
| 3     | E_LATLNG_DOMAIN      | Latitude or longitude arguments were outside of acceptable range
| 4     | E_RES_DOMAIN         | Resolution argument was outside of acceptable range
| 5     | E_CELL_INVALID       | `H3Index` cell argument was not valid
| 6     | E_DIR_EDGE_INVALID   | `H3Index` directed edge argument was not valid
| 7     | E_UNDIR_EDGE_INVALID | `H3Index` undirected edge argument was not valid
| 8     | E_VERTEX_INVALID     | `H3Index` vertex argument was not valid
| 9     | E_PENTAGON           | Pentagon distortion was encountered which the algorithm could not handle it
| 10    | E_DUPLICATE_INPUT    | Duplicate input was encountered in the arguments and the algorithm could not handle it
| 11    | E_NOT_NEIGHBORS      | `H3Index` cell arguments were not neighbors
| 12    | E_RES_MISMATCH       | `H3Index` cell arguments had incompatible resolutions
| 13    | E_MEMORY_ALLOC       | Necessary memory allocation failed
| 14    | E_MEMORY_BOUNDS      | Bounds of provided memory were not large enough

The H3 library may always add additional error messages. Error messages not recognized by the application should be treated as `E_FAILED`. The latest version of this table is available [in the documentation](https://h3geo.org/docs/next/library/errors#table-of-error-codes).

#### Example error code results

```
latLngToCell(lat=Infinity, lng=0, res=0, &out) => E_LATLNG_DOMAIN
latLngToCell(lat=0, lat=0, res=-1, &out) => E_RES_DOMAIN
cellToLatLng(index=0, &out) => E_CELL_INVALID
h3IsResClassIII(index=RES_0_INDEX, &out) => E_SUCCESS
h3IsResClassIII(index=RES_1_INDEX, &out) => E_SUCCESS
h3IsResClassIII(index=0, &out) => E_CELL_INVALID
maxKringSize(k=-1) => E_DOMAIN
hexRange(origin=0, k=0, &out) => E_CELL_INVALID
hexRange(origin=AN_INDEX, k=-1, &out) => E_DOMAIN
hexRange(origin=PENTAGON_INDEX, k=1, &out) => E_PENTAGON
# Failed to allocate internal buffer:
kRing(origin=AN_INDEX, k=1, &out) => E_MEMORY_ALLOC
distanceRads({Infinity, Infinity}, {0, 0}, &out) => E_LATLNG_DOMAIN
hexAreaKm2(res=-1) => E_RES_DOMAIN
# Cannot parse:
stringToH3(str="zzzz", &out) => E_FAILED
# Output buffer is not large enough:
h3ToString(index=AN_INDEX, &out, sizeof(out)) => E_FAILED
h3IsValid(index=AN_INDEX) => E_SUCCESS
h3IsValid(index=0) => E_CELL_INVALID
compact({AN_INDEX, AN_INDEX}, &out, numHexes) => E_DUPLICATE_INPUT
getOriginH3IndexFromUnidirectionalEdge(edge=AN_INDEX, &out) => E_EDGE_INVALID
h3Distance(AN_INDEX, AN_INDEX_FAR_AWAY, &out) => E_FAILED
h3IndexesAreNeighbors(AN_INDEX, A_NEIGHBOR) => E_SUCCESS
h3IndexesAreNeighbors(AN_INDEX, AN_INDEX_FAR_AWAY) => E_NOT_NEIGHBORS
h3IndexesAreNeighbors(AN_INDEX, AN_INDEX_WRONG_RES) => E_RES_MISMATCH
getH3UnidirectionalEdge(AN_INDEX, AN_INDEX_FAR_AWAY, &out) => E_NOT_NEIGHBORS
```

### Bindings

Bindings should translate error codes into the error handling mechanism appropriate to their language. For example, Java will convert error codes into Java Exceptions.

When possible, it is preferable to retain the error code. When this is not possible it is fine to elide them. Language bindings should include error messages that are formatted as is usual in their language. An example in Java could be:

```
public class H3Exception extends Exception {
    private final long errorCode;

    public H3Exception(long errorCode) {
        super(messageFromErrorCode(errorCode));
        this.errorCode = errorCode;
    }

    public long getErrorCode() {
        return errorCode;
    }

    public static String messageFromErrorCode(long errorCode) {
        switch (errorCode) {
            case 4: // E_RES_INVALID
                return "Resolution arugment was invalid";
            // elided ...
        }
    }
}
``
