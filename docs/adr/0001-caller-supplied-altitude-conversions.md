# Altitude-type conversions use caller-supplied lookups, not bundled data

Voxels are defined on WGS84 ellipsoidal altitude, but callers also need to locate voxels from mean-sea-level and above-ground altitudes. Converting those requires a geoid model (EGM96 is a few MB, EGM2008 hundreds of MB) or terrain data (far larger). We keep the C core dependency-free and data-free, matching upstream H3: ellipsoidal altitude is built in and the default, while mean-sea-level and above-ground conversions take a caller-supplied lookup that returns geoid or terrain height at a lat/lng. Language bindings may bundle a geoid model (e.g. EGM96) as a convenience.

## Considered Options

- **Bundle EGM96 in the C core**: rejected; adds megabytes of data to a library that ships none, and locks every user to one geoid model.
- **Ellipsoidal only, no conversion support**: rejected; mean-sea-level and above-ground altitudes are what aviation and drone users actually have.
