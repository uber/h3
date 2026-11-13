# Voxel indexes pack cell fields upward so each column is one contiguous range

A voxel index is laid out as `0 | mode | resolution | base cell | digits (3 × resolution) | layer number`, with the layer number (stored unsigned, counted from the bottom of that resolution's altitude range) filling the low bits. This differs from the H3 cell layout, where the 3 reserved bits sit above the base cell and unused digits are filled with 7s: keeping that layout would put the top bits of the layer number above the base cell at resolutions 10, 12 and 13, splitting a column into up to 8 separate runs when sorted. Packing makes voxels sort exactly like their footprint cells and then by altitude, so a whole column is one range scan in any database.

## Consequences

- Recovering the footprint is a shift and mask plus restoring the cell layout's reserved bits and trailing 7s, rather than a mask alone.
