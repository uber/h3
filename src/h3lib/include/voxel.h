/*
 * Copyright 2026 J Reynolds
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
/** @file voxel.h
 * @brief Internal voxel layer functions. The public API is h3voxel.h.
 */

#ifndef VOXEL_H
#define VOXEL_H

#include <stdbool.h>
#include <stdint.h>

/**
 * The valid layer numbers at a resolution, as [min, maxExclusive). Returns
 * false when the resolution has no voxels.
 */
bool layerRange(int res, int32_t *min, int32_t *maxExclusive);

/**
 * The floor altitude, in metres, of a layer at a resolution. This is the
 * single definition of layer boundaries: a layer holds exactly the altitudes
 * from its floor up to, but not including, the next layer's floor.
 */
double layerFloorM(int32_t layer, int res);

#endif
