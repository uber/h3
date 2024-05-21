// Contains code adapted from https://observablehq.com/@nrabinowitz/h3-index-inspector under the ISC license

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
  h3IndexToSplitLong,
  cellArea,
  edgeLength,
  originToDirectedEdges,
  UNITS,
} from "h3-js";
import styled from "styled-components";
import { Banner, BannerContainer, HeroExampleContainer } from "./styled";
import { useQueryState } from "use-location-state";

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

function cellUnits(hex) {
  return getResolution(hex) < 8
    ? { area: UNITS.km2, dist: UNITS.km }
    : { area: UNITS.m2, dist: UNITS.m };
}

export function App({
  userInput = [],
  userValidHex = false,
  initialViewState = INITIAL_VIEW_STATE,
  mapStyle = MAP_STYLE,
  objectOnClick = undefined,
  coordinateOnClick = undefined,
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

  useEffect(() => {
    // TODO: This doesn't set the viewport right on restoring state
    // TODO: Do not do this on manual (click on map) selection
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
  }, [userInput, userValidHex]);

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
          getLineWidth: 3,
          lineWidthMinPixels: 3,
          highPrecision: true,
          pickable: true,
          filled: true,
          // transparent fill purely for picking
          getFillColor: [0, 0, 0, 30],
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
    ({ object, coordinate }) => {
      if (object && object.hex) {
        // TODO: click to copy?
        if (objectOnClick) {
          objectOnClick({ hex: object.hex });
        }
      } else {
        if (coordinateOnClick) {
          coordinateOnClick(coordinate);
        }
      }
    },
    [objectOnClick, coordinateOnClick],
  );

  return (
    <DeckGL
      ref={deckRef}
      layers={layers}
      initialViewState={currentInitialViewState}
      controller={true}
      getTooltip={getTooltip}
      getCursor={getCursor}
      onClick={onClick}
    >
      <Map reuseMaps mapStyle={mapStyle} />
    </DeckGL>
  );
}

function ClickableH3Index({ hex, setUserInput }) {
  const onClick = useCallback(() => {
    setUserInput(hex);
  }, [hex, setUserInput]);

  return (
    <a onClick={onClick} style={{ cursor: "pointer" }}>
      {hex}
    </a>
  );
}

function ClickableH3IndexList({ hexes, setUserInput, showAll = true }) {
  const onShowAllClick = useCallback(() => {
    setUserInput(hexes.join(", "));
  }, [hexes, setUserInput]);
  return (
    <>
      {hexes.map((hex, index) => {
        const link = (
          <ClickableH3Index key={index} setUserInput={setUserInput} hex={hex} />
        );
        if (index === 0) {
          return link;
        } else {
          return <span key={index}>, {link}</span>;
        }
      })}
      {showAll ? (
        <>
          &nbsp;
          <a onClick={onShowAllClick} style={{ cursor: "pointer" }}>
            (show all)
          </a>
        </>
      ) : (
        <></>
      )}
    </>
  );
}

function SelectedHexDetails({ setUserInput, splitUserInput }) {
  if (splitUserInput.length === 1) {
    const hex = splitUserInput[0];
    const units = cellUnits(hex);
    const res = getResolution(hex);
    const baseCell = getBaseCellNumber(hex);
    const pent = isPentagon(hex);
    const faces = getIcosahedronFaces(hex).join(", ");
    const coords = cellToLatLng(hex)
      .map((n) => n.toPrecision(res / 3 + 7))
      .join(", ");
    const digits =
      res === 0 ? "(none)" : h3IndexToDigits(hex).slice(0, res).join("");
    const boundary = cellToBoundary(hex);
    const area = cellArea(hex, units.area);
    const edgeLengths = originToDirectedEdges(hex).map((e) =>
      edgeLength(e, units.dist),
    );
    const meanEdgeLength =
      edgeLengths.reduce((prev, curr) =>
        prev !== undefined ? prev + curr : curr,
      ) / edgeLengths.length;
    const parent = res !== 0 && cellToParent(hex, res - 1);
    const children = res !== 15 && cellToChildren(hex, res + 1);
    const neighbors = new Set(gridDisk(hex, 1));
    neighbors.delete(hex);

    return (
      <p>
        Resolution: <tt>{res}</tt>
        <br />
        Center: <tt>{coords}</tt>
        <br />
        Parent:{" "}
        {parent ? (
          <ClickableH3Index hex={parent} setUserInput={setUserInput} />
        ) : (
          <tt>(none)</tt>
        )}
        <br />
        Children:{" "}
        {children ? (
          <ClickableH3IndexList hexes={children} setUserInput={setUserInput} />
        ) : (
          <tt>(none)</tt>
        )}
        <br />
        Neighbors:{" "}
        <ClickableH3IndexList
          hexes={[...neighbors]}
          setUserInput={setUserInput}
        />
        <br />
        <details>
          <summary>Details</summary>
          Base cell: <tt>{baseCell}</tt>
          <br />
          Pentagon: <tt>{`${pent}`}</tt>
          <br />
          Icosa Faces: <tt>{faces}</tt>
          <br /># of Boundary Verts: <tt>{boundary.length}</tt>
          <br />
          Cell Area: <tt>{area}</tt> {units.area}
          <br />
          Mean Edge Length: <tt>{meanEdgeLength}</tt> {units.dist}
          <br />
          Indexing Digits: <tt>{digits}</tt>
        </details>
      </p>
    );
  } else {
    return (
      <p>
        <ClickableH3IndexList
          hexes={splitUserInput}
          setUserInput={setUserInput}
          showAll={false}
        />
      </p>
    );
  }
}

export default function HomeExporer({ children }) {
  const [userInput, setUserInput] = useQueryState("hex", "");
  const [geolocationStatus, setGeolocationStatus] = useState("");

  const doGeoLocation = useCallback(async () => {
    if ("geolocation" in navigator) {
      setGeolocationStatus("Locating...");
      navigator.geolocation.getCurrentPosition(
        (position) => {
          // TODO: Consider using the accuracy to select resolution
          const newHexes = [];
          for (let res = 0; res <= 15; res++) {
            newHexes.push(
              latLngToCell(
                position.coords.latitude,
                position.coords.longitude,
                res,
              ),
            );
          }
          setUserInput(newHexes.join(", "));
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

  const splitUserInput = useMemo(() => {
    return doSplitUserInput(userInput);
  }, [userInput]);
  const userValidHex = useMemo(
    () => splitUserInput.map(isValidCell).includes(true),
    [splitUserInput],
  );
  const constantResolution = useMemo(() => {
    const resAsSet = new Set(splitUserInput.map(getResolution));
    if (resAsSet.size === 1) {
      return [...resAsSet][0];
    } else {
      return undefined;
    }
  }, [splitUserInput]);

  const objectOnClick = useCallback(
    ({ hex }) => {
      const asSet = new Set(splitUserInput);
      if (!asSet.delete(hex)) {
        asSet.add(hex);
      }
      setUserInput([...asSet].join(", "));
    },
    [splitUserInput, setUserInput],
  );
  const coordinateOnClick = useCallback(
    (coordinate) => {
      if (constantResolution !== undefined) {
        const asSet = new Set(splitUserInput);
        asSet.add(
          latLngToCell(coordinate[1], coordinate[0], constantResolution),
        );
        setUserInput([...asSet].join(", "));
      } else if (splitUserInput.length === 0) {
        // TODO: Detect desired resolution based on zoom?
        const newHexes = [];
        for (let res = 0; res <= 15; res++) {
          newHexes.push(latLngToCell(coordinate[1], coordinate[0], res));
        }
        const prev = splitUserInput.length
          ? `${splitUserInput.join(", ")},`
          : "";
        setUserInput(`${prev}${newHexes.join(", ")}`);
      }
    },
    [splitUserInput, setUserInput, constantResolution],
  );

  // Note: The Layout "wrapper" component adds header and footer etc
  return (
    <>
      <Banner>
        <HeroExampleContainer>
          <DemoContainer>
            <App
              userInput={splitUserInput}
              userValidHex={userValidHex}
              objectOnClick={objectOnClick}
              coordinateOnClick={coordinateOnClick}
            />
          </DemoContainer>
        </HeroExampleContainer>
        <BannerContainer>
          <textarea
            type="text"
            value={userInput}
            onChange={(e) => {
              setUserInput(e.target.value);
            }}
            placeholder="Click on map or enter cell IDs"
            style={{ marginRight: "0.5rem", width: "50%", height: "3em", minHeight: "2em", maxWidth: "100%" }}
          />
          <input
            type="button"
            value="Where am I?"
            onClick={doGeoLocation}
            style={{ marginRight: "0.5rem" }}
          />
          {geolocationStatus}
          {splitUserInput.length ? (
            <SelectedHexDetails
              splitUserInput={splitUserInput}
              setUserInput={setUserInput}
            />
          ) : (
            <></>
          )}
        </BannerContainer>
      </Banner>
      {children}
    </>
  );
}
