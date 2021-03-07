# Indexing functions

These function are used for finding the H3 index containing coordinates, and for finding the center and boundary of H3 indexes.

## geoToH3

```
H3Error geoToH3(const GeoCoord *g, int res, H3Index *out);
```

Indexes the location at the specified resolution, returning 0 (E_SUCCESS) on success
or another value on error. The index of the cell containing the location is placed
in `out` on success.

## h3ToGeo

```
H3Error h3ToGeo(H3Index h3, GeoCoord *g);
```

Finds the centroid of the index.

## h3ToGeoBoundary

```
H3Error h3ToGeoBoundary(H3Index h3, GeoBoundary *gp);
```

Finds the boundary of the index.
