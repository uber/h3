
import React, { useEffect, useMemo, useRef, useState } from 'react';
import { Map } from 'react-map-gl/maplibre';
import DeckGL from '@deck.gl/react';
import { H3HexagonLayer } from '@deck.gl/geo-layers';
import { PathStyleExtension } from '@deck.gl/extensions';
import { getRes0Cells, isValidCell, uncompactCells } from 'h3-js';
import styled from 'styled-components';
import { Banner, BannerContainer, HeroExampleContainer } from './styled';
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

export function App({
  hex = undefined,
  initialViewState = INITIAL_VIEW_STATE,
  mapStyle = MAP_STYLE,
}) {
  const deckRef = useRef();
  const res0Cells = useMemo(() => getRes0Cells().map((hex) => ({ hex })), []);
  const res1Cells = useMemo(() => uncompactCells(getRes0Cells(), 1).map((hex) => ({ hex })), []);
  const res2Cells = useMemo(() => uncompactCells(getRes0Cells(), 2).map((hex) => ({ hex })), []);
  const userValidHex = useMemo(() => isValidCell(hex), [hex]);
  useEffect(() => {
    if (userValidHex && deckRef.current) {
      //TODO: zoom to bounds here...
    }
  }, [hex, userValidHex]);

  const layers = userValidHex ? [new H3HexagonLayer({
    id: "userhex",
    data: [{ hex }],
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

export default function HomeExplorer({ children }) {
  const [hex, setHex] = useState("");

  // Note: The Layout "wrapper" component adds header and footer etc
  return (
    <>
      <Banner>
        <BrowserOnly>
          {() => <HeroExampleContainer>
            <DemoContainer>
              <App
                hex={hex}
              />
            </DemoContainer>
          </HeroExampleContainer>}
        </BrowserOnly>
        <BannerContainer>
          {/* <ProjectName>{siteConfig.title}</ProjectName>
          <p>{siteConfig.tagline}</p>
          <GetStartedLink href="./docs/get-started/getting-started">GET STARTED</GetStartedLink> */}
          <input type="text" value={hex} onChange={(e) => { setHex(e.target.value); }} />
        </BannerContainer>
      </Banner>
      {children}
    </>
  );
}
