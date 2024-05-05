import {makeExample} from '../components';

import React, {Component, useEffect, useMemo, useRef} from 'react';
import {Map} from 'react-map-gl/maplibre';
import DeckGL from '@deck.gl/react';
import {H3HexagonLayer} from '@deck.gl/geo-layers';
import {PathStyleExtension} from '@deck.gl/extensions';
import {getRes0Cells, isValidCell, uncompactCells} from 'h3-js';

const INITIAL_VIEW_STATE = {
  longitude: -74,
  latitude: 40.72,
  zoom: 13,
  pitch: 45,
  bearing: 0
};

const MAP_STYLE = 'https://basemaps.cartocdn.com/gl/positron-gl-style/style.json';

export function App({
  hex = undefined,
  initialViewState = INITIAL_VIEW_STATE,
  mapStyle = MAP_STYLE,
}) {
  const deckRef = useRef();
  const res0Cells = useMemo(() => getRes0Cells().map((hex) => ({hex})), []);
  const res1Cells = useMemo(() => uncompactCells(getRes0Cells(), 1).map((hex) => ({hex})), []);
  const res2Cells = useMemo(() => uncompactCells(getRes0Cells(), 2).map((hex) => ({hex})), []);
  const userValidHex = useMemo(() => isValidCell(hex), [hex]);
  useEffect(() => {
    if (userValidHex && deckRef.current) {
      //TODO: zoom to bounds here...
    }
  }, [hex, userValidHex]);

  const layers = userValidHex? [new H3HexagonLayer({
    id: "userhex",
    data: [{hex}],
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
      extensions: [new PathStyleExtension({dash: true})]
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
      extensions: [new PathStyleExtension({dash: true})]
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

class HomeDemo extends Component {
  static mapStyle = undefined; // MAPBOX_STYLES.LIGHT;

  constructor(props) {
    super(props);

    this.initialViewState = {
      longitude: -74.012,
      latitude: 40.705,
      zoom: 2.5,
      pitch: 0,
      bearing: 0
    };
  }

  render() {
    const {data, ...otherProps} = this.props;

    return (
      <App
        {...otherProps}
        initialViewState={this.initialViewState}
      />
    );
  }
}

export default makeExample(HomeDemo, {isInteractive: false, style: {}});
