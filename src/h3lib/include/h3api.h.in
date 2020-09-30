/*
 * Copyright 2016-2020 Uber Technologies, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/** @file h3api.h
 * @brief   Primary H3 core library entry points.
 *
 * This file defines the public API of the H3 library. Incompatible changes to
 * these functions require the library's major version be increased.
 */

#ifndef H3API_H
#define H3API_H

/*
 * Preprocessor code to support renaming (prefixing) the public API.
 * All public functions should be wrapped in H3_EXPORT so they can be
 * renamed.
 */
#ifdef H3_PREFIX
#define XTJOIN(a, b) a##b
#define TJOIN(a, b) XTJOIN(a, b)

/* export joins the user provided prefix with our exported function name */
#define H3_EXPORT(name) TJOIN(H3_PREFIX, name)
#else
#define H3_EXPORT(name) name
#endif

/* For uint64_t */
#include <stdint.h>
/* For size_t */
#include <stdlib.h>

/*
 * H3 is compiled as C, not C++ code. `extern "C"` is needed for C++ code
 * to be able to use the library.
 */
#ifdef __cplusplus
extern "C" {
#endif

/** @brief the H3Index fits within a 64-bit unsigned integer */
typedef uint64_t H3Index;

/* library version numbers generated from VERSION file */
// clang-format off
#define H3_VERSION_MAJOR @H3_VERSION_MAJOR@
#define H3_VERSION_MINOR @H3_VERSION_MINOR@
#define H3_VERSION_PATCH @H3_VERSION_PATCH@
// clang-format on

/** Maximum number of cell boundary vertices; worst case is pentagon:
 *  5 original verts + 5 edge crossings
 */
#define MAX_CELL_BNDRY_VERTS 10

/** @struct GeoCoord
    @brief latitude/longitude in radians
*/
typedef struct {
    double lat;  ///< latitude in radians
    double lon;  ///< longitude in radians
} GeoCoord;

/** @struct GeoBoundary
    @brief cell boundary in latitude/longitude
*/
typedef struct {
    int numVerts;                          ///< number of vertices
    GeoCoord verts[MAX_CELL_BNDRY_VERTS];  ///< vertices in ccw order
} GeoBoundary;

/** @struct Geofence
 *  @brief similar to GeoBoundary, but requires more alloc work
 */
typedef struct {
    int numVerts;
    GeoCoord *verts;
} Geofence;

/** @struct GeoPolygon
 *  @brief Simplified core of GeoJSON Polygon coordinates definition
 */
typedef struct {
    Geofence geofence;  ///< exterior boundary of the polygon
    int numHoles;       ///< number of elements in the array pointed to by holes
    Geofence *holes;    ///< interior boundaries (holes) in the polygon
} GeoPolygon;

/** @struct GeoMultiPolygon
 *  @brief Simplified core of GeoJSON MultiPolygon coordinates definition
 */
typedef struct {
    int numPolygons;
    GeoPolygon *polygons;
} GeoMultiPolygon;

/** @struct LinkedGeoCoord
 *  @brief A coordinate node in a linked geo structure, part of a linked list
 */
typedef struct LinkedGeoCoord LinkedGeoCoord;
struct LinkedGeoCoord {
    GeoCoord vertex;
    LinkedGeoCoord *next;
};

/** @struct LinkedGeoLoop
 *  @brief A loop node in a linked geo structure, part of a linked list
 */
typedef struct LinkedGeoLoop LinkedGeoLoop;
struct LinkedGeoLoop {
    LinkedGeoCoord *first;
    LinkedGeoCoord *last;
    LinkedGeoLoop *next;
};

/** @struct LinkedGeoPolygon
 *  @brief A polygon node in a linked geo structure, part of a linked list.
 */
typedef struct LinkedGeoPolygon LinkedGeoPolygon;
struct LinkedGeoPolygon {
    LinkedGeoLoop *first;
    LinkedGeoLoop *last;
    LinkedGeoPolygon *next;
};

/** @struct CoordIJ
 * @brief IJ hexagon coordinates
 *
 * Each axis is spaced 120 degrees apart.
 */
typedef struct {
    int i;  ///< i component
    int j;  ///< j component
} CoordIJ;

/** @defgroup geoToH3 geoToH3
 * Functions for geoToH3
 * @{
 */
/** @brief find the H3 index of the resolution res cell containing the lat/lng
 */
H3Index H3_EXPORT(geoToH3)(const GeoCoord *g, int res);
/** @} */

/** @defgroup h3ToGeo h3ToGeo
 * Functions for h3ToGeo
 * @{
 */
/** @brief find the lat/lon center point g of the cell h3 */
void H3_EXPORT(h3ToGeo)(H3Index h3, GeoCoord *g);
/** @} */

/** @defgroup h3ToGeoBoundary h3ToGeoBoundary
 * Functions for h3ToGeoBoundary
 * @{
 */
/** @brief give the cell boundary in lat/lon coordinates for the cell h3 */
void H3_EXPORT(h3ToGeoBoundary)(H3Index h3, GeoBoundary *gp);
/** @} */

/** @defgroup kRing kRing
 * Functions for kRing
 * @{
 */
/** @brief maximum number of hexagons in k-ring */
int H3_EXPORT(maxKringSize)(int k);

/** @brief hexagons neighbors in all directions, assuming no pentagons */
int H3_EXPORT(hexRange)(H3Index origin, int k, H3Index *out);
/** @} */

/** @brief hexagons neighbors in all directions, assuming no pentagons,
 * reporting distance from origin */
int H3_EXPORT(hexRangeDistances)(H3Index origin, int k, H3Index *out,
                                 int *distances);

/** @brief collection of hex rings sorted by ring for all given hexagons */
int H3_EXPORT(hexRanges)(H3Index *h3Set, int length, int k, H3Index *out);

/** @brief hexagon neighbors in all directions */
void H3_EXPORT(kRing)(H3Index origin, int k, H3Index *out);
/** @} */

/** @defgroup kRingDistances kRingDistances
 * Functions for kRingDistances
 * @{
 */
/** @brief hexagon neighbors in all directions, reporting distance from origin
 */
void H3_EXPORT(kRingDistances)(H3Index origin, int k, H3Index *out,
                               int *distances);
/** @} */

/** @defgroup hexRing hexRing
 * Functions for hexRing
 * @{
 */
/** @brief hollow hexagon ring at some origin */
int H3_EXPORT(hexRing)(H3Index origin, int k, H3Index *out);
/** @} */

/** @defgroup polyfill polyfill
 * Functions for polyfill
 * @{
 */
/** @brief maximum number of hexagons in the geofence */
int H3_EXPORT(maxPolyfillSize)(const GeoPolygon *geoPolygon, int res);

/** @brief hexagons within the given geofence */
void H3_EXPORT(polyfill)(const GeoPolygon *geoPolygon, int res, H3Index *out);
/** @} */

/** @defgroup h3SetToMultiPolygon h3SetToMultiPolygon
 * Functions for h3SetToMultiPolygon (currently a binding-only concept)
 * @{
 */
/** @brief Create a LinkedGeoPolygon from a set of contiguous hexagons */
void H3_EXPORT(h3SetToLinkedGeo)(const H3Index *h3Set, const int numHexes,
                                 LinkedGeoPolygon *out);

/** @brief Free all memory created for a LinkedGeoPolygon */
void H3_EXPORT(destroyLinkedPolygon)(LinkedGeoPolygon *polygon);
/** @} */

/** @defgroup degsToRads degsToRads
 * Functions for degsToRads
 * @{
 */
/** @brief converts degrees to radians */
double H3_EXPORT(degsToRads)(double degrees);
/** @} */

/** @defgroup radsToDegs radsToDegs
 * Functions for radsToDegs
 * @{
 */
/** @brief converts radians to degrees */
double H3_EXPORT(radsToDegs)(double radians);
/** @} */

/** @defgroup pointDist pointDist
 * Functions for pointDist
 * @{
 */
/** @brief "great circle distance" between pairs of GeoCoord points in radians*/
double H3_EXPORT(pointDistRads)(const GeoCoord *a, const GeoCoord *b);

/** @brief "great circle distance" between pairs of GeoCoord points in
 * kilometers*/
double H3_EXPORT(pointDistKm)(const GeoCoord *a, const GeoCoord *b);

/** @brief "great circle distance" between pairs of GeoCoord points in meters*/
double H3_EXPORT(pointDistM)(const GeoCoord *a, const GeoCoord *b);
/** @} */

/** @defgroup hexArea hexArea
 * Functions for hexArea
 * @{
 */
/** @brief average hexagon area in square kilometers (excludes pentagons) */
double H3_EXPORT(hexAreaKm2)(int res);

/** @brief average hexagon area in square meters (excludes pentagons) */
double H3_EXPORT(hexAreaM2)(int res);
/** @} */

/** @defgroup cellArea cellArea
 * Functions for cellArea
 * @{
 */
/** @brief exact area for a specific cell (hexagon or pentagon) in radians^2 */
double H3_EXPORT(cellAreaRads2)(H3Index h);

/** @brief exact area for a specific cell (hexagon or pentagon) in kilometers^2
 */
double H3_EXPORT(cellAreaKm2)(H3Index h);

/** @brief exact area for a specific cell (hexagon or pentagon) in meters^2 */
double H3_EXPORT(cellAreaM2)(H3Index h);
/** @} */

/** @defgroup edgeLength edgeLength
 * Functions for edgeLength
 * @{
 */
/** @brief average hexagon edge length in kilometers (excludes pentagons) */
double H3_EXPORT(edgeLengthKm)(int res);

/** @brief average hexagon edge length in meters (excludes pentagons) */
double H3_EXPORT(edgeLengthM)(int res);
/** @} */

/** @defgroup exactEdgeLength exactEdgeLength
 * Functions for exactEdgeLength
 * @{
 */
/** @brief exact length for a specific unidirectional edge in radians*/
double H3_EXPORT(exactEdgeLengthRads)(H3Index edge);

/** @brief exact length for a specific unidirectional edge in kilometers*/
double H3_EXPORT(exactEdgeLengthKm)(H3Index edge);

/** @brief exact length for a specific unidirectional edge in meters*/
double H3_EXPORT(exactEdgeLengthM)(H3Index edge);
/** @} */

/** @defgroup numHexagons numHexagons
 * Functions for numHexagons
 * @{
 */
/** @brief number of cells (hexagons and pentagons) for a given resolution */
int64_t H3_EXPORT(numHexagons)(int res);
/** @} */

/** @defgroup getRes0Indexes getRes0Indexes
 * Functions for getRes0Indexes
 * @{
 */
/** @brief returns the number of resolution 0 cells (hexagons and pentagons) */
int H3_EXPORT(res0IndexCount)();

/** @brief provides all base cells in H3Index format*/
void H3_EXPORT(getRes0Indexes)(H3Index *out);
/** @} */

/** @defgroup getPentagonIndexes getPentagonIndexes
 * Functions for getPentagonIndexes
 * @{
 */
/** @brief returns the number of pentagons per resolution */
int H3_EXPORT(pentagonIndexCount)();

/** @brief generates all pentagons at the specified resolution */
void H3_EXPORT(getPentagonIndexes)(int res, H3Index *out);
/** @} */

/** @defgroup h3GetResolution h3GetResolution
 * Functions for h3GetResolution
 * @{
 */
/** @brief returns the resolution of the provided H3 index
 * Works on both cells and unidirectional edges. */
int H3_EXPORT(h3GetResolution)(H3Index h);
/** @} */

/** @defgroup h3GetBaseCell h3GetBaseCell
 * Functions for h3GetBaseCell
 * @{
 */
/** @brief returns the base cell "number" (0 to 121) of the provided H3 cell
 *
 * Note: Technically works on H3 edges, but will return base cell of the
 * origin cell. */
int H3_EXPORT(h3GetBaseCell)(H3Index h);
/** @} */

/** @defgroup stringToH3 stringToH3
 * Functions for stringToH3
 * @{
 */
/** @brief converts the canonical string format to H3Index format */
H3Index H3_EXPORT(stringToH3)(const char *str);
/** @} */

/** @defgroup h3ToString h3ToString
 * Functions for h3ToString
 * @{
 */
/** @brief converts an H3Index to a canonical string */
void H3_EXPORT(h3ToString)(H3Index h, char *str, size_t sz);
/** @} */

/** @defgroup h3IsValid h3IsValid
 * Functions for h3IsValid
 * @{
 */
/** @brief confirms if an H3Index is a valid cell (hexagon or pentagon)
 * In particular, returns 0 (False) for H3 unidirectional edges or invalid data
 */
int H3_EXPORT(h3IsValid)(H3Index h);
/** @} */

/** @defgroup h3ToParent h3ToParent
 * Functions for h3ToParent
 * @{
 */
/** @brief returns the parent (or grandparent, etc) hexagon of the given hexagon
 */
H3Index H3_EXPORT(h3ToParent)(H3Index h, int parentRes);
/** @} */

/** @defgroup h3ToChildren h3ToChildren
 * Functions for h3ToChildren
 * @{
 */
/** @brief determines the maximum number of children (or grandchildren, etc)
 * that could be returned for the given hexagon */
int H3_EXPORT(maxH3ToChildrenSize)(H3Index h, int childRes);

/** @brief provides the children (or grandchildren, etc) of the given hexagon */
void H3_EXPORT(h3ToChildren)(H3Index h, int childRes, H3Index *children);
/** @} */

/** @defgroup h3ToCenterChild h3ToCenterChild
 * Functions for h3ToCenterChild
 * @{
 */
/** @brief returns the center child of the given hexagon at the specified
 * resolution */
H3Index H3_EXPORT(h3ToCenterChild)(H3Index h, int childRes);
/** @} */

/** @defgroup compact compact
 * Functions for compact
 * @{
 */
/** @brief compacts the given set of hexagons as best as possible */
int H3_EXPORT(compact)(const H3Index *h3Set, H3Index *compactedSet,
                       const int numHexes);
/** @} */

/** @defgroup uncompact uncompact
 * Functions for uncompact
 * @{
 */
/** @brief determines the maximum number of hexagons that could be uncompacted
 * from the compacted set */
int H3_EXPORT(maxUncompactSize)(const H3Index *compactedSet, const int numHexes,
                                const int res);

/** @brief uncompacts the compacted hexagon set */
int H3_EXPORT(uncompact)(const H3Index *compactedSet, const int numHexes,
                         H3Index *h3Set, const int maxHexes, const int res);
/** @} */

/** @defgroup h3IsResClassIII h3IsResClassIII
 * Functions for h3IsResClassIII
 * @{
 */
/** @brief determines if a hexagon is Class III (or Class II) */
int H3_EXPORT(h3IsResClassIII)(H3Index h);
/** @} */

/** @defgroup h3IsPentagon h3IsPentagon
 * Functions for h3IsPentagon
 * @{
 */
/** @brief determines if an H3 cell is a pentagon */
int H3_EXPORT(h3IsPentagon)(H3Index h);
/** @} */

/** @defgroup h3GetFaces h3GetFaces
 * Functions for h3GetFaces
 * @{
 */
/** @brief Max number of icosahedron faces intersected by an index */
int H3_EXPORT(maxFaceCount)(H3Index h3);

/** @brief Find all icosahedron faces intersected by a given H3 index */
void H3_EXPORT(h3GetFaces)(H3Index h3, int *out);
/** @} */

/** @defgroup h3IndexesAreNeighbors h3IndexesAreNeighbors
 * Functions for h3IndexesAreNeighbors
 * @{
 */
/** @brief returns whether or not the provided hexagons border */
int H3_EXPORT(h3IndexesAreNeighbors)(H3Index origin, H3Index destination);
/** @} */

/** @defgroup getH3UnidirectionalEdge getH3UnidirectionalEdge
 * Functions for getH3UnidirectionalEdge
 * @{
 */
/** @brief returns the unidirectional edge H3Index for the specified origin and
 * destination */
H3Index H3_EXPORT(getH3UnidirectionalEdge)(H3Index origin, H3Index destination);
/** @} */

/** @defgroup h3UnidirectionalEdgeIsValid h3UnidirectionalEdgeIsValid
 * Functions for h3UnidirectionalEdgeIsValid
 * @{
 */
/** @brief returns whether the H3Index is a valid unidirectional edge */
int H3_EXPORT(h3UnidirectionalEdgeIsValid)(H3Index edge);
/** @} */

/** @defgroup getOriginH3IndexFromUnidirectionalEdge \
 * getOriginH3IndexFromUnidirectionalEdge
 * Functions for getOriginH3IndexFromUnidirectionalEdge
 * @{
 */
/** @brief Returns the origin hexagon H3Index from the unidirectional edge
 * H3Index */
H3Index H3_EXPORT(getOriginH3IndexFromUnidirectionalEdge)(H3Index edge);
/** @} */

/** @defgroup getDestinationH3IndexFromUnidirectionalEdge \
 * getDestinationH3IndexFromUnidirectionalEdge
 * Functions for getDestinationH3IndexFromUnidirectionalEdge
 * @{
 */
/** @brief Returns the destination hexagon H3Index from the unidirectional edge
 * H3Index */
H3Index H3_EXPORT(getDestinationH3IndexFromUnidirectionalEdge)(H3Index edge);
/** @} */

/** @defgroup getH3IndexesFromUnidirectionalEdge \
 * getH3IndexesFromUnidirectionalEdge
 * Functions for getH3IndexesFromUnidirectionalEdge
 * @{
 */
/** @brief Returns the origin and destination hexagons from the unidirectional
 * edge H3Index */
void H3_EXPORT(getH3IndexesFromUnidirectionalEdge)(H3Index edge,
                                                   H3Index *originDestination);
/** @} */

/** @defgroup getH3UnidirectionalEdgesFromHexagon \
 * getH3UnidirectionalEdgesFromHexagon
 * Functions for getH3UnidirectionalEdgesFromHexagon
 * @{
 */
/** @brief Returns the 6 (or 5 for pentagons) edges associated with the H3Index
 */
void H3_EXPORT(getH3UnidirectionalEdgesFromHexagon)(H3Index origin,
                                                    H3Index *edges);
/** @} */

/** @defgroup getH3UnidirectionalEdgeBoundary getH3UnidirectionalEdgeBoundary
 * Functions for getH3UnidirectionalEdgeBoundary
 * @{
 */
/** @brief Returns the GeoBoundary containing the coordinates of the edge */
void H3_EXPORT(getH3UnidirectionalEdgeBoundary)(H3Index edge, GeoBoundary *gb);
/** @} */

/** @defgroup h3Distance h3Distance
 * Functions for h3Distance
 * @{
 */
/** @brief Returns grid distance between two indexes */
int H3_EXPORT(h3Distance)(H3Index origin, H3Index h3);
/** @} */

/** @defgroup h3Line h3Line
 * Functions for h3Line
 * @{
 */
/** @brief Number of indexes in a line connecting two indexes */
int H3_EXPORT(h3LineSize)(H3Index start, H3Index end);

/** @brief Line of h3 indexes connecting two indexes */
int H3_EXPORT(h3Line)(H3Index start, H3Index end, H3Index *out);
/** @} */

/** @defgroup experimentalH3ToLocalIj experimentalH3ToLocalIj
 * Functions for experimentalH3ToLocalIj
 * @{
 */
/** @brief Returns two dimensional coordinates for the given index */
int H3_EXPORT(experimentalH3ToLocalIj)(H3Index origin, H3Index h3,
                                       CoordIJ *out);
/** @} */

/** @defgroup experimentalLocalIjToH3 experimentalLocalIjToH3
 * Functions for experimentalLocalIjToH3
 * @{
 */
/** @brief Returns index for the given two dimensional coordinates */
int H3_EXPORT(experimentalLocalIjToH3)(H3Index origin, const CoordIJ *ij,
                                       H3Index *out);
/** @} */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
