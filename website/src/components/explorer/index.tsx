// Contains code adapted from https://observablehq.com/@nrabinowitz/h3-index-inspector under the ISC license

import React, { useCallback, useMemo, ReactNode, useState, useId } from "react";
import {
  isValidCell,
  isValidDirectedEdge,
  latLngToCell,
  getResolution,
} from "h3-js";
import {
  Banner,
  BannerContainer,
  HeroExampleContainer,
  DemoContainer,
} from "../styled";
import { useQueryState } from "use-location-state";
import { SelectedEdgeDetails, SelectedHexDetails } from "./details";
import { ExplorerMap } from "./map";
import { WhereAmIButton } from "./where-am-i";
import { doSplitUserInput } from "./parseInput";
import { useColorMode } from "@docusaurus/theme-common";

function zoomToResolution(zoom: number) {
  return Math.max(Math.min(zoom / 1.5, 15), 0);
}

export default function HomeExporer({ children }: { children: ReactNode }) {
  const [userInput, setUserInput] = useQueryState("hex", "");
  const [userResolution, setUserResolution] = useQueryState<number>("res", -1);
  const [previewCells, setPreviewCells] = useState<string[]>([]);
  const { colorMode } = useColorMode();
  const resolutionInputId = useId();

  const { splitUserInput, showCellId, inputGeoJson, showResolutionInput } =
    useMemo(
      () => doSplitUserInput(userInput, userResolution),
      [userInput, userResolution],
    );
  // Cell and edge indexes are kept apart from here on. The cell functions used
  // below and in the details panel return plausible but meaningless values for
  // an edge index rather than throwing, so an edge reaching them shows up as
  // wrong data rather than an error.
  const cells = useMemo(
    () => splitUserInput.filter(isValidCell),
    [splitUserInput],
  );
  const edges = useMemo(
    () => splitUserInput.filter(isValidDirectedEdge),
    [splitUserInput],
  );
  const userValidInput = cells.length > 0 || edges.length > 0;
  const constantResolution = useMemo(() => {
    const resAsSet = new Set(cells.map(getResolution));
    if (resAsSet.size === 1) {
      return [...resAsSet][0];
    } else {
      return undefined;
    }
  }, [cells]);

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
              userInput={cells}
              userEdges={edges}
              inputGeoJson={inputGeoJson}
              userValidInput={userValidInput}
              objectOnClick={objectOnClick}
              coordinateOnClick={coordinateOnClick}
              previewCells={previewCells}
            />
          </DemoContainer>
        </HeroExampleContainer>
        <BannerContainer colorMode={colorMode}>
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
          {cells.length ? (
            <SelectedHexDetails
              splitUserInput={cells}
              showCellId={showCellId}
              setUserInput={setUserInput}
              showNavigation={true}
              showDetails={true}
              onHoverCells={setPreviewCells}
            />
          ) : null}
          {edges.length === 1 ? <SelectedEdgeDetails edge={edges[0]} /> : null}
          {showResolutionInput !== null ? (
            <div>
              <label htmlFor={resolutionInputId}>Resolution:</label>
              <input
                id={resolutionInputId}
                type="number"
                min="0"
                max="15"
                placeholder="Auto"
                value={`${userResolution !== -1 ? userResolution : ""}`}
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
                style={{
                  marginLeft: "0.25em",
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
