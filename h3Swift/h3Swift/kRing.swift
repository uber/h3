//
//  kRing.swift
//  h3Swift
//
//  Created by 十亀眞怜 on 2018/07/27.
//  Copyright © 2018年 Pocket7878. All rights reserved.
//

import Foundation
import h3

public func maxKringSize(k: Int32) -> Int32 {
    return h3.maxKringSize(k)
}

public extension H3Index {
    public func kRingDistances(k: Int32, out: inout H3Index, distance: inout Int32) {
        withUnsafeMutablePointer(to: &out) { (outPtr) -> Void in
            withUnsafeMutablePointer(to: &distance, { (distancePtr) -> Void in
                h3.kRingDistances(self, k, outPtr, distancePtr)
            })
        }
    }
}
