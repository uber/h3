
import React, { useCallback, useEffect, useMemo, useRef, useState } from 'react';
import { Map } from 'react-map-gl/maplibre';
import DeckGL from '@deck.gl/react';
import { H3HexagonLayer } from '@deck.gl/geo-layers';
import { PathStyleExtension } from '@deck.gl/extensions';
import { getRes0Cells, isValidCell, uncompactCells, latLngToCell } from 'h3-js';
import styled from 'styled-components';
import { Banner, BannerContainer, HeroExampleContainer, ProjectName } from './styled';
import BrowserOnly from '@docusaurus/BrowserOnly';

const INITIAL_VIEW_STATE = {
  longitude: -74.012,
  latitude: 40.705,
  zoom: 2.5,
  pitch: 0,
  bearing: 0,
};

const MAP_STYLE = 'https://basemaps.cartocdn.com/gl/positron-gl-style/style.json';

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
  str = str.replaceAll('[', ' ');
  str = str.replaceAll(']', ' ');
  str = str.replaceAll('"', ' ');
  str = str.replaceAll("'", ' ');
  str = str.replaceAll(",", ' ');

  // Remove any Python set wrapper
  str = str.replaceAll('{', ' ');
  str = str.replaceAll('}', ' ');

  return str;
}

export function App({
  userInput = undefined,
  initialViewState = INITIAL_VIEW_STATE,
  mapStyle = MAP_STYLE,
}) {
  const deckRef = useRef();
  const res0Cells = useMemo(() => getRes0Cells().map((hex) => ({ hex })), []);
  const res1Cells = useMemo(() => uncompactCells(getRes0Cells(), 1).map((hex) => ({ hex })), []);
  const res2Cells = useMemo(() => uncompactCells(getRes0Cells(), 2).map((hex) => ({ hex })), []);
  const splitUserInput = useMemo(() => {
    if (userInput) {
      const unwrapAnyArray = fullyUnwrap(userInput);
      const split = unwrapAnyArray.split(/\s/).filter((str) => str !== "");
      const result = [];

      for (let i = 0; i < split.length; i++) {
        const currentInput = fullyTrim(split[i]);
        const nextInput = fullyTrim(split[i + 1]);

        if (isValidCell(currentInput)) {
          result.push(currentInput);
        } else if (i < split.length - 1 && Number.isFinite(Number.parseFloat(currentInput)) && Number.isFinite(Number.parseFloat(nextInput))) {
          const lat = Number.parseFloat(currentInput);
          const lng = Number.parseFloat(nextInput);

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
  }, [userInput]);

  const userValidHex = useMemo(() => splitUserInput.map(isValidCell).includes(true), [splitUserInput]);
  useEffect(() => {
    if (userValidHex && deckRef.current) {
      // TODO: zoom to bounds here...
    }
  }, [userInput, userValidHex]);

  const layers = userValidHex ? [new H3HexagonLayer({
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
  })] : [
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
      lineWidthUnits: 'pixels',
      getDashArray: [5, 1],
      dashJustified: true,
      dashGapPickable: true,
      extensions: [new PathStyleExtension({ dash: true })]
    }),
    new H3HexagonLayer({
      id: "res2",
      data: res2Cells,
      getHexagon: d => d.hex,
      extruded: false,
      filled: false,
      stroked: true,
      getLineColor: [0, 0, 0],
      getLineWidth: 1,
      lineWidthMinPixels: 1,
      highPrecision: true,
      lineWidthUnits: 'pixels',
      getDashArray: [5, 5],
      dashJustified: true,
      dashGapPickable: true,
      extensions: [new PathStyleExtension({ dash: true })]
    })
  ];

  return (
    <DeckGL
      ref={deckRef}
      layers={layers}
      initialViewState={initialViewState}
      controller={true}
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
      navigator.geolocation.getCurrentPosition((position) => {
        setUserInput(`${position.coords.latitude}, ${position.coords.longitude}`)
        setGeolocationStatus("");
      }, () => {
        setGeolocationStatus("Error");
      });
    } else {
      setGeolocationStatus("No location services");
    }
  }, [setUserInput]);

  // Note: The Layout "wrapper" component adds header and footer etc
  return (
    <>
      <Banner>
        <HeroExampleContainer>
          <DemoContainer>
            <App
              userInput={userInput}
            />
          </DemoContainer>
        </HeroExampleContainer>
        <BannerContainer>
          <ProjectName>H3</ProjectName>
          <p>Enter coordinates or cell IDs</p>
          {/* <GetStartedLink href="./docs/get-started/getting-started">GET STARTED</GetStartedLink> */}
          <input type="text" value={userInput} onChange={(e) => { setUserInput(e.target.value); }} placeholder='822d57fffffffff' />
          <p>
            <input type="button" value="Where am I?" onClick={doGeoLocation} />
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
      <BrowserOnly>
        {() => <HomeExplorerInternal />}
      </BrowserOnly>
      {children}
    </>
  );
}
