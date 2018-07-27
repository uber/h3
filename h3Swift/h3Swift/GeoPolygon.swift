//
//  GeoPolygon.swift
//  h3Swift
//
//  Created by 十亀眞怜 on 2018/07/27.
//  Copyright © 2018年 Pocket7878. All rights reserved.
//

import Foundation
import h3

public typealias GeoPolygon = h3.GeoPolygon

public extension GeoPolygon {
    public mutating func polyfill(res: Int32, out: inout Array<H3Index>) {
        withUnsafePointer(to: &self) { (inPtr) -> Void in
            out.withUnsafeMutableBufferPointer({ (outPtr) -> Void in
                h3.polyfill(inPtr, res, outPtr.baseAddress)
            })
        }
    }
    
    public mutating func maxPolyfillSize(res: Int32) -> Int32 {
        return withUnsafePointer(to: &self) { (inPtr) -> Int32 in
            return h3.maxPolyfillSize(inPtr, res)
        }
    }
}
