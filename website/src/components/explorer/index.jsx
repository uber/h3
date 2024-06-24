// Contains code adapted from https://observablehq.com/@nrabinowitz/h3-index-inspector under the ISC license

import React, { useCallback, useMemo } from "react";
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

function doSplitUserInput(userInput) {
  if (userInput) {
    // TODO: support WKT, GeoJSON inputs here too
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

function zoomToResolution(zoom) {
  return Math.max(Math.min(zoom / 1.5, 15), 0);
}

export default function HomeExporer({ children }) {
  const [userInput, setUserInput] = useQueryState("hex", "");

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
    ({ coordinate, zoom }) => {
      if (constantResolution !== undefined) {
        const asSet = new Set(splitUserInput);
        asSet.add(
          latLngToCell(coordinate[1], coordinate[0], constantResolution),
        );
        setUserInput([...asSet].join(", "));
      } else if (splitUserInput.length === 0) {
        const detectedRes = zoomToResolution(zoom);
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
              setUserInput={setUserInput}
              showNavigation={false}
              showDetails={true}
            />
          ) : (
            <></>
          )}
        </BannerContainer>
        <WhereAmIButton setUserInput={setUserInput} />
      </Banner>
      {children}
    </>
  );
}
