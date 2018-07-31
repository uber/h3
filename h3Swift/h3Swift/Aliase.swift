//
//  Aliase.swift
//  h3Swift
//
//  Created by 十亀眞怜 on 2018/07/27.
//  Copyright © 2018年 Pocket7878. All rights reserved.
//

import Foundation
import h3

public let MAX_CELL_BNDRY_VERTS = 10
public typealias GeoCoord = h3.GeoCoord
public typealias GeoBoundary = h3.GeoBoundary
public typealias Geofence = h3.Geofence
public typealias GeoPolygon = h3.GeoPolygon
public typealias GeoMultiPolygon = h3.GeoMultiPolygon
public typealias LinkedGeoCoord = h3.LinkedGeoCoord
public typealias LinkedGeoPolygon = h3.LinkedGeoPolygon

public extension GeoCoord {
    public static var zero: GeoCoord {
        return GeoCoord(lat: 0, lon: 0)
    }
}

public extension GeoBoundary {
    public static var zero: GeoBoundary {
        return GeoBoundary(numVerts: 0, verts: (
            GeoCoord.zero,
            GeoCoord.zero,
            GeoCoord.zero,
            GeoCoord.zero,
            GeoCoord.zero,
            GeoCoord.zero,
            GeoCoord.zero,
            GeoCoord.zero,
            GeoCoord.zero,
            GeoCoord.zero
        ))
    }
}

public extension Geofence {
    init(coords: inout [GeoCoord]) {
        self.init(numVerts: Int32(coords.count), verts: &coords)
    }
}

public extension GeoPolygon {
    init(coords: inout [GeoCoord]) {
        let fence = Geofence(coords: &coords)
        var emptyArray: [Geofence] = []
        self.init(geofence: fence, numHoles: 0, holes: &emptyArray)
    }
    
    init(coords: inout [GeoCoord], holes: inout [Geofence]) {
        let edge = Geofence(coords: &coords)
        self.init(geofence: edge, numHoles: Int32(holes.count), holes: &holes)
    }
}
