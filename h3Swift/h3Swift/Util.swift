//
//  Util.swift
//  h3Swift
//
//  Created by 十亀眞怜 on 2018/07/27.
//  Copyright © 2018年 Pocket7878. All rights reserved.
//

import Foundation
import h3

public func degsToRads(_ double: Double) -> Double {
    return h3.degsToRads(double)
}

public func radsToDegs(_ double: Double) -> Double {
    return h3.radsToDegs(double)
}

public enum DistanceUnit {
    case km
    case m
}

public func hexArea(res: Int32, unit: DistanceUnit) -> Double {
    switch(unit) {
    case .km:
        return h3.hexAreaKm2(res)
    case .m:
        return h3.hexAreaM2(res)
    }
}

public func edgeLength(res: Int32, unit: DistanceUnit) -> Double {
    switch(unit) {
    case .km:
        return h3.edgeLengthKm(res)
    case .m:
        return h3.edgeLengthM(res)
    }
}

public func numHexagons(res: Int32) -> Int64 {
    return h3.numHexagons(res)
}
