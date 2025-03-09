// Contains code adapted from https://observablehq.com/@nrabinowitz/h3-index-inspector under the ISC license

import React, {
  useCallback,
  useEffect,
  useMemo,
  useRef,
  useState,
} from "react";
import { Map } from "react-map-gl";
import DeckGL from "@deck.gl/react";
import { H3HexagonLayer } from "@deck.gl/geo-layers";
import { WebMercatorViewport, FlyToInterpolator } from "@deck.gl/core";
import { getRes0Cells, uncompactCells, cellToBoundary } from "h3-js";
import useDocusaurusContext from "@docusaurus/useDocusaurusContext";
import { MOBILE_CUTOFF_WINDOW_WIDTH } from "../common";
import { useHex } from "./useHex";

const INITIAL_VIEW_STATE = {
  longitude: -74.012,
  latitude: 40.705,
  zoom: 2.5,
  pitch: 0,
  bearing: 0,
  maxZoom: 22,
  minZoom: 0,
};

const MAP_STYLE = "mapbox://styles/mapbox/light-v11";

export function ExplorerMap({
  userInput = [],
  userValidHex = false,
  initialViewState = INITIAL_VIEW_STATE,
  mapStyle = MAP_STYLE,
  objectOnClick = undefined,
  coordinateOnClick = undefined,
}) {
  const context = useDocusaurusContext();
  const [currentInitialViewState, setCurrentInitialViewState] =
    useState(initialViewState);
  const [deckLoaded, setDeckLoaded] = useState(false);
  const deckRef = useRef();
  const res0Cells = useMemo(() => getRes0Cells().map((hex) => ({ hex })), []);
  const res1Cells = useMemo(
    () => uncompactCells(getRes0Cells(), 1).map((hex) => ({ hex })),
    [],
  );
  const res2Cells = useMemo(
    () => uncompactCells(getRes0Cells(), 2).map((hex) => ({ hex })),
    [],
  );
  const [windowWidth, setWindowWidth] = useState(null);

  useEffect(() => {
    const updateWidth = () => {
      setWindowWidth(window.width);
    };

    updateWidth();

    window.addEventListener("resize", updateWidth);
    return () => window.removeEventListener("resize", updateWidth);
  }, [setWindowWidth]);

  useEffect(() => {
    if (userValidHex && deckRef.current) {
      const { width, height } = deckRef.current.deck;

      const viewport = new WebMercatorViewport({ width, height });
      let minX = Infinity;
      let minY = Infinity;
      let maxX = -Infinity;
      let maxY = -Infinity;

      for (const hex of userInput) {
        const coords = cellToBoundary(hex, true);
        for (const coord of coords) {
          if (coord[0] < minX) {
            minX = coord[0];
          } else if (coord[0] > maxX) {
            maxX = coord[0];
          }
          if (coord[1] < minY) {
            minY = coord[1];
          } else if (coord[1] > maxY) {
            maxY = coord[1];
          }
        }
      }

      if (
        Number.isFinite(minX) &&
        Number.isFinite(minY) &&
        Number.isFinite(maxX) &&
        Number.isFinite(maxY) &&
        width > 1 &&
        height > 1
      ) {
        const { latitude, longitude, zoom } = viewport.fitBounds(
          [
            [minX, minY],
            [maxX, maxY],
          ],
          { padding: 96 },
        );

        setCurrentInitialViewState({
          latitude,
          longitude,
          zoom,
          transitionInterpolator: new FlyToInterpolator(),
          transitionDuration: 1600,
        });
      }
    }
  }, [userInput, userValidHex, deckLoaded]);

const addSelectedHexes = useCallback((hex) => {
  objectOnClick({ hex });
}, [objectOnClick]);

  const {
    handleResize: hexHandleResize,
    hexLayers: backgroundHexLayers,
    resolution,
} = useHex({
    resolutionFrozen: false,
    addSelectedHexes,
});

  const layers = userValidHex
    ? [
        new H3HexagonLayer({
          id: "userhex",
          data: userInput.map((hex) => ({ hex })),
          getHexagon: (d) => d.hex,
          extruded: false,
          filled: false,
          stroked: true,
          getLineColor: [0, 0, 0],
          getLineWidth: 2,
          lineWidthUnits: "pixels",
          lineWidthMinPixels: 2,
          highPrecision: true,
          pickable: true,
          filled: true,
          getFillColor: [0, 0, 0, 30],
        }),
      ]
    : backgroundHexLayers;

  const getTooltip = useCallback(({ object }) => {
    if (object && object.hex) {
      return {
        html: `<tt>${object.hex}</tt>`,
      };
    }
  }, []);

  const getCursor = useCallback(({ isHovering }) => {
    if (isHovering) {
      return "pointer";
    } else {
      return "auto";
    }
  }, []);

  const onClick = useCallback(
    ({ object, coordinate, viewport }) => {
      if (object && object.hex) {
        if (objectOnClick) {
          objectOnClick({ hex: object.hex });
        }
      } else if (object && object instanceof string) {
        if (objectOnClick) {
          objectOnClick({ hex: object });
        }
      } else {
        // if (coordinateOnClick) {
        //   coordinateOnClick({ coordinate, zoom: viewport.zoom });
        // }
      }
    },
    [objectOnClick, coordinateOnClick],
  );

  return (
    <DeckGL
      ref={deckRef}
      layers={layers}
      initialViewState={currentInitialViewState}
      onViewStateChange={({viewState}) => {
        hexHandleResize(viewState);
      }}
      getTooltip={getTooltip}
      getCursor={getCursor}
      onClick={onClick}
      onLoad={() => setDeckLoaded(true)}
      controller={{
        dragPan: windowWidth && windowWidth >= MOBILE_CUTOFF_WINDOW_WIDTH,
      }}
      touchAction="pan-y"
    >
      <Map
        reuseMaps
        interactive={false}
        projection={"mercator"}
        mapboxAccessToken={context.siteConfig.customFields.mapboxAccessToken}
        mapStyle={mapStyle}
      />
    </DeckGL>
  );
}
