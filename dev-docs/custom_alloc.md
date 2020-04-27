# Custom Memory Allocators

H3 supports replacing the memory management functions (`malloc`, `calloc`, `realloc`, `free`) used by the library at build time. This can be used to integrate H3 within a managed framework.

When using custom memory allocators, H3 prefixes the names of memory allocation functions with the string you specify. The application linking H3 must have the prefixed replacement functions defined.

When building H3, specify the `H3_ALLOC_PREFIX` option to your prefix of choice, as below:

```
cmake -DH3_ALLOC_PREFIX=my_prefix_ .
```

Then, in your application using H3, implement the following functions (replacing `my_prefix_` with the prefix you chose above):

```
void* my_prefix_malloc(size_t size);
void* my_prefix_calloc(size_t num, size_t size);
void* my_prefix_realloc(void* ptr, size_t size);
void my_prefix_free(void* ptr);
```

Link to H3 as you would have without the custom allocators. The custom allocators will be used for allocating heap memory in H3.

## Notes/Caveats

* There are a few algorithms like `kRing` that still use the call stack to recurse and could run out of memory that way.
* H3 does not currently use `realloc`.
