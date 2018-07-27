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

public func hexAreaKm2(res: Int32) -> Double {
    return h3.hexAreaKm2(res)
}

public func hexAreaM2(res: Int32) -> Double {
    return h3.hexAreaM2(res)
}

public func edgeLengthKm(res: Int32) -> Double {
    return h3.edgeLengthKm(res)
}

public func edgeLengthM(res: Int32) -> Double {
    return h3.edgeLengthM(res)
}

public func numHexagons(res: Int32) -> Int64 {
    return h3.numHexagons(res)
}
