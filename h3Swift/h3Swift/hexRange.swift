//
//  hexRange.swift
//  h3Swift
//
//  Created by 十亀眞怜 on 2018/07/27.
//  Copyright © 2018年 Pocket7878. All rights reserved.
//

import Foundation
import h3

public extension H3Index {
    public func hexRange(k: Int32, out: inout Array<H3Index>) -> Bool {
        return out.withUnsafeMutableBufferPointer({ (outPtr) -> Bool in
            return h3.hexRange(self, k, outPtr.baseAddress) != 0
        })
    }
    
    public func hexRangeDistances(k: Int32, out: inout Array<H3Index>, distances: inout Array<Int32>) -> Bool {
        return out.withUnsafeMutableBufferPointer({ (outPtr) -> Bool in
            return distances.withUnsafeMutableBufferPointer({ (distancesPtr) -> Bool in
                return h3.hexRangeDistances(self, k, outPtr.baseAddress, distancesPtr.baseAddress) != 0
            })
        })
    }
}
