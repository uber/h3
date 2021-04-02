module.exports = {
  someSidebar: {
    "Getting Started": [
      "README",
      {
        type: "category",
        label: "Highlights",
        items: [
          "highlights/aggregation",
          "highlights/joining",
          "highlights/flowmodel",
          "highlights/ml",
          "highlights/indexing",
        ],
      },
      {
        type: "category",
        label: "Comparisons",
        items: [
          "comparisons/s2",
          "comparisons/geohash",
          "comparisons/hexbin",
          "comparisons/admin",
          "comparisons/placekey",
        ],
      },
      "installation",
      "quickstart",
      // TODO: Add FAQ here
    ],
    "API Reference": [
      "api/indexing",
      "api/inspection",
      "api/traversal",
      "api/hierarchy",
      "api/regions",
      "api/uniedge",
      "api/misc",
    ],
    Community: [
      "community/bindings",
      "community/libraries",
      "community/tutorials",
      "community/applications",
    ],
    "H3 Internals": [
      "core-library/overview",
      "core-library/h3Indexing",
      "core-library/coordsystems",
      "core-library/geoToH3desc",
      "core-library/h3ToGeoDesc",
      "core-library/h3ToGeoBoundaryDesc",
      "core-library/filters",
      "core-library/usage",
      "core-library/restable",
    ],
  },
};
