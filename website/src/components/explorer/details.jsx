// Contains code adapted from https://observablehq.com/@nrabinowitz/h3-index-inspector under the ISC license

import React, { useCallback } from "react";
import {
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

const THREE_BITS = 7; // 1 | 2 | 4

function h3IndexToDigits(h) {
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

function cellUnits(hex) {
  return getResolution(hex) < 8
    ? { area: UNITS.km2, dist: UNITS.km }
    : { area: UNITS.m2, dist: UNITS.m };
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

export function SelectedHexDetails({ setUserInput, splitUserInput }) {
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
