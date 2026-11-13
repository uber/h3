# Voxels use a 64-bit index; resolutions 11–13 cover narrow altitude ranges and 14–15 have none

Voxels must be easy to use from many languages and databases, so a voxel index is a single 64-bit integer with bit 63 kept at 0 (fits a signed `BIGINT`), like an H3 cell. The H3 layout leaves only 3 reserved bits plus 3 bits per unused resolution digit, so a voxel's layer number fits the full altitude range only up to resolution 10 (244,944 layers in 18 bits). Resolutions 11–13 instead cover narrow altitude ranges sharing a floor at about −597 m and reaching about +1,440 km, +81 km and +2.8 km respectively; each narrow range is made of whole layers of the next coarser resolution, so every voxel has either all of its children or none. Resolutions 14–15 have no voxels because their ranges (about 140 m and 9 m) are too thin to be useful.

## Considered Options

- **128-bit voxel index (cell index + 64-bit vertical word)**: every resolution over the full range, footprint verbatim; rejected because few languages or databases have a native 128-bit integer.
- **64-bit, resolutions 0–10 only**: rejected; strictly less capable than narrow ranges for 11–13.
- **Claiming unused mode values or bit 63 for extra layer bits**: would roughly 8× or 2× the narrow ranges (e.g. resolution 13 to ~6 km with bit 63, covering high plateaus), but upstream H3 reserves both and could claim them; bit 63 also makes voxel indices negative in signed 64-bit types. Rejected; bit 63 is the switch to revisit if high-altitude-terrain drone work matters.

## Consequences

- Narrow ranges are fixed in ellipsoidal altitude and do not follow terrain: resolution 13 does not reach drones over high plateaus (Altiplano, Tibet).
- "Every point in the altitude range is in a voxel" holds per resolution, over that resolution's own range.
