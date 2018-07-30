//
//  Aliase.swift
//  h3Swift
//
//  Created by 十亀眞怜 on 2018/07/27.
//  Copyright © 2018年 Pocket7878. All rights reserved.
//

import Foundation
import h3

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
