# RFC: Overrideable allocation functions for H3

* **Authors**: Isaac Brodsky (@isaacbrodsky)
* **Date**: February 7, 2020
* **Status**: Implemented

## Overview

This is a proposal for adding a mechanism for users of the H3 library to provide heap allocator instead of the default
malloc implementation.

## Motivation

This will address the following use cases:

* H3 is used inside of another application which has its own heap management scheme. For example, using the allocation
  functions provided by Postgres or the Java Virtual Machine.
* Testing of failure cases of H3, by simulating allocation failures.

Most H3 functions accept memory from the caller in order to avoid this problem. This will still be the preferred way
to handle memory management in H3. Stack allocation is avoided because H3 cannot know whether there is sufficient stack
memory available. (Note that `_kRingInternal`/`kRingDistances` implicitly uses stack allocation because it implements DFS
recursively.)

A few functions in H3 do heap allocate memory because it is not feasible to do otherwise, or as a convenience. The
functions that heap allocate are:

| Function | Reason
| --- | ---
| `kRing`| Convenience wrapper around `kRingDistances`
| `polyfill` | Convenience (could be passed in, requires internal knowledge)
| `compact` | Convenience (could be passed in, requires internal knowledge)
| `h3SetToLinkedGeo` | Requires knowledge of how to initialize the internal struct
| `destroyLinkedPolygon` | Required for `h3SetToLinkedGeo`

## Prior Art

Reading materials to reference:

* [C++ `vector`](http://www.cplusplus.com/reference/vector/vector/) (via templates)
* [SDL](https://discourse.libsdl.org/t/sdl-2-0-7-prerelease/23232) (via `SDL_SetMemoryFunctions`)
* [PostgreSQL](https://www.postgresql.org/docs/10/xfunc-c.html) (via `palloc`)
* [SQLite](https://sqlite.org/malloc.html)

## Approaches

All approaches assume the user has defined the following functions:

```
void* my_malloc(size_t size);
void* my_calloc(size_t count, size_t size);
void my_free(void* pointer);

// TODO: Do we want my_realloc?
```

### Global statics

In this approach, H3 stores the allocation functions in a set of static variables.

```
h3SetAllocator(&my_alloc, &my_calloc, &my_free);

// call into H3 as before
polyfill(geoPolygon, res, out);
```

Pro:
* Allows the user to replace allocators at run time.

Con:
* Not thread safe, or an additional, complicated dependency is needed to ensure thread safety.
* Global state.

### Templates

This approach is similar to how C++ handles allocator replacement in its standard library, by accepting the allocator
as a template argument. However, H3 is written in C and must implement templates using macros.

```
POLYFILL_WITH_ALLLOCATORS(my_polyfill, my_malloc, my_calloc, my_free);

// Call the function created by the template
my_polyfill(geoPolygon, res, out);
```

Pro:
* Allows the user to have multiple allocator replacements in use at once.

Con:
* Exposes a complicated build process to the user in the form of macros.

### Allocator argument approach

In this approach, every function call includes allocators.

```
H3MemoryManager allocFunctions = {
  .malloc = &my_malloc,
  .calloc = &my_calloc,
  .free = &my_free
};

polyfill(geoPolygon, res, out, &allocFunctions);
```

Pro:
* Allowing replacement on a per-call basis allows for maximum control by the user.

Con:
* The user must always specify allocators, which is unlikely to be needed by most users.
* Alternately, additional overloads of all H3 functions that heap-allocate are needed.

### `#define` approach

In this approach, the allocators are specified at build time.

```
# In build process:
cmake -DH3_ALLOC_PREFIX=my_ ...

// in source file, functions are used as before.
```

Alternately, instead of setting a prefix, the build could accept individual options
for functions, such as `-DH3_MALLOC=my_malloc -DH3_CALLOC=my_calloc`. (Although this
could allow a user to accidentally override `malloc` but not `free`, which is generally
very bad.)

Pro:
* Minimal overhead for users and developers when allocator replacement is not needed.

Con:
* Complex allocator replacement (i.e. different allocators for different calls) is possible but requires implementation
  by the user.

## Proposal

`#define` based allocator replacement seems like the clearest and lowest overhead to implement, while still supporting
the full range of use cases. A user could optionally implement a more complicated replacement inside their custom
allocator functions.
