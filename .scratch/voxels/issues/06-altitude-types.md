# 06: Altitude types: mean sea level and above ground via a caller-supplied lookup

**What to build:** A user can locate a voxel from a mean-sea-level or above-ground altitude by passing altitude options with a lookup function (and opaque user data). The lookup returns geoid separation or terrain ellipsoidal height at a lat/lng. Ellipsoidal stays the default and needs no lookup. The core ships no geoid or terrain data (ADR-0001).

**Blocked by:** 02 (Tracer bullet: locate and inspect a voxel)

**Status:** ready-for-agent

- [ ] Altitude options carry the altitude type, an optional lookup and user data; omitting the options means ellipsoidal
- [ ] Mean sea level: ellipsoidal altitude = given altitude + geoid separation from the lookup
- [ ] Above ground: ellipsoidal altitude = terrain ellipsoidal height from the lookup + given altitude
- [ ] Mean sea level or above ground without a lookup returns the existing option-invalid error
- [ ] A lookup reporting an error makes the call return the lookup-failed error
- [ ] The lookup receives the caller's user data unchanged
- [ ] Tests use stub lookups (fixed offsets, failing lookup) and check the voxel equals the ellipsoidal result at the converted altitude
- [ ] The CLI's locate subcommand accepts an altitude type plus a fixed geoid/terrain offset; smoke test for each type
