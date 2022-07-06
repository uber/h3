---
id: errors
title: Error handling
sidebar_label: Error handling
slug: /library/errors
---

H3 does it's best to be robust to system failures or unexpected inputs, but
some times these cannot be recovered from. H3's way of doing this is to return
an error code to the caller.

## Example

This code example checks for an error when calling an H3 function and prints a message if the call did not succeed:

```c
H3Error err;
H3Index result;

err = latLngToCell(lat, lng, res, &result);
if (err) {
    fprintf(stderr, "Error: %d", err);
}
```

## H3Error type

The type returned by most H3 functions is `H3Error`, a 32 bit integer type with the following properties:

* `H3Error` will be an integer type of 32 bits, i.e. `uint32_t`.
* `H3Error` with value 0 indicates success (no error).
* No `H3Error` value will set the most significant bit.
* As a result of these properties, no `H3Error` value will set the bits that correspond with the **Mode** bit field in an `H3Index`.

32 bit return codes with the high bit never set allows for mixing error codes and resulting indexes if desired by the application, after copying the error codes into the result buffer.

## Table of error codes

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
| 15    | E_OPTION_INVALID     | Mode or flags argument was not valid

The H3 library may always add additional error messages. Error messages not recognized by the application should be treated as `E_FAILED`.

### Bindings

Bindings translate error codes into the error handling mechanism appropriate to their language. For example, Java will convert error codes into Java Exceptions.

When possible, it is preferable to retain the error code. When this is not possible it is fine to elide them. Language bindings should include error messages that are formatted as is usual in their language.

## References

* [Technical RFC on error handling](https://github.com/uber/h3/blob/master/dev-docs/RFCs/v4.0.0/error-handling-rfc.md)
