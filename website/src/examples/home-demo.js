// import {MAPBOX_STYLES, DATA_URI} from '../constants/defaults';
// import App from 'website-examples/trips/app'; // eslint-disable-line

import {_GlobeView as GlobeView} from '@deck.gl/core';

import {makeExample} from '../components';

import React, {useState, useEffect, Component, useMemo} from 'react';
import {Map} from 'react-map-gl/maplibre';
import DeckGL from '@deck.gl/react';
import {SolidPolygonLayer} from '@deck.gl/layers';
import {H3HexagonLayer} from '@deck.gl/geo-layers';
import {animate} from 'popmotion';
import {getRes0Cells, uncompactCells} from 'h3-js';

const INITIAL_VIEW_STATE = {
  longitude: -74,
  latitude: 40.72,
  zoom: 13,
  pitch: 45,
  bearing: 0
};

const MAP_STYLE = 'https://basemaps.cartocdn.com/gl/positron-gl-style/style.json';

export function App({
  initialViewState = INITIAL_VIEW_STATE,
  mapStyle = MAP_STYLE,
  loopLength = 1800, // unit corresponds to the timestamp in source data
  animationSpeed = 1
}) {
  const [time, setTime] = useState(0);

  useEffect(() => {
    const animation = animate({
      from: 0,
      to: loopLength,
      duration: (loopLength * 60) / animationSpeed,
      repeat: Infinity,
      onUpdate: setTime
    });
    return () => animation.stop();
  }, [loopLength, animationSpeed]);

  const res0Cells = useMemo(() => getRes0Cells().map((hex) => ({hex})), []);
  const res1Cells = useMemo(() => uncompactCells(getRes0Cells(), 1).map((hex) => ({hex})), []);
  const res2Cells = useMemo(() => uncompactCells(getRes0Cells(), 2).map((hex) => ({hex})), []);

  const layers = [
    // For GlobeView
    // new SolidPolygonLayer({
    //   id: 'background',
    //   data: [
    //     [[-180, 90], [0, 90], [180, 90], [180, -90], [0, -90], [-180, -90]]
    //   ],
    //   getPolygon: d => d,
    //   stroked: false,
    //   filled: true,
    //   getFillColor: [178, 178, 255]
    // }),
    new H3HexagonLayer({
      id: "res0",
      data: res0Cells,
      getHexagon: (d) => d.hex,
      extruded: false,
      filled: false,
      stroked: true,
      getLineColor: [255, 0, 255],
      getLineWidth: 12.5,
      lineWidthMinPixels: 5,
      highPrecision: true,
    }),
    new H3HexagonLayer({
      id: "res1",
      data: res1Cells,
      getHexagon: (d) => d.hex,
      extruded: false,
      filled: false,
      stroked: true,
      getLineColor: [0, 255, 0],
      getLineWidth: 7.5,
      lineWidthMinPixels: 7.5/2,
      highPrecision: true,
    }),
    new H3HexagonLayer({
      id: "res2",
      data: res2Cells,
      getHexagon: d => d.hex,
      extruded: false,
      filled: false,
      stroked: true,
      getLineColor: [0, 0, 0],
      getLineWidth: 2.5,
      lineWidthMinPixels: 1,
      highPrecision: true,
    })
  ];

  return (
    <DeckGL
      layers={layers}
      // effects={theme.effects}
      initialViewState={initialViewState}
      controller={true}
      // TODO: GlobeView causes problems
      // views={new GlobeView({id: 'globe', controller: true})}
      parameters={{cull: true}}
    >
      <Map reuseMaps mapStyle={mapStyle} />
    </DeckGL>
  );
}

class HomeDemo extends Component {
  static data = [
    // {
    //   url: `${DATA_URI}/trips-data-s.txt`,
    //   worker: '/workers/trips-data-decoder.js?loop=1800&trail=180'
    // },
    // {
    //   url: `${DATA_URI}/building-data-s.txt`,
    //   worker: '/workers/building-data-decoder.js'
    // }
  ];

  static mapStyle = undefined; // MAPBOX_STYLES.LIGHT;

  constructor(props) {
    super(props);

    // const material = {
    //   ambient: 0.2,
    //   diffuse: 0.6,
    //   shininess: 32,
    //   specularColor: [232, 232, 247]
    // };

    // const lightingEffect = new LightingEffect({
    //   light1: new AmbientLight({
    //     color: [255, 255, 255],
    //     intensity: 1.0
    //   }),
    //   light2: new DirectionalLight({
    //     color: [255, 255, 255],
    //     intensity: 2.0,
    //     direction: [-1, -2, -3],
    //     _shadow: true
    //   })
    // });

    // lightingEffect.shadowColor = [0, 0, 0, 0.3];

    // this.theme = {
    //   buildingColor: [232, 232, 247],
    //   trailColor0: [91, 145, 244],
    //   trailColor1: [115, 196, 150],
    //   material,
    //   effects: [lightingEffect]
    // };

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
        trips={(data && data[0]) || null}
        buildings={(data && data[1]) || null}
        trailLength={180}
        animationSpeed={1}
        // theme={this.theme}
        initialViewState={this.initialViewState}
      />
    );
  }
}

export default makeExample(HomeDemo, {isInteractive: false, style: {}});
