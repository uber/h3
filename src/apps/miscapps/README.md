# miscapps

These apps generate data, either static tables used in the library or test data used in regression tests.

## Generate Test Data

For a specific base cell at a specific res:

    ./bin/cellToLatLngHier -p 800bfffffffffff -r 8 > tests/inputfiles/bc05r08centers.txt
    ./bin/cellToBoundaryHier -p 800bfffffffffff -r 8 > tests/inputfiles/bc05r08cells.txt

For all cells at a given res:

    ./bin/h3ToHier -r 0 | xargs -I {} ./bin/cellToBoundaryHier -p {} -r 3 > tests/inputfiles/res03cells.txt

For a random sample of cells at a given res:

    ./bin/mkRandGeoBoundary -n 5000 -r 5 > tests/inputfiles/rand05cells.txt
