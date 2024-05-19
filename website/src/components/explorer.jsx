import React, {
  useCallback,
  useEffect,
  useMemo,
  useRef,
  useState,
} from "react";
import { Map } from "react-map-gl/maplibre";
import DeckGL from "@deck.gl/react";
import { H3HexagonLayer } from "@deck.gl/geo-layers";
import { PathStyleExtension } from "@deck.gl/extensions";
import { WebMercatorViewport, FlyToInterpolator } from "@deck.gl/core";
import {
  getRes0Cells,
  isValidCell,
  uncompactCells,
  latLngToCell,
  cellToBoundary,
  cellToParent,
  getResolution,
  cellToChildren,
  gridDisk,
  getBaseCellNumber,
  isPentagon,
  getIcosahedronFaces,
  cellToLatLng,
  cellToCenterChild,
  h3IndexToSplitLong,
} from "h3-js";
import styled from "styled-components";
import { Banner, BannerContainer, HeroExampleContainer } from "./styled";
import BrowserOnly from "@docusaurus/BrowserOnly";
import copy from "copy-text-to-clipboard";

const INITIAL_VIEW_STATE = {
  longitude: -74.012,
  latitude: 40.705,
  zoom: 2.5,
  pitch: 0,
  bearing: 0,
};

const MAP_STYLE =
  "https://basemaps.cartocdn.com/gl/positron-gl-style/style.json";

const DemoContainer = styled.div`
  height: 100%;
  .tooltip,
  .deck-tooltip {
    position: absolute;
    padding: 4px 12px;
    background: rgba(0, 0, 0, 0.8);
    color: var(--ifm-color-white);
    max-width: 300px;
    font-size: 12px;
    z-index: 9;
    pointer-events: none;
    white-space: nowrap;
  }
`;

function h3IndexToDigits(h) {
  const THREE_BITS = 0x1 | 0x2 | 0x4;
  const split = h3IndexToSplitLong(h);
  const digit1 = (split[1] >> 0xa) & THREE_BITS;
  const digit2 = (split[1] >> 0x7) & THREE_BITS;
  const digit3 = (split[1] >> 0x4) & THREE_BITS;
  const digit4 = (split[1] >> 0x1) & THREE_BITS;
  const digit5 = ((split[1] & 0x1) << 2) | ((split[0] >> 0x1e) & 0x3);
  const digit6 = (split[0] >> 0x1b) & THREE_BITS;
  const digit7 = (split[0] >> 0x18) & THREE_BITS;
  const digit8 = (split[0] >> 0x15) & THREE_BITS;
  const digit9 = (split[0] >> 0x12) & THREE_BITS;
  const digit10 = (split[0] >> 0xf) & THREE_BITS;
  const digit11 = (split[0] >> 0xc) & THREE_BITS;
  const digit12 = (split[0] >> 0x9) & THREE_BITS;
  const digit13 = (split[0] >> 0x6) & THREE_BITS;
  const digit14 = (split[0] >> 0x3) & THREE_BITS;
  const digit15 = split[0] & THREE_BITS;
  return [
    digit1,
    digit2,
    digit3,
    digit4,
    digit5,
    digit6,
    digit7,
    digit8,
    digit9,
    digit10,
    digit11,
    digit12,
    digit13,
    digit14,
    digit15,
  ];
}

function fullyTrim(str) {
  if (!str) {
    return "";
  }

  return str.trim();
}

function fullyUnwrap(str) {
  if (!str) {
    return "";
  }

  str = str.trim();
  // Remove any JSON wrapper stuff
  str = str.replaceAll("[", " ");
  str = str.replaceAll("]", " ");
  str = str.replaceAll('"', " ");
  str = str.replaceAll("'", " ");
  str = str.replaceAll(",", " ");

  // Remove any Python set wrapper
  str = str.replaceAll("{", " ");
  str = str.replaceAll("}", " ");

  return str;
}

function doSplitUserInput(userInput) {
  if (userInput) {
    const unwrapAnyArray = fullyUnwrap(userInput);
    const split = unwrapAnyArray.split(/\s/).filter((str) => str !== "");
    const result = [];

    for (let i = 0; i < split.length; i++) {
      const currentInput = fullyTrim(split[i]);
      const nextInput = fullyTrim(split[i + 1]);

      if (isValidCell(currentInput)) {
        result.push(currentInput);
      } else if (
        i < split.length - 1 &&
        Number.isFinite(Number.parseFloat(currentInput)) &&
        Number.isFinite(Number.parseFloat(nextInput))
      ) {
        const lat = Number.parseFloat(currentInput);
        const lng = Number.parseFloat(nextInput);

        // Note this order is important for picking to work correctly
        for (let res = 0; res < 16; res++) {
          result.push(latLngToCell(lat, lng, res));
        }

        // consumed, skip next coordinate
        i++;
      }
    }

    return result;
  }

  return [];
}

export function App({
  userInput = undefined,
  initialViewState = INITIAL_VIEW_STATE,
  mapStyle = MAP_STYLE,
  objectOnClick = undefined,
}) {
  const [currentInitialViewState, setCurrentInitialViewState] =
    useState(initialViewState);
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
  const splitUserInput = useMemo(() => {
    return doSplitUserInput(userInput);
  }, [userInput]);

  const userValidHex = useMemo(
    () => splitUserInput.map(isValidCell).includes(true),
    [splitUserInput],
  );
  useEffect(() => {
    if (userValidHex && deckRef.current) {
      const { width, height } = deckRef.current.deck;

      const viewport = new WebMercatorViewport({ width, height });
      let minX = Infinity;
      let minY = Infinity;
      let maxX = -Infinity;
      let maxY = -Infinity;

      for (const hex of splitUserInput) {
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
        Number.isFinite(maxY)
      ) {
        const { latitude, longitude, zoom } = viewport.fitBounds(
          [
            [minX, minY],
            [maxX, maxY],
          ],
          { padding: 48 },
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
  }, [userInput, userValidHex, splitUserInput]);

  const layers = userValidHex
    ? [
      new H3HexagonLayer({
        id: "userhex",
        data: splitUserInput.map((hex) => ({ hex })),
        getHexagon: (d) => d.hex,
        extruded: false,
        filled: false,
        stroked: true,
        getLineColor: [0, 0, 0],
        getLineWidth: 3,
        lineWidthMinPixels: 3,
        highPrecision: true,
        pickable: true,
        filled: true,
        // transparent fill purely for picking
        getFillColor: [0, 0, 0, 0],
      }),
    ]
    : [
      new H3HexagonLayer({
        id: "res0",
        data: res0Cells,
        getHexagon: (d) => d.hex,
        extruded: false,
        filled: false,
        stroked: true,
        getLineColor: [0, 0, 0],
        getLineWidth: 3,
        lineWidthMinPixels: 3,
        highPrecision: true,
        pickable: true,
        filled: true,
        // transparent fill purely for picking
        getFillColor: [0, 0, 0, 0],
      }),
      new H3HexagonLayer({
        id: "res1",
        data: res1Cells,
        getHexagon: (d) => d.hex,
        extruded: false,
        filled: false,
        stroked: true,
        getLineColor: [0, 0, 0],
        getLineWidth: 2,
        lineWidthMinPixels: 2,
        highPrecision: true,
        lineWidthUnits: "pixels",
        getDashArray: [5, 1],
        dashJustified: true,
        dashGapPickable: true,
        extensions: [new PathStyleExtension({ dash: true })],
      }),
      new H3HexagonLayer({
        id: "res2",
        data: res2Cells,
        getHexagon: (d) => d.hex,
        extruded: false,
        filled: false,
        stroked: true,
        getLineColor: [0, 0, 0],
        getLineWidth: 1,
        lineWidthMinPixels: 1,
        highPrecision: true,
        lineWidthUnits: "pixels",
        getDashArray: [5, 5],
        dashJustified: true,
        dashGapPickable: true,
        extensions: [new PathStyleExtension({ dash: true })],
      }),
    ];

  const getTooltip = useCallback(({ object }) => {
    if (object && object.hex) {
      const res = getResolution(object.hex);
      const baseCell = getBaseCellNumber(object.hex);
      const pent = isPentagon(object.hex);
      const faces = getIcosahedronFaces(object.hex).join(", ");
      const coords = cellToLatLng(object.hex)
        .map((n) => n.toPrecision(res / 3 + 7))
        .join(", ");
      const digits =
        res === 0
          ? "(none)"
          : h3IndexToDigits(object.hex).slice(0, res).join("");

      return {
        html: `<tt>${object.hex}</tt><br/>Resolution: <tt>${res}</tt><br/>Base cell: <tt>${baseCell}</tt><br/>Pentagon: <tt>${pent}</tt><br/>Icosa Faces: <tt>${faces}</tt><br/>Center: <tt>${coords}</tt><br/>Indexing Digits: <tt>${digits}</tt>`,
      };
    }
  }, []);

  const onClick = useCallback(({ object }) => {
    if (object && object.hex) {
      // TODO: click to copy?
      objectOnClick({ hex: object.hex });
    }
  }, []);

  return (
    <DeckGL
      ref={deckRef}
      layers={layers}
      initialViewState={currentInitialViewState}
      controller={true}
      getTooltip={getTooltip}
      onClick={onClick}
    >
      <Map reuseMaps mapStyle={mapStyle} />
    </DeckGL>
  );
}

export function HomeExplorerInternal({ children }) {
  const [userInput, setUserInput] = useState("");
  const [geolocationStatus, setGeolocationStatus] = useState("");

  const doGeoLocation = useCallback(async () => {
    if ("geolocation" in navigator) {
      setGeolocationStatus("Locating...");
      navigator.geolocation.getCurrentPosition(
        (position) => {
          setUserInput(
            `${position.coords.latitude}, ${position.coords.longitude}`,
          );
          setGeolocationStatus("");
        },
        () => {
          setGeolocationStatus("Error");
        },
      );
    } else {
      setGeolocationStatus("No location services");
    }
  }, [setUserInput]);

  const objectOnClick = useCallback(
    ({ hex }) => {
      setUserInput(hex);
    },
    [setUserInput],
  );

  const safeSetUserInput = useCallback(
    (newUserInput) => {
      const allCommas = newUserInput.match(/,/g) || [];
      if (allCommas.length > 10000) {
        if (
          !confirm(
            `This would render about ${allCommas.length} hexagons, are you sure? This can cause a lot of slowdown or crash your tab.`,
          )
        ) {
          return;
        }
      }
      setUserInput(newUserInput);
    },
    [setUserInput],
  );

  const doMapParents = useCallback(() => {
    const hexes = doSplitUserInput(userInput);
    const newHexes = new Set();
    for (const hex of hexes) {
      const newResolution = getResolution(hex) - 1;
      newHexes.add(cellToParent(hex, newResolution < 0 ? 0 : newResolution));
    }
    safeSetUserInput([...newHexes].join(","));
  }, [userInput, safeSetUserInput]);
  const doMapChildren = useCallback(() => {
    const hexes = doSplitUserInput(userInput);
    const newHexes = new Set();
    for (const hex of hexes) {
      const newResolution = getResolution(hex) + 1;
      const children = cellToChildren(
        hex,
        newResolution > 15 ? 15 : newResolution,
      );
      for (const child of children) {
        newHexes.add(child);
      }
    }
    safeSetUserInput([...newHexes].join(","));
  }, [userInput, safeSetUserInput]);
  const doMapCenterChild = useCallback(() => {
    const hexes = doSplitUserInput(userInput);
    const newHexes = new Set();
    for (const hex of hexes) {
      const newResolution = getResolution(hex) + 1;
      newHexes.add(
        cellToCenterChild(hex, newResolution > 15 ? 15 : newResolution),
      );
    }
    safeSetUserInput([...newHexes].join(","));
  }, [userInput, safeSetUserInput]);
  const doMapNeighbors = useCallback(() => {
    const hexes = doSplitUserInput(userInput);
    const newHexes = new Set();
    for (const hex of hexes) {
      const neighbors = gridDisk(hex, 1);
      for (const neighbor of neighbors) {
        newHexes.add(neighbor);
      }
    }
    safeSetUserInput([...newHexes].join(","));
  }, [userInput, safeSetUserInput]);

  // Note: The Layout "wrapper" component adds header and footer etc
  return (
    <>
      <Banner>
        <HeroExampleContainer>
          <DemoContainer>
            <App userInput={userInput} objectOnClick={objectOnClick} />
          </DemoContainer>
        </HeroExampleContainer>
        <BannerContainer>
          {/* <ProjectName>H3</ProjectName> */}
          <p style={{ marginBottom: 0 }}>Enter coordinates or cell IDs</p>
          {/* <GetStartedLink href="./docs/get-started/getting-started">GET STARTED</GetStartedLink> */}
          <input
            type="text"
            value={userInput}
            onChange={(e) => {
              setUserInput(e.target.value);
            }}
            placeholder="822d57fffffffff"
          />
          <p style={{ marginTop: "1rem", marginBottom: 0 }}>
            <input
              type="button"
              value="Parents"
              onClick={doMapParents}
              style={{ marginRight: "0.5rem" }}
            />
            <input
              type="button"
              value="Children"
              onClick={doMapChildren}
              style={{ marginRight: "0.5rem" }}
            />
            {/* <input
              type="button"
              value="Center Child"
              onClick={doMapCenterChild}
              style={{ marginRight: "0.5rem" }}
            /> */}
            <input
              type="button"
              value="Neighbors"
              onClick={doMapNeighbors}
              style={{ marginRight: "0.5rem" }}
            />
          </p>
          <p style={{ marginTop: "1rem", marginBottom: 0 }}>
            <input
              type="button"
              value="Where am I?"
              onClick={doGeoLocation}
              style={{ marginRight: "0.5rem" }}
            />
            {geolocationStatus}
          </p>
        </BannerContainer>
      </Banner>
      {children}
    </>
  );
}

export default function HomeExplorer({ children }) {
  return (
    <>
      <BrowserOnly>{() => <HomeExplorerInternal />}</BrowserOnly>
      {children}
    </>
  );
}
