//
//  h3Set.swift
//  h3Swift
//
//  Created by 十亀眞怜 on 2018/07/27.
//  Copyright © 2018年 Pocket7878. All rights reserved.
//

import Foundation
import h3

public extension Array where Element == H3Index {
    public mutating func hexRanges(length: Int32, k: Int32, out: inout Array<H3Index>) -> Bool {
        return self.withUnsafeMutableBufferPointer({ (h3SetPtr) -> Bool in
            return out.withUnsafeMutableBufferPointer({ (outPtr) -> Bool in
                return h3.hexRanges(h3SetPtr.baseAddress, length, k, outPtr.baseAddress) != 0
            })
        })
    }
    
    public mutating func compact(compactedSet: inout Array<H3Index>, numHexes: Int32) -> Bool {
        return self.withUnsafeMutableBufferPointer { (inputPtr) -> Bool in
            compactedSet.withUnsafeMutableBufferPointer({ (outPtr) -> Bool in
                return h3.compact(inputPtr.baseAddress, outPtr.baseAddress, numHexes) == 0
            })
        }
    }
    
    public func uncompact(numHexes: Int32, h3Set: inout Array<H3Index>, maxHexes: Int32, res: Int32) -> Bool {
        return self.withUnsafeBufferPointer({ (inPtr) -> Bool in
            return h3Set.withUnsafeMutableBufferPointer({ (outPtr) -> Bool in
                return h3.uncompact(inPtr.baseAddress, numHexes, outPtr.baseAddress, maxHexes, res) == 0
            })
        })
    }
    
    public func maxUncompactSize(numHexes: Int32, res: Int32) -> Int32 {
        return self.withUnsafeBufferPointer { (inPtr) -> Int32 in
            return h3.maxUncompactSize(inPtr.baseAddress, numHexes, res)
        }
    }
    
    public mutating func toLinkedGeo(numHexes: Int32) -> LinkedGeoPolygon? {
        var out: UnsafeMutablePointer<LinkedGeoPolygon>?
        self.withUnsafeBufferPointer { (inPtr) -> Void in
            h3SetToLinkedGeo(inPtr.baseAddress, numHexes, out)
        }
        return out?.pointee
    }
}
