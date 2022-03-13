---
id: quickstart
title: Quick Start
sidebar_label: Quick Start
slug: /quickstart
---

This page shows you how to get started with the functions in H3 that convert points to cell identifiers, and from cell identifiers back to geometry. These are the core indexing functions used in many applications of H3.

You can run the code on this page directly in your browser. The page uses the JavaScript bindings for H3 to run the code, or follow along with the same API in [your preferred programming language](/docs/community/bindings).

## Point / cell

First, we'll take the coordinates of the Ferry Building in San Francisco and find the containing H3 cell:

```js live
function example() {
  const lat = 37.7955;
  const lng = -122.3937;
  const res = 10;
  return h3.latLngToCell(lat, lng, res);
}
```

The result is the identifier of the hexagonal cell in H3 containing this point. We can retrieve the center of this cell:

```js live
function example() {
  const h = '8a283082a677fff';
  return h3.cellToLatLng(h);
}
```

Note that the result of this example is not our original coordinates. That's because the identifier represents the hexagonal cell, not the coordinates. All points indexed in H3 within the bounds of this cell will have the same identifier. We can find the bounds of this cell:

```js live
function example() {
  const h = '8a283082a677fff';
  return h3.cellToBoundary(h);
}
```
