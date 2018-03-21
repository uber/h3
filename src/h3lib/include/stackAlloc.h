#ifndef STACKALLOC_H
#define STACKALLOC_H

#include <assert.h>
#include <string.h>

#ifdef H3_HAVE_VLA

#define STACK_ARRAY_CALLOC(type, name, numElements)        \
    assert((numElements) > 0);                             \
    type name##Buffer[(numElements)];                      \
    memset(name##Buffer, 0, (numElements) * sizeof(type)); \
    type* name = name##Buffer

#elif defined(H3_HAVE_ALLOCA)

#ifdef _MSC_VERSION

#include <malloc.h>

#define STACK_ARRAY_CALLOC(type, name, numElements)            \
    assert((numElements) > 0);                                 \
    type* name = (type*)_alloca(sizeof(type) * (numElements)); \
    memset(name, 0, sizeof(type) * (numElements))

#else

#include <alloca.h>

#define STACK_ARRAY_CALLOC(type, name, numElements)           \
    assert((numElements) > 0);                                \
    type* name = (type*)alloca(sizeof(type) * (numElements)); \
    memset(name, 0, sizeof(type) * (numElements))

#endif

#else

#error \
    "This platform does not support stack array allocation, please submit an issue on https://github.com/uber/h3 to report this error"

#endif

#endif
