# Indexing functions

## geoToH3

```
H3Index geoToH3(const GeoCoord *g, int res);
```

Indexes the location at the specified resolution.

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
