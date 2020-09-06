# RFC: Consistent error handling

* **Authors**: -
* **Date**: -
* **Status**: Draft

## Abstract

Error handling in the H3 library is inconsistent across functions, many of which have no explicit mechanism to indicate that an error occured. This RFC proposes updating all functions that allocate blocks of memory or that could receive invalid input to return an integer code indicating success or error.

## Motivation

*Why is this important?*

## Approaches

*What are the various options to address this issue?*

## Proposal

*What is the recommended approach?*