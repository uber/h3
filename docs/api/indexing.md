# Indexing functions

These function are used for finding the H3 index containing coordinates, and for finding the center and boundary of H3 indexes.

## geoToH3

```
H3Index geoToH3(const GeoCoord *g, int res);
```

Indexes the location at the specified resolution, returning the index of the cell containing the location.

Returns 0 on error.

## h3ToGeo

```
void h3ToGeo(H3Index h3, GeoCoord *g);
```

Finds the centroid of the index.

## h3ToGeoBoundary

```
void h3ToGeoBoundary(H3Index h3, GeoBoundary *gp);
```

Finds the boundary of the index.
