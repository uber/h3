//
//  H3Index.swift
//  h3Swift
//
//  Created by 十亀眞怜 on 2018/07/27.
//  Copyright © 2018年 Pocket7878. All rights reserved.
//

import Foundation
import h3

public extension H3Index {
    public var resolution: Int32 {
        return h3GetResolution(self)
    }
    
    public var baseCell: Int32 {
        return h3GetBaseCell(self)
    }
    
    public static func from(string: inout String) -> H3Index {
        return stringToH3((string as NSString).utf8String)
    }
    
    public var isValid: Bool {
        return h3IsValid(self) != 0
    }
    
    public var isResClassIII: Bool {
        return h3IsResClassIII(self) != 0
    }
    
    public var isPentagon: Bool {
        return h3IsPentagon(self) != 0
    }
    
    public func kRing(k: Int32, out: inout H3Index) -> H3Index {
        withUnsafeMutablePointer(to: &out) { (ptr) -> Void in
            h3.kRing(self, k, ptr)
        }
        return out
    }
    
    public func toParent(parentRes: Int32) -> H3Index {
        return h3ToParent(self, parentRes)
    }
    
    public func children(childRes: Int32, buf: inout Array<H3Index>) -> Array<H3Index> {
        buf.withUnsafeMutableBufferPointer { (bufPtr) -> Void in
            h3.h3ToChildren(self, childRes, bufPtr.baseAddress)
        }
        return buf
    }
    
    public func maxChildrenSize(childRes: Int32) -> Int32 {
        return h3.maxH3ToChildrenSize(self, childRes)
    }
    
    public func isNeighbors(_ destination: H3Index) -> Bool {
        return h3.h3IndexesAreNeighbors(self, destination) == 1
    }
    
    public func unidirectionalEdge(_ destination: H3Index) -> H3Index {
        return h3.getH3UnidirectionalEdge(self, destination)
    }
}
