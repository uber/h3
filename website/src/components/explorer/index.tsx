// Contains code adapted from https://observablehq.com/@nrabinowitz/h3-index-inspector under the ISC license

import React, { useCallback, useMemo, ReactNode } from "react";
import { isValidCell, latLngToCell, getResolution } from "h3-js";
import {
  Banner,
  BannerContainer,
  HeroExampleContainer,
  DemoContainer,
} from "../styled";
import { useQueryState } from "use-location-state";
import { SelectedHexDetails } from "./details";
import { ExplorerMap } from "./map";
import { WhereAmIButton } from "./where-am-i";
import geojson2h3 from "geojson2h3";
import wkt from "wkt";
import { Feature, MultiPolygon, Polygon } from "geojson";

const CELL_COUNT_THRESHOLD = 50;
const CELL_COUNT_UPPER_THRESHOLD = 5000;

function fullyTrim(str: string) {
  if (!str) {
    return "";
  }

  return str.trim();
}

function fullyUnwrap(str: string) {
  if (!str) {
    return "";
  }

  str = str.trim();
  // Remove any JSON wrapper characters
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

function maybeDecimalCell(input: string) {
  try {
    if (/^\d+$/.test(input)) {
      const asBigInt = BigInt(input);
      const asHex = asBigInt.toString(16);
      if (isValidCell(asHex)) {
        return asHex;
      }
    }
  } catch {
    // Ignore
  }
  return null;
}

function maybePrefixedHexCell(input: string) {
  if (input && input.startsWith("0x")) {
    return input.substring(2);
  }
  return null;
}

function geoJsonToCells(
  geoJson: Feature | Polygon | MultiPolygon,
  userResolution: number,
) {
  // TODO: Handle point geometries, lines, etc. Only polygons and multipolygons supported.
  // TODO: Pass containment in to featureToH3Set.
  if (geoJson.type === "Polygon" || geoJson.type === "MultiPolygon") {
    geoJson = {
      type: "Feature",
      geometry: geoJson,
    } as Feature;
  }

  const hasUserResolution = userResolution !== -1;

  for (let res = 0; res < 16; res++) {
    const cells = geojson2h3.featureToH3Set(geoJson, res);
    if (
      (!hasUserResolution &&
        (cells.length > CELL_COUNT_THRESHOLD || res === 15)) ||
      (hasUserResolution &&
        (cells.length > CELL_COUNT_UPPER_THRESHOLD || res === userResolution))
    ) {
      return {
        splitUserInput: cells,
        showCellId: false,
        showResolutionInput: res,
        inputGeoJson: geoJson,
      };
    }
  }
  return null;
}

function tryParsePolygonInput(input: string, userResolution: number) {
  try {
    const parsed = JSON.parse(input) as Feature;
    if (parsed && parsed.type) {
      const geoJsonResult = geoJsonToCells(parsed, userResolution);
      if (geoJsonResult) {
        return geoJsonResult;
      }
    }
  } catch {
    // No-op
  }

  try {
    const parsed = wkt.parse(input) as Feature;
    if (parsed && parsed.type) {
      const wktResult = geoJsonToCells(parsed, userResolution);
      if (wktResult) {
        return wktResult;
      }
    }
  } catch {
    // No-op
  }
}

function doSplitUserInput(userInput: string, userResolution: number) {
  if (userInput) {
    // Acceptable inputs, in order of test preference:
    // GeoJSON
    // WKT
    // Valid hexadecimal cell ID
    // Valid hexadecimal cell ID, prefixed by 0x
    // Valid decimal cell ID
    // lat,lng coordinate pairs

    const resultPolygon = tryParsePolygonInput(userInput, userResolution);
    if (resultPolygon) {
      return resultPolygon;
    }

    let showCellId = false;
    let showResolutionInput = null;
    const unwrapAnyArray = fullyUnwrap(userInput);
    const split = unwrapAnyArray.split(/\s/).filter((str) => str !== "");
    const result = [];

    for (let i = 0; i < split.length; i++) {
      const currentInput = fullyTrim(split[i]);
      const nextInput = fullyTrim(split[i + 1]);
      const cellIdFromDecimal = maybeDecimalCell(currentInput);
      const cellIdFromPrefixedHex = maybePrefixedHexCell(currentInput);

      if (isValidCell(currentInput)) {
        result.push(currentInput);
      } else if (
        cellIdFromPrefixedHex !== null &&
        isValidCell(cellIdFromPrefixedHex)
      ) {
        result.push(cellIdFromPrefixedHex);
        showCellId = true;
      } else if (cellIdFromDecimal) {
        result.push(cellIdFromDecimal);
        // Show what the cell ID would look like normally
        showCellId = true;
      } else if (
        i < split.length - 1 &&
        Number.isFinite(Number.parseFloat(currentInput)) &&
        Number.isFinite(Number.parseFloat(nextInput))
      ) {
        const lat = Number.parseFloat(currentInput);
        const lng = Number.parseFloat(nextInput);

        if (userResolution === -1) {
          // Note this order is important for picking to work correctly
          for (let res = 0; res < 16; res++) {
            result.push(latLngToCell(lat, lng, res));
          }

          // We don't need to set showCellId, because we are showing multiple cell IDs
          // anyways, so they will be clickable.
        } else {
          result.push(latLngToCell(lat, lng, userResolution));
          showCellId = true;
        }
        showResolutionInput = -1;
        // consumed, skip next coordinate
        i++;
      }
    }

    return {
      splitUserInput: result,
      showResolutionInput,
      showCellId,
      inputGeoJson: null,
    };
  }

  return {
    splitUserInput: [],
    showCellId: false,
    showResolutionInput: null,
    inputGeoJson: null,
  };
}

function zoomToResolution(zoom: number) {
  return Math.max(Math.min(zoom / 1.5, 15), 0);
}

export default function HomeExporer({ children }: { children: ReactNode }) {
  const [userInput, setUserInput] = useQueryState("hex", "");
  const [userResolution, setUserResolution] = useQueryState<number>("res", -1);

  const { splitUserInput, showCellId, inputGeoJson, showResolutionInput } =
    useMemo(
      () => doSplitUserInput(userInput, userResolution),
      [userInput, userResolution],
    );
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
    ({ hex }: { hex: string }) => {
      const asSet = new Set(splitUserInput);
      if (!asSet.delete(hex)) {
        asSet.add(hex);
      }
      setUserInput([...asSet].join(", "));
    },
    [splitUserInput, setUserInput],
  );
  const coordinateOnClick = useCallback(
    ({
      coordinate,
      zoom,
      resolution,
    }: {
      coordinate: [number, number];
      zoom: number;
      resolution?: number;
    }) => {
      if (constantResolution !== undefined) {
        const asSet = new Set(splitUserInput);
        asSet.add(
          latLngToCell(coordinate[1], coordinate[0], constantResolution),
        );
        setUserInput([...asSet].join(", "));
      } else if (splitUserInput.length === 0) {
        const detectedRes =
          resolution !== undefined ? resolution : zoomToResolution(zoom);
        setUserInput(
          `${latLngToCell(coordinate[1], coordinate[0], detectedRes)}`,
        );
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
            <ExplorerMap
              userInput={splitUserInput}
              inputGeoJson={inputGeoJson}
              userValidHex={userValidHex}
              objectOnClick={objectOnClick}
              coordinateOnClick={coordinateOnClick}
            />
          </DemoContainer>
        </HeroExampleContainer>
        <BannerContainer>
          <textarea
            value={userInput}
            onChange={(e) => {
              setUserInput(e.target.value);
            }}
            placeholder="Click on map or enter cell IDs"
            style={{
              marginRight: "0.5rem",
              height: "3em",
              minHeight: "2em",
              maxHeight: "10em",
              width: "100%",
              resize: "vertical",
            }}
          />
          {splitUserInput.length ? (
            <SelectedHexDetails
              splitUserInput={splitUserInput}
              showCellId={showCellId}
              setUserInput={setUserInput}
              showNavigation={false}
              showDetails={true}
            />
          ) : null}
          {showResolutionInput !== null ? (
            <div>
              <input
                type="number"
                min="0"
                max="15"
                placeholder="Resolution"
                value={`${userResolution}`}
                onChange={(e) => {
                  try {
                    const res = parseInt(e.target.value, 10);
                    if (!isNaN(res) && res >= 0 && res <= 15) {
                      setUserResolution(res);
                    }
                  } catch (err) {
                    // Ignore
                    console.error(err);
                  }
                }}
              />
            </div>
          ) : null}
        </BannerContainer>
        <WhereAmIButton setUserInput={setUserInput} />
      </Banner>
      {children}
    </>
  );
}
