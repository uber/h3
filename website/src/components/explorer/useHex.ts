// From: https://github.com/JosephChotard/h3-viewer/blob/21da63f9c5056296a6fe3a8f1ab5fbe8afebb754/src/useHex.ts
// Under MIT License
import { H3HexagonLayer } from "@deck.gl/geo-layers";
import { MapViewState, PickingInfo, WebMercatorViewport } from "deck.gl";
import { polygonToCells } from "h3-js";
import { useCallback, useEffect, useState } from "react";
import { throttle } from "./utils";

export type OurViewState = {
    width: number;
    height: number;
} & MapViewState;

function adjustToRange(value: number, min: number, max: number): number {
    const range = max - min;
    return ((((value - min) % range) + range) % range) + min;
}

function adjustLongitude(longitude: number): number {
    return adjustToRange(longitude, -180, 180);
}

type Bounds = {
    minLat: number;
    minLon: number;
    maxLat: number;
    maxLon: number;
};

const getVisibleBounds = (viewState: OurViewState) => {
    try{
    const viewport = new WebMercatorViewport(viewState);
    const { width, height } = viewState;

    const topLeft = viewport.unproject([0, 0]);
    const bottomRight = viewport.unproject([width, height]);

    const minLat = bottomRight[1];
    const minLon = topLeft[0];
    const maxLat = topLeft[1];
    const maxLon = bottomRight[0];

    return {
        minLat,
        minLon,
        maxLat,
        maxLon,
    } as Bounds;
} catch (e) {
    console.error("Problem", e);
    return {
        minLat: -1,
        minLon: -1,
        maxLat: 1,
        maxLon: 1,
    } as Bounds;
}
};

const splitPolygon = ({ minLat, minLon, maxLat, maxLon }: Bounds) =>
    splitLongitudeRange(minLon, maxLon).map(([minLon, maxLon]) => ({
        minLat,
        minLon,
        maxLat,
        maxLon,
    }));

function splitLongitudeRange(
    minLon: number,
    maxLon: number
): [number, number][] {
    const result: [number, number][] = [];

    let curPos = minLon;

    const adjustedMax = adjustLongitude(maxLon);

    while (curPos < maxLon) {
        const normalized = adjustLongitude(curPos);
        const next = normalized < 0 ? 0 : 180;
        curPos = curPos + next - normalized;
        if (curPos > maxLon) {
            result.push([normalized, adjustedMax]);
        } else {
            result.push([normalized, next]);
        }
    }

    return result;
}

const boundsToPolygon = ({ minLat, minLon, maxLat, maxLon }: Bounds) => [
    [minLon, minLat],
    [maxLon, minLat],
    [maxLon, maxLat],
    [minLon, maxLat],
    [minLon, minLat],
];

export const ZOOM_TO_RESOLUTION: Record<number, number> = {
    0: 0,
    1: 0,
    2: 1,
    3: 2,
    4: 2,
    5: 3,
    6: 4,
    7: 5,
    8: 5,
    9: 6,
    10: 6,
    11: 7,
    12: 8,
    13: 9,
    14: 10,
    15: 11,
    16: 11,
    17: 12,
    18: 12,
    19: 13,
    20: 14,
    21: 15,
};

export const RESOLUTION_TO_ZOOM: Record<number, number> = Object.entries(ZOOM_TO_RESOLUTION).reduce((acc, [k, v]) => ({ ...acc, [v]: +k }), {});

const getHexagons = (bounds: Bounds, resolution: number) => {
    const all_bounds = splitPolygon(bounds);

    const polygons = all_bounds.map(boundsToPolygon);

    const hexagons = [
        ...new Set(
            polygons.flatMap((polygon) =>
                polygonToCells(polygon, resolution, true)
            )
        ),
    ];
    return hexagons
}

export interface UseHexProps {
    resolutionFrozen: boolean;
    addSelectedHexes: (hexes: string[]) => void;
}

export const useHex = ({ resolutionFrozen, addSelectedHexes }: UseHexProps) => {
    const [hexagons, setHexagons] = useState<string[]>([]);
    const [resolution, setResolution] = useState<number>(0);
    const [hexLayer, setHexLayer] = useState<H3HexagonLayer<string> | null>(
        null
    );

    useEffect(() => {
        setHexLayer(
            new H3HexagonLayer<string>({
                id: "H3HexagonLayer",
                extruded: false,
                getHexagon: (d: string) => d,
                getFillColor: [0, 0, 0, 1],
                getLineColor: [150, 150, 150, 100],
                getLineWidth: 2,
                lineWidthUnits: "pixels",
                elevationScale: 20,
                pickable: true,
                data: hexagons,
                wrapLongitude: true,
                onClick: (info: PickingInfo<string>) => {
                    if (info.object) {
                        addSelectedHexes([info.object]);
                    }
                }
            })
        );
    }, [addSelectedHexes, hexagons])

    // eslint-disable-next-line react-hooks/exhaustive-deps
    const handleResize = useCallback(
        throttle((viewState: OurViewState) => {
            if (resolutionFrozen) {
                return
            }
            const zoom = viewState.zoom;
            const resolution = ZOOM_TO_RESOLUTION[Math.round(zoom)];
            setResolution(resolution)
            console.log("viewState", viewState, resolution)
            const bounds = getVisibleBounds(viewState);
            console.log(bounds);
            const hexagons = getHexagons(bounds, resolution);
            console.log(hexagons);
            setHexagons(hexagons);
        }, 200),
        [setHexagons, setResolution, resolutionFrozen]
    );

    return { handleResize, hexLayer, resolution };
}