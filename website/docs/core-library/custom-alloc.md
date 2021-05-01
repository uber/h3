---
id: custom-alloc
title: Memory allocation
sidebar_label: Memory allocation
slug: /core-library/custom-alloc
---

H3's approach to memory management is to rely on memory allocated by the caller as much as possible. This allows memory to be managed by an external framework.

In some cases (for example, `polygonToCells`), H3 allocates heap memory. When this is needed, it uses the standard C memory allocation functions.

## Custom Memory Allocators

:::caution

On some systems, such as Windows, the undefined symbols cannot be undefined at build time. Further changes to the H3 build are needed to provide custom implementations.

:::

:::caution

There are a few algorithms like `kRing` that still use the call stack to recurse and could run out of memory that way.

:::

H3 supports replacing the memory management functions (`malloc`, `calloc`, `realloc`, `free`) used by the library at build time. This can be used to integrate H3 within a managed framework.

When using custom memory allocators, H3 prefixes the names of memory allocation functions with the string you specify. The application linking H3 must have the prefixed replacement functions defined, or you must change the H3 build to link against the prefixed replacement functions.

When building H3, specify the `H3_ALLOC_PREFIX` option to your prefix of choice, as below:

```bash
cmake -DH3_ALLOC_PREFIX=my_prefix_ .
```

Then, in your application using H3, implement the following functions, replacing `my_prefix_` with the prefix you chose above:

```c
void* my_prefix_malloc(size_t size);
void* my_prefix_calloc(size_t num, size_t size);
void* my_prefix_realloc(void* ptr, size_t size);
void my_prefix_free(void* ptr);
```

:::info

H3 does not currently use `realloc`.

:::

Link to H3 as you would have without the custom allocators. The custom allocators will be used for allocating heap memory in H3.
