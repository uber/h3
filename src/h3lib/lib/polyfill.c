/*
 * Copyright 2023 Uber Technologies, Inc.
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
/** @file polyfill.c
 * @brief   Functions relating to the cell-to-polygon algorithm
 */

#include "polyfill.h"

#include <math.h>
#include <string.h>

#include "alloc.h"
#include "baseCells.h"
#include "coordijk.h"
#include "h3Assert.h"
#include "h3Index.h"
#include "polygon.h"

// Factor by which to scale the cell bounding box to include all cells.
// This was determined empirically by finding the smallest factor that
// passed exhaustive tests.
#define CELL_SCALE_FACTOR 1.1

// Factor by which to scale the cell bounding box to include all children.
// This was determined empirically by finding the smallest factor that
// passed exhaustive tests.
#define CHILD_SCALE_FACTOR 1.4

/**
 * Max cell edge length, in radians, for each resolution. This was computed
 * by taking the max exact edge length for cells at the center of each base
 * cell at that resolution.
 */
static double MAX_EDGE_LENGTH_RADS[MAX_H3_RES + 1] = {
    0.21577206265130, 0.08308767068495, 0.03148970436439, 0.01190662871439,
    0.00450053330908, 0.00170105523619, 0.00064293917678, 0.00024300820659,
    0.00009184847087, 0.00003471545901, 0.00001312121017, 0.00000495935129,
    0.00000187445860, 0.00000070847876, 0.00000026777980, 0.00000010121125};

/** All cells that contain the north pole, by res */
static H3Index NORTH_POLE_CELLS[MAX_H3_RES + 1] = {
    0x8001fffffffffff, 0x81033ffffffffff, 0x820327fffffffff, 0x830326fffffffff,
    0x8403263ffffffff, 0x85032623fffffff, 0x860326237ffffff, 0x870326233ffffff,
    0x880326233bfffff, 0x890326233abffff, 0x8a0326233ab7fff, 0x8b0326233ab0fff,
    0x8c0326233ab03ff, 0x8d0326233ab03bf, 0x8e0326233ab039f, 0x8f0326233ab0399};

/** All cells that contain the south pole, by res */
static H3Index SOUTH_POLE_CELLS[MAX_H3_RES + 1] = {
    0x80f3fffffffffff, 0x81f2bffffffffff, 0x82f297fffffffff, 0x83f293fffffffff,
    0x84f2939ffffffff, 0x85f29383fffffff, 0x86f29380fffffff, 0x87f29380effffff,
    0x88f29380e1fffff, 0x89f29380e0fffff, 0x8af29380e0d7fff, 0x8bf29380e0d0fff,
    0x8cf29380e0d0dff, 0x8df29380e0d0cff, 0x8ef29380e0d0cc7, 0x8ff29380e0d0cc4};

/** Pre-calculated bounding boxes for all res 0 cells */
static BBox RES0_BBOXES[NUM_BASE_CELLS] = {
    {1.52480158339146, 1.20305471830087, -0.60664883654036, 0.00568297271999},
    {1.52480158339146, 1.17872424267511, -0.60664883654036, 2.54046980298264},
    {1.52480158339146, 1.09069387298096, -2.85286053297673, 1.64310689027893},
    {1.41845302535151, 1.01285145697208, 0.00568297271999, -1.16770379632602},
    {1.27950477868453, 0.97226652536306, 0.55556064983494, -0.18229924845326},
    {1.32929586572429, 0.91898920750071, 2.05622344943192, 1.08813154278274},
    {1.32899086063916, 0.94271815376360, -2.29875289606378, 3.01700008041993},
    {1.26020983864103, 0.84291228415618, -0.89971867664861, -1.75967359310997},
    {1.21114673854945, 0.86170600921069, 1.19129757609455, 0.43777608996454},
    {1.21075831414294, 0.83795331049498, -1.72022875779891, -2.43793861727138},
    {1.15546530929588, 0.78982455384253, 2.53659412229266, 1.85709133451243},
    {1.15528445067052, 0.76641428724335, -3.06738507202411, 2.53646110244042},
    {1.10121643537669, 0.71330093663066, 0.09640581900154, -0.52154514518248},
    {1.07042472765165, 0.67603948819406, -0.47984202840088, -1.10306159603090},
    {1.03270228748960, 0.72356358827215, -2.24990138725146, -2.74510220919157},
    {1.01929924623886, 0.65491232835426, 0.63035574240731, 0.03537030096470},
    {1.01786037568858, 0.58827636737638, 1.53192721817065, 0.93672682511233},
    {0.98081434136020, 0.61076063532947, -2.67100636598529, 3.06516463008733},
    {0.98106023192774, 0.58679836571570, 2.02829766214461, 1.51334374970280},
    {0.96374551790056, 0.55186491737474, -1.42976721313659, -1.96852202530104},
    {0.87536136210723, 0.50008952762292, -1.92435613571430, -2.41641343219793},
    {0.88611243445554, 0.52742963716774, -0.95781946324194, -1.47628966305930},
    {0.86881343251986, 0.50770567021439, 1.03236795495839, 0.50347284027426},
    {0.89235638181782, 0.48781264892508, 2.76430302119150, 2.29989716697031},
    {0.82570569254601, 0.52173101741059, 2.30921681461428, 1.93198541828980},
    {0.80599330438546, 0.40150819579319, -3.06417559403240, 2.70079300784409},
    {0.81612079704781, 0.38396800633226, -0.21614378891839, -0.70420149722178},
    {0.75822779851431, 0.39943555383751, -2.34059978084699, -2.82127373822444},
    {0.78861390967531, 0.38742018303868, 0.23115687731652, -0.22599491086066},
    {0.71515840341957, 0.33012478438475, -0.64847976163163, -1.08249728121219},
    {0.70359051048414, 0.29148673180722, 1.71441081857246, 1.28443348381696},
    {0.69190629544818, 0.28808313184381, 0.64863909244647, 0.16372369282557},
    {0.64863235654749, 0.26290420067147, 2.10318098268379, 1.69556122548344},
    {0.65722892279906, 0.28222653310929, 1.30918693285466, 0.87594416271685},
    {0.64750997738584, 0.24149865709850, -1.30272192474556, -1.68708570163242},
    {0.62380174028378, 0.25522080363509, -2.72428423026826, 3.10401473237630},
    {0.64228460410023, 0.21206753429148, -1.67639240992071, -2.11772366767341},
    {0.59919175361146, 0.21620460836570, 2.48592868387690, 2.07350353893591},
    {0.55637406851384, 0.25276557437230, -0.99885388505694, -1.32642489358939},
    {0.55648013300665, 0.15187401321019, 2.87032088421324, 2.44642320475367},
    {0.54603687970450, 0.15589091511369, -2.06789866067060, -2.49091419631961},
    {0.51206347752697, 0.15522020377124, 0.95446767315996, 0.54443262110414},
    {0.49767951537101, 0.10944898890579, -0.04335162263358, -0.42900268178569},
    {0.46538045483671, 0.06029968637720, -0.41240613713421, -0.80603623808166},
    {0.44686891066946, 0.06926857458503, 0.32053284794952, -0.07005748900849},
    {0.43208958202064, 0.07796440938140, -3.06232453079660, 2.80602499990282},
    {0.43103892586713, 0.02927431919853, -2.41589238618422, -2.85735809951951},
    {0.38073727558986, -0.00297016159959, -0.77039553861218, -1.14788248745028},
    {0.39113816687141, -0.01518764903038, 1.49130246958290, 1.14714731736311},
    {0.33421063142418, 0.02526613430348, 1.15141032578749, 0.85000706261644},
    {0.38915669778582, -0.04371359825454, 1.88046353933242, 1.48230231380717},
    {0.33787520825987, -0.04835090128296, -1.12274014380603, -1.49454408844749},
    {0.33601418932337, -0.06675068178541, 2.23792354204464, 1.85723423013211},
    {0.31838318078049, -0.05821955623722, 0.66058854060373, 0.25452572938783},
    {0.33630761471457, -0.07589541031521, -1.47957331741818, -1.85981735718264},
    {0.28924817322870, -0.09150638064667, -1.83561930288569, -2.21855897384292},
    {0.26678632252475, -0.10058088990867, -2.76808651991421, 3.12792953247061},
    {0.29285254112587, -0.13483165093783, 2.61406468380434, 2.20466422911705},
    {0.20150342788824, -0.10279852729762, 0.06881896344365, -0.23925229432978},
    {0.21283813275258, -0.18626835417891, 2.93800440256577, 2.57470747655623},
    {0.19587614179884, -0.17237030304155, -2.16941795427335, -2.55405165906601},
    {0.17237030304155, -0.19587614179884, 0.97217469931645, 0.58754099452378},
    {0.18626835417891, -0.21283813275258, -0.20358825102402, -0.56688517703356},
    {0.10279852729762, -0.20150342788824, -3.07277369014614, 2.90234035926002},
    {0.13483165093783, -0.29285254112587, -0.52752796978545, -0.93692842447275},
    {0.10058088990867, -0.26678632252475, 0.37350613367558, -0.01366312111919},
    {0.09150638064667, -0.28924817322870, 1.30597335070410, 0.92303367974687},
    {0.07589541031521, -0.33630761471457, 1.66201933617161, 1.28177529640715},
    {0.05821955623722, -0.31838318078049, -2.48100411298606, -2.88706692420196},
    {0.06675068178541, -0.33601418932337, -0.90366911154516, -1.28435842345769},
    {0.04835090128296, -0.33787520825987, 2.01885250978376, 1.64704856514230},
    {0.04371359825454, -0.38915669778582, -1.26112911425737, -1.65929033978262},
    {-0.02526613430348, -0.33421063142418, -1.99018232780231,
     -2.29158559097336},
    {0.01518764903038, -0.39113816687140, -1.65029018400690, -1.99444533622668},
    {0.00297016159959, -0.38073727558986, 2.37119711497761, 1.99371016613951},
    {-0.02927431919853, -0.43103892586713, 0.72570026740558, 0.28423455407029},
    {-0.07796440938140, -0.43208958202064, 0.07926812279319, -0.33556765368697},
    {-0.06926857458503, -0.44686891066946, -2.82105980564027, 3.07153516458131},
    {-0.06029968637720, -0.46538045483671, 2.72918651645558, 2.33555641550814},
    {-0.10944898890579, -0.49767951537101, 3.09824103095621, 2.71258997180410},
    {-0.15522020377124, -0.51206347752697, -2.18712498042983,
     -2.59716003248565},
    {-0.15589091511369, -0.54603687970450, 1.07369399291919, 0.65067845727018},
    {-0.15187401321019, -0.55648013300665, -0.27127176937655,
     -0.69516944883612},
    {-0.25276557437230, -0.55637406851385, 2.14273876853285, 1.81516776000041},
    {-0.21620460836570, -0.59919175361146, -0.65566396971290,
     -1.06808911465388},
    {-0.21206753429148, -0.64228460410023, 1.46520024366909, 1.02386898591638},
    {-0.25522080363509, -0.62380174028378, 0.41730842332153, -0.03757792121350},
    {-0.24149865709850, -0.64750997738584, 1.83887072884423, 1.45450695195737},
    {-0.28222653310929, -0.65722892279906, -1.83240572073513,
     -2.26564849087294},
    {-0.26290420067147, -0.64863235654749, -1.03841167090601,
     -1.44603142810635},
    {-0.28808313184381, -0.69190629544818, -2.49295356114332,
     -2.97786896076422},
    {-0.29148673180722, -0.70359051048414, -1.42718183501734,
     -1.85715916977284},
    {-0.33012478438475, -0.71515840341957, 2.49311289195816, 2.05909537237761},
    {-0.38742018303868, -0.78861390967531, -2.91043577627328, 2.91559774272914},
    {-0.39943555383751, -0.75822779851431, 0.80099287274280, 0.32031891536535},
    {-0.38396800633226, -0.81612079704781, 2.92544886467140, 2.43739115636801},
    {-0.40150819579319, -0.80599330438546, 0.07741705955739, -0.44079964574570},
    {-0.52173101741059, -0.82570569254601, -0.83237583897551,
     -1.20960723529999},
    {-0.48781264892508, -0.89235638181782, -0.37728963239830,
     -0.84169548661948},
    {-0.50770567021439, -0.86881343251986, -2.10922469863141,
     -2.63811981331554},
    {-0.52742963716774, -0.88611243445554, 2.18377319034785, 1.66530299053050},
    {-0.50008952762292, -0.87536136210723, 1.21723651787549, 0.72517922139186},
    {-0.55186491737474, -0.96374551790056, 1.71182544045320, 1.17307062828876},
    {-0.58679836571570, -0.98106023192774, -1.11329499144518,
     -1.62824890388699},
    {-0.61076063532947, -0.98081434136020, 0.47058628760450, -0.07642802350246},
    {-0.58827636737638, -1.01786037568858, -1.60966543541914,
     -2.20486582847747},
    {-0.65491232835426, -1.01929924623886, -2.51123691118248,
     -3.10622235262510},
    {-0.72356358827215, -1.03270228748960, 0.89169126633833, 0.39649044439822},
    {-0.67603948819406, -1.07042472765165, 2.66175062518892, 2.03853105755889},
    {-0.71330093663066, -1.10121643537669, -3.04518683458825, 2.62004750840731},
    {-0.76641428724335, -1.15528445067052, 0.07420758156568, -0.60513155114938},
    {-0.78982455384253, -1.15546530929588, -0.60499853129713,
     -1.28450131907736},
    {-0.83795331049498, -1.21075831414294, 1.42136389579088, 0.70365403631841},
    {-0.86170600921069, -1.21114673854945, -1.95029507749525,
     -2.70381656362525},
    {-0.84291228415618, -1.26020983864103, 2.24187397694118, 1.38191906047983},
    {-0.94271815376360, -1.32899086063916, 0.84283975752601, -0.12459257316986},
    {-0.91898920750071, -1.32929586572429, -1.08536920415787,
     -2.05346111080706},
    {-0.97226652536306, -1.27950477868453, -2.58603200375485, 2.95929340513654},
    {-1.01285145697208, -1.41845302535151, -3.13590968086981, 1.97388885726377},
    {-1.09069387298096, -1.52480158339146, 0.28873212061306, -1.49848576331087},
    {-1.17872424267511, -1.52480158339146, 2.53494381704943, -0.60112285060716},
    {-1.20305471830087, -1.52480158339146, -0.60112285060716,
     2.53494381704943}};

static BBox VALID_RANGE_BBOX = {M_PI_2, -M_PI_2, M_PI, -M_PI};

/**
 * For a given cell, return its bounding box. If coverChildren is true, the bbox
 * will be guaranteed to contain its children at any finer resolution. Note that
 * no guarantee is provided as to the level of accuracy, and the bounding box
 * may have a significant margin of error.
 * @param cell Cell to calculate bbox for
 * @param out  BBox to hold output
 * @param coverChildren Whether the bounding box should cover all children
 */
H3Error cellToBBox(H3Index cell, BBox *out, bool coverChildren) {
    // Adjust the BBox to handle poles, if needed
    int res = H3_GET_RESOLUTION(cell);

    if (res == 0) {
        int baseCell = H3_GET_BASE_CELL(cell);
        if (NEVER(baseCell < 0) || baseCell >= NUM_BASE_CELLS) {
            return E_CELL_INVALID;
        }
        *out = RES0_BBOXES[baseCell];
    } else {
        LatLng center;
        H3Error centerErr = H3_EXPORT(cellToLatLng)(cell, &center);
        if (centerErr != E_SUCCESS) {
            return centerErr;
        }
        double lngRatio = 1 / cos(center.lat);
        out->north = center.lat + MAX_EDGE_LENGTH_RADS[res];
        out->south = center.lat - MAX_EDGE_LENGTH_RADS[res];
        out->east = center.lng + MAX_EDGE_LENGTH_RADS[res] * lngRatio;
        out->west = center.lng - MAX_EDGE_LENGTH_RADS[res] * lngRatio;
    }

    // Buffer the bounding box to cover children. Call this even if no buffering
    // is required in order to normalize the bbox to lat/lng bounds
    scaleBBox(out, coverChildren ? CHILD_SCALE_FACTOR : CELL_SCALE_FACTOR);

    // Cell that contains the north pole
    if (cell == NORTH_POLE_CELLS[res]) {
        out->north = M_PI_2;
    }

    // Cell that contains the south pole
    if (cell == SOUTH_POLE_CELLS[res]) {
        out->south = -M_PI_2;
    }

    // If we contain a pole, expand the longitude to include the full domain,
    // effectively making the bbox a circle around the pole.
    if (out->north == M_PI_2 || out->south == -M_PI_2) {
        out->east = M_PI;
        out->west = -M_PI;
    }

    return E_SUCCESS;
}

/**
 * Get a base cell by number, or H3_NULL if out of bounds
 */
H3Index baseCellNumToCell(int baseCellNum) {
    if (baseCellNum < 0 || baseCellNum >= NUM_BASE_CELLS) {
        return H3_NULL;
    }
    H3Index baseCell;
    setH3Index(&baseCell, 0, baseCellNum, 0);
    return baseCell;
}

static void iterErrorPolygonCompact(IterCellsPolygonCompact *iter,
                                    H3Error error) {
    iterDestroyPolygonCompact(iter);
    iter->error = error;
}

/**
 * Given a cell, find the next cell in the sequence of all cells
 * to check in the iteration.
 */
static H3Index nextCell(H3Index cell) {
    int res = H3_GET_RESOLUTION(cell);
    while (true) {
        // If this is a base cell, set to next base cell (or H3_NULL if done)
        if (res == 0) {
            return baseCellNumToCell(H3_GET_BASE_CELL(cell) + 1);
        }

        // Faster cellToParent when we know the resolution is valid
        // and we're only moving up one level
        H3Index parent = cell;
        H3_SET_RESOLUTION(parent, res - 1);
        H3_SET_INDEX_DIGIT(parent, res, H3_DIGIT_MASK);

        // If not the last sibling of parent, return next sibling
        Direction digit = H3_GET_INDEX_DIGIT(cell, res);
        if (digit < INVALID_DIGIT - 1) {
            H3_SET_INDEX_DIGIT(cell, res,
                               digit + ((H3_EXPORT(isPentagon)(parent) &&
                                         digit == CENTER_DIGIT)
                                            ? 2  // Skip missing pentagon child
                                            : 1));
            return cell;
        }
        // Move up to the parent for the next loop iteration
        res--;
        cell = parent;
    }
}

/**
 * Internal function - initialize the iterator without stepping to the first
 * value
 */
static IterCellsPolygonCompact _iterInitPolygonCompact(
    const GeoPolygon *polygon, int res, uint32_t flags) {
    IterCellsPolygonCompact iter = {// Initialize output properties. The first
                                    // valid cell will be set in iterStep
                                    .cell = baseCellNumToCell(0),
                                    .error = E_SUCCESS,
                                    // Save input arguments
                                    ._polygon = polygon,
                                    ._res = res,
                                    ._flags = flags,
                                    ._bboxes = NULL,
                                    ._started = false};

    if (res < 0 || res > MAX_H3_RES) {
        iterErrorPolygonCompact(&iter, E_RES_DOMAIN);
        return iter;
    }

    H3Error flagErr = validatePolygonFlags(flags);
    if (flagErr) {
        iterErrorPolygonCompact(&iter, flagErr);
        return iter;
    }

    // Initialize bounding boxes for polygon and any holes. Memory allocated
    // here must be released through iterDestroyPolygonCompact
    iter._bboxes = H3_MEMORY(calloc)((polygon->numHoles + 1), sizeof(BBox));
    if (!iter._bboxes) {
        iterErrorPolygonCompact(&iter, E_MEMORY_ALLOC);
        return iter;
    }
    bboxesFromGeoPolygon(polygon, iter._bboxes);

    return iter;
}

/**
 * Initialize a IterCellsPolygonCompact struct representing the sequence of
 * compact cells within the target polygon. The test for including edge cells is
 * defined by the polyfill mode passed in the `flags` argument.
 *
 * Initialization of this object may fail, in which case the `error` property
 * will be set and all iteration will return H3_NULL. It is the responsibility
 * of the caller to check the error property after initialization.
 *
 * At any point in the iteration, starting once the struct is initialized, the
 * output value can be accessed through the `cell` property.
 *
 * Note that initializing the iterator allocates memory. If an iterator is
 * exhausted or returns an error that memory is released; otherwise it must be
 * released manually with iterDestroyPolygonCompact.
 *
 * @param  polygon Polygon to fill with compact cells
 * @param  res     Finest resolution for output cells
 * @param  flags   Bit mask of option flags
 * @return         Initialized iterator, with the first value available
 */
IterCellsPolygonCompact iterInitPolygonCompact(const GeoPolygon *polygon,
                                               int res, uint32_t flags) {
    IterCellsPolygonCompact iter = _iterInitPolygonCompact(polygon, res, flags);

    // Start the iterator by taking the first step.
    // This is necessary to have a valid value after initialization.
    iterStepPolygonCompact(&iter);

    return iter;
}

/**
 * Increment the polyfill iterator, running the polygon to cells algorithm.
 *
 * Briefly, the algorithm checks every cell in the global grid hierarchically,
 * starting with the base cells. Cells coarser than the target resolution are
 * checked for complete child inclusion using a bounding box guaranteed to
 * contain all children.
 * - If the bounding box is contained by the polygon, output is set to the cell
 * - If the bounding box intersects, recurse into the first child
 * - Otherwise, continue with the next cell in sequence
 *
 * For cells at the target resolution, a finer-grained check is used according
 * to the inclusion criteria set in flags.
 *
 * @param  iter Iterator to increment
 */
void iterStepPolygonCompact(IterCellsPolygonCompact *iter) {
    H3Index cell = iter->cell;

    // once the cell is H3_NULL, the iterator returns an infinite sequence of
    // H3_NULL
    if (cell == H3_NULL) return;

    // For the first step, we need to evaluate the current cell; after that, we
    // should start with the next cell.
    if (iter->_started) {
        cell = nextCell(cell);
    } else {
        iter->_started = true;
    }

    // Short-circuit iteration for 0-vert polygon
    if (iter->_polygon->geoloop.numVerts == 0) {
        iterDestroyPolygonCompact(iter);
        return;
    }

    ContainmentMode mode = FLAG_GET_CONTAINMENT_MODE(iter->_flags);

    while (cell) {
        int cellRes = H3_GET_RESOLUTION(cell);

        // Target res: Do a fine-grained check
        if (cellRes == iter->_res) {
            if (mode == CONTAINMENT_CENTER || mode == CONTAINMENT_OVERLAPPING ||
                mode == CONTAINMENT_OVERLAPPING_BBOX) {
                // Check if the cell center is inside the polygon
                LatLng center;
                H3Error centerErr = H3_EXPORT(cellToLatLng)(cell, &center);
                if (centerErr != E_SUCCESS) {
                    iterErrorPolygonCompact(iter, centerErr);
                    return;
                }
                if (pointInsidePolygon(iter->_polygon, iter->_bboxes,
                                       &center)) {
                    // Set to next output
                    iter->cell = cell;
                    return;
                }
            }
            if (mode == CONTAINMENT_OVERLAPPING ||
                mode == CONTAINMENT_OVERLAPPING_BBOX) {
                // For overlapping, we need to do a quick check to determine
                // whether the polygon is wholly contained by the cell. We
                // check the first polygon vertex, which if it is contained
                // could also mean we simply intersect.

                // Deferencing verts[0] is safe because we check numVerts above
                LatLng firstVertex = iter->_polygon->geoloop.verts[0];

                // We have to check whether the point is in the expected range
                // first, because out-of-bounds values will yield false
                // positives with latLngToCell
                if (bboxContains(&VALID_RANGE_BBOX, &firstVertex)) {
                    H3Index polygonCell;
                    H3Error polygonCellErr = H3_EXPORT(latLngToCell)(
                        &(iter->_polygon->geoloop.verts[0]), cellRes,
                        &polygonCell);
                    if (NEVER(polygonCellErr != E_SUCCESS)) {
                        // This should be unreachable with the bbox check
                        iterErrorPolygonCompact(iter, polygonCellErr);
                        return;
                    }
                    if (polygonCell == cell) {
                        // Set to next output
                        iter->cell = cell;
                        return;
                    }
                }
            }
            if (mode == CONTAINMENT_FULL || mode == CONTAINMENT_OVERLAPPING ||
                mode == CONTAINMENT_OVERLAPPING_BBOX) {
                CellBoundary boundary;
                H3Error boundaryErr =
                    H3_EXPORT(cellToBoundary)(cell, &boundary);
                if (boundaryErr != E_SUCCESS) {
                    iterErrorPolygonCompact(iter, boundaryErr);
                    return;
                }
                BBox bbox;
                H3Error bboxErr = cellToBBox(cell, &bbox, false);
                if (NEVER(bboxErr != E_SUCCESS)) {
                    // Should be unreachable - invalid cells would be caught in
                    // the previous boundaryErr
                    iterErrorPolygonCompact(iter, bboxErr);
                    return;
                }
                // Check if the cell is fully contained by the polygon
                if ((mode == CONTAINMENT_FULL ||
                     mode == CONTAINMENT_OVERLAPPING_BBOX) &&
                    cellBoundaryInsidePolygon(iter->_polygon, iter->_bboxes,
                                              &boundary, &bbox)) {
                    // Set to next output
                    iter->cell = cell;
                    return;
                }
                // For overlap, we've already checked for center point inclusion
                // above; if that failed, we only need to check for line
                // intersection
                else if ((mode == CONTAINMENT_OVERLAPPING ||
                          mode == CONTAINMENT_OVERLAPPING_BBOX) &&
                         cellBoundaryCrossesPolygon(
                             iter->_polygon, iter->_bboxes, &boundary, &bbox)) {
                    // Set to next output
                    iter->cell = cell;
                    return;
                }
            }
            if (mode == CONTAINMENT_OVERLAPPING_BBOX) {
                // Get a bounding box containing all the cell's children, so
                // this can work for the max size calculation
                BBox bbox;
                H3Error bboxErr = cellToBBox(cell, &bbox, true);
                if (bboxErr) {
                    iterErrorPolygonCompact(iter, bboxErr);
                    return;
                }
                if (bboxOverlapsBBox(&iter->_bboxes[0], &bbox)) {
                    CellBoundary bboxBoundary = bboxToCellBoundary(&bbox);
                    if (
                        // cell bbox contains the polygon
                        bboxContainsBBox(&bbox, &iter->_bboxes[0]) ||
                        // polygon contains cell bbox
                        pointInsidePolygon(iter->_polygon, iter->_bboxes,
                                           &bboxBoundary.verts[0]) ||
                        // polygon crosses cell bbox
                        cellBoundaryCrossesPolygon(iter->_polygon,
                                                   iter->_bboxes, &bboxBoundary,
                                                   &bbox)) {
                        iter->cell = cell;
                        return;
                    }
                }
            }
        }

        // Coarser cell: Check the bounding box
        if (cellRes < iter->_res) {
            // Get a bounding box for all of the cell's children
            BBox bbox;
            H3Error bboxErr = cellToBBox(cell, &bbox, true);
            if (bboxErr) {
                iterErrorPolygonCompact(iter, bboxErr);
                return;
            }
            if (bboxOverlapsBBox(&iter->_bboxes[0], &bbox)) {
                // Quick check for possible containment
                if (bboxContainsBBox(&iter->_bboxes[0], &bbox)) {
                    CellBoundary bboxBoundary = bboxToCellBoundary(&bbox);
                    // Do a fine-grained, more expensive check on the polygon
                    if (cellBoundaryInsidePolygon(iter->_polygon, iter->_bboxes,
                                                  &bboxBoundary, &bbox)) {
                        // Bounding box is fully contained, so all children are
                        // included. Set to next output.
                        iter->cell = cell;
                        return;
                    }
                }
                // Otherwise, the intersecting bbox means we need to test all
                // children, starting with the first child
                H3Index child;
                H3Error childErr =
                    H3_EXPORT(cellToCenterChild)(cell, cellRes + 1, &child);
                if (childErr) {
                    iterErrorPolygonCompact(iter, childErr);
                    return;
                }
                // Restart the loop with the child cell
                cell = child;
                continue;
            }
        }

        // Find the next cell in the sequence of all cells and continue
        cell = nextCell(cell);
    }
    // If we make it out of the loop, we're done
    iterDestroyPolygonCompact(iter);
}

/**
 * Destroy an iterator, releasing any allocated memory. Iterators destroyed in
 * this manner are safe to use but will always return H3_NULL.
 * @param  iter Iterator to destroy
 */
void iterDestroyPolygonCompact(IterCellsPolygonCompact *iter) {
    if (iter->_bboxes) {
        H3_MEMORY(free)(iter->_bboxes);
    }
    iter->cell = H3_NULL;
    iter->error = E_SUCCESS;
    iter->_polygon = NULL;
    iter->_res = -1;
    iter->_flags = 0;
    iter->_bboxes = NULL;
}

/**
 * Initialize a IterCellsPolygon struct representing the sequence of
 * cells within the target polygon. The test for including edge cells is defined
 * by the polyfill mode passed in the `flags` argument.
 *
 * Initialization of this object may fail, in which case the `error` property
 * will be set and all iteration will return H3_NULL. It is the responsibility
 * of the caller to check the error property after initialization.
 *
 * At any point in the iteration, starting once the struct is initialized, the
 * output value can be accessed through the `cell` property.
 *
 * Note that initializing the iterator allocates memory. If an iterator is
 * exhausted or returns an error that memory is released; otherwise it must be
 * released manually with iterDestroyPolygon.
 *
 * @param  polygon Polygon to fill with cells
 * @param  res     Resolution for output cells
 * @param  flags   Bit mask of option flags
 * @return         Initialized iterator, with the first value available
 */
IterCellsPolygon iterInitPolygon(const GeoPolygon *polygon, int res,
                                 uint32_t flags) {
    // Create the sub-iterator for compact cells
    IterCellsPolygonCompact cellIter =
        iterInitPolygonCompact(polygon, res, flags);
    // Create the sub-iterator for children
    IterCellsChildren childIter = iterInitParent(cellIter.cell, res);

    IterCellsPolygon iter = {.cell = childIter.h,
                             .error = cellIter.error,
                             ._cellIter = cellIter,
                             ._childIter = childIter};
    return iter;
}

/**
 * Increment the polyfill iterator, outputting the latest cell at the
 * desired resolution.
 *
 * @param  iter Iterator to increment
 */
void iterStepPolygon(IterCellsPolygon *iter) {
    if (iter->cell == H3_NULL) return;

    // See if there are more children to output
    iterStepChild(&(iter->_childIter));
    if (iter->_childIter.h) {
        iter->cell = iter->_childIter.h;
        return;
    }

    // Otherwise, increment the polyfill iterator
    iterStepPolygonCompact(&(iter->_cellIter));
    if (iter->_cellIter.cell) {
        _iterInitParent(iter->_cellIter.cell, iter->_cellIter._res,
                        &(iter->_childIter));
        iter->cell = iter->_childIter.h;
        return;
    }

    // All done, set to null and report errors if any
    iter->cell = H3_NULL;
    iter->error = iter->_cellIter.error;
}

/**
 * Destroy an iterator, releasing any allocated memory. Iterators destroyed in
 * this manner are safe to use but will always return H3_NULL.
 * @param  iter Iterator to destroy
 */
void iterDestroyPolygon(IterCellsPolygon *iter) {
    iterDestroyPolygonCompact(&(iter->_cellIter));
    // null out the child iterator by passing H3_NULL
    _iterInitParent(H3_NULL, 0, &(iter->_childIter));
    iter->cell = H3_NULL;
    iter->error = E_SUCCESS;
}

/**
 * polygonToCells takes a given GeoJSON-like data structure and preallocated,
 * zeroed memory, and fills it with the hexagons that are contained by
 * the GeoJSON-like data structure. Polygons are considered in Cartesian space.
 *
 * @param polygon The geoloop and holes defining the relevant area
 * @param res The Hexagon resolution (0-15)
 * @param flags Algorithm flags such as containment mode
 * @param size Maximum number of indexes to write to `out`.
 * @param out The slab of zeroed memory to write to. Must be at least of size
 * `size`.
 */
H3Error H3_EXPORT(polygonToCellsExperimental)(const GeoPolygon *polygon,
                                              int res, uint32_t flags,
                                              int64_t size, H3Index *out) {
    IterCellsPolygon iter = iterInitPolygon(polygon, res, flags);
    int64_t i = 0;
    for (; iter.cell; iterStepPolygon(&iter)) {
        if (i >= size) {
            iterDestroyPolygon(&iter);
            return E_MEMORY_BOUNDS;
        }
        out[i++] = iter.cell;
    }
    return iter.error;
}

static int MAX_SIZE_CELL_THRESHOLD = 10;

static double getAverageCellArea(int res) {
    double hexAreaKm2;
    H3_EXPORT(getHexagonAreaAvgKm2)(res, &hexAreaKm2);
    return hexAreaKm2;
}

/**
 * maxPolygonToCellsSize returns the number of cells to allocate space for
 * when performing a polygonToCells on the given GeoJSON-like data structure.
 * @param polygon A GeoJSON-like data structure indicating the poly to fill
 * @param res Hexagon resolution (0-15)
 * @param flags Bit mask of option flags
 * @param out number of cells to allocate for
 * @return 0 (E_SUCCESS) on success.
 */
H3Error H3_EXPORT(maxPolygonToCellsSizeExperimental)(const GeoPolygon *polygon,
                                                     int res, uint32_t flags,
                                                     int64_t *out) {
    // Special case: 0-vertex polygon
    if (polygon->geoloop.numVerts == 0) {
        *out = 0;
        return E_SUCCESS;
    }

    // Initialize the iterator without stepping, so we can adjust the res and
    // flags (after they are validated by the initialization) before we start
    IterCellsPolygonCompact iter = _iterInitPolygonCompact(polygon, res, flags);

    if (iter.error) {
        return iter.error;
    }

    // Ignore the requested flags and use the faster overlapping-bbox mode
    iter._flags = CONTAINMENT_OVERLAPPING_BBOX;

    // Get a (very) rough area of the polygon bounding box
    BBox *polygonBBox = &iter._bboxes[0];
    double polygonBBoxAreaKm2 =
        bboxHeightRads(polygonBBox) * bboxWidthRads(polygonBBox) /
        cos(fmin(fabs(polygonBBox->north), fabs(polygonBBox->south))) *
        EARTH_RADIUS_KM * EARTH_RADIUS_KM;

    // Determine the res for the size estimate, based on a (very) rough estimate
    // of the number of cells at various resolutions that would fit in the
    // polygon. All we need here is a general order of magnitude.
    while (iter._res > 0 &&
           polygonBBoxAreaKm2 / getAverageCellArea(iter._res - 1) >
               MAX_SIZE_CELL_THRESHOLD) {
        iter._res--;
    }

    // Now run the polyfill, counting the output in the target res.
    // We have to take the first step outside the loop, to get the first
    // valid output cell
    iterStepPolygonCompact(&iter);

    *out = 0;
    int64_t childrenSize;
    for (; iter.cell; iterStepPolygonCompact(&iter)) {
        H3_EXPORT(cellToChildrenSize)(iter.cell, res, &childrenSize);
        *out += childrenSize;
    }

    return iter.error;
}
