# Layers are log-radial and nest exactly across resolutions

Layer thickness grows in proportion to distance from the Earth's centre (layers are uniform in log-radius), so a voxel keeps roughly the same proportions from the ground to geostationary orbit. At the ground, the target thickness is √(average cell area) at that resolution, so voxels are roughly as tall as they are wide. Layers nest exactly: each layer splits into a whole number of child layers, following a fixed schedule for res 1→15 of `3 2 3 3 2 3 3 3 2 3 3 2 3 3 2`, which keeps thickness within 0.83–1.21× of the target at every resolution. Altitude zero is a layer boundary at every resolution, and the full altitude range is 14 whole resolution-0 layers (one below zero, thirteen above): about −1,780 km to +443,000 km, the most a 64-bit voxel index holds at resolution 10 (see ADR-0004).

## Considered Options

- **Fixed thickness per resolution**: simpler arithmetic and readable boundaries, but voxels flatten to ~6.6× wider than tall at GEO and need ~3× more layers. Rejected because GEO coverage is a requirement.
- **Exactly proportional, non-nested layers** (thickness shrinks by √7 per resolution): rejected because a child voxel could straddle two parents, and each resolution's top layer would overshoot the range differently.
- **Range clipped exactly to −50 km / +100,000 km**: rejected because voxels at either end would be cut short, needing special cases throughout.

## Consequences

- Layer boundaries are irrational altitudes (e.g. not round metres), and locating a layer uses `log`/`exp`, so points exactly on a boundary may classify differently across platforms.
- Thickness is only roughly proportional (±20%), because 2 and 3 bracket the ideal √7 ≈ 2.65 per resolution.
