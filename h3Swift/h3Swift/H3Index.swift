//
//  H3Index.swift
//  h3Swift
//
//  Created by 十亀眞怜 on 2018/07/27.
//  Copyright © 2018年 Pocket7878. All rights reserved.
//

import Foundation
import h3

public class H3Index: Equatable, Hashable {
    let index: h3.H3Index
    
    //MARK: Constructor
    public required init(_ idx: UInt64) {
        self.index = idx
    }
    
    public convenience init(string: String) {
        self.init(h3.stringToH3((string as NSString).utf8String))
    }
    
    
    public var resolution: Int32 {
        return h3GetResolution(index)
    }
    
    public var baseCell: Int32 {
        return h3GetBaseCell(index)
    }
    
    public func toString() -> String {
        return String(index, radix: 16)
    }
    
    public var isValid: Bool {
        return h3IsValid(index) != 0
    }
    
    public var isResClassIII: Bool {
        return h3IsResClassIII(index) != 0
    }
    
    public var isPentagon: Bool {
        return h3IsPentagon(index) != 0
    }
    
    public func toParent(parentRes: Int32) -> H3Index {
        return H3Index(h3ToParent(index, parentRes))
    }
    
    public func children(childRes: Int32) -> Array<H3Index> {
        let sz: Int = Int(maxChildrenSize(childRes: childRes))
        var buf: Array<h3.H3Index> = Array.init(repeating: 0, count: sz)
        buf.withUnsafeMutableBufferPointer { (bufPtr) -> Void in
            h3.h3ToChildren(index, childRes, bufPtr.baseAddress)
        }
        return rawArrayToClassArray(filterNonZero(buf))
    }
    
    public func maxChildrenSize(childRes: Int32) -> Int32 {
        return h3.maxH3ToChildrenSize(index, childRes)
    }
    
    public func isNeighbors(_ destination: H3Index) -> Bool {
        return h3.h3IndexesAreNeighbors(index, destination.index) == 1
    }
    
    public func unidirectionalEdge(_ destination: H3Index) -> H3Index {
        return H3Index(h3.getH3UnidirectionalEdge(index, destination.index))
    }
    
    public func geoBoundary() -> Array<GeoCoord> {
        var gb: GeoBoundary = GeoBoundary.zero
        var res: Array<GeoCoord> = Array()
        withUnsafeMutablePointer(to: &gb) { (ptr) -> Void in
            h3.h3ToGeoBoundary(index, ptr)
        }
        let coords: [GeoCoord] = Mirror(reflecting: gb.verts).children.map { $0.value as! GeoCoord }
        return coords[0..<Int(gb.numVerts)].map{$0}
    }
    
    
    //MARK: UnidirectionalEdge
    var isValidUnidirectionalEdge: Bool {
        return h3.h3UnidirectionalEdgeIsValid(index) == 1
    }
    
    var unidirectionalEdgeOrigin: H3Index {
        return H3Index(h3.getOriginH3IndexFromUnidirectionalEdge(index))
    }
    
    var unidirectionalEdgeDestination: H3Index {
        return H3Index(h3.getDestinationH3IndexFromUnidirectionalEdge(index))
    }
    
    public func unidirectionalEdgeH3Indexes() -> Array<H3Index> {
        var buf: Array<h3.H3Index> = Array.init(repeating: 0, count: 2)
        buf.withUnsafeMutableBufferPointer { (ptr) -> Void in
            h3.getH3IndexesFromUnidirectionalEdge(index, ptr.baseAddress)
        }
        return rawArrayToClassArray(filterNonZero(buf))
    }
    
    public func hexagonUnidirectionalEdges() -> Array<H3Index> {
        var buf: Array<h3.H3Index> = Array.init(repeating: 0, count: 6)
        buf.withUnsafeMutableBufferPointer { (ptr) -> Void in
            h3.getH3UnidirectionalEdgesFromHexagon(index, ptr.baseAddress)
        }
        return rawArrayToClassArray(filterNonZero(buf))
    }
    
    public func unidirectionalEdgeBoundary(_ gb: inout GeoBoundary) {
        withUnsafeMutablePointer(to: &gb) { (gbPtr) -> Void in
            h3.getH3UnidirectionalEdgeBoundary(index, gbPtr)
        }
    }
    
    //MARK: hexRange
    public func hexRange(k: Int32) throws -> Array<Array<H3Index>> {
        let sz:Int = Int(maxKringSize(k))
        var buf: Array<h3.H3Index> = Array.init(repeating: 0, count: sz)
        let res: Int32 = buf.withUnsafeMutableBufferPointer({ (outPtr) -> Int32 in
            return h3.hexRange(index, k, outPtr.baseAddress)
        })
        guard res != 0 else {
            throw PentagonEncounteredException()
        }
        
        var ret: Array<Array<H3Index>> = Array()
        var ring: Array<H3Index> = Array()
        var currentK: Int = 0
        var nextRing: Int = 0
        for i in 0..<sz {
            if i == nextRing {
                ring = Array()
                ret.append(ring)
                if currentK == 0 {
                    nextRing = 1
                } else {
                    nextRing += (6 * currentK)
                }
                currentK += 1
            }
            let h = buf[i]
            ring.append(H3Index(h))
        }
        
        return ret
    }
    
    //MARK: hexRing
    public func hexRing(k: Int32) throws -> Array<H3Index> {
        let sz: Int
        if k == 0 {
            sz = 1
        } else {
            sz = 6 * Int(k)
        }
        var buf: Array<h3.H3Index> = Array(repeating: 0, count: sz)
        let res = buf.withUnsafeMutableBufferPointer({ (outPtr) -> Int32 in
            return h3.hexRing(index, k, outPtr.baseAddress)
        })
        guard res != 0 else {
            throw PentagonEncounteredException()
        }
        return rawArrayToClassArray(filterNonZero(buf))
    }
    
    //MARK: kRing
    public func kRing(k: Int32) -> Array<H3Index> {
        let sz: Int = Int(maxKringSize(k))
        var buf: Array<h3.H3Index> = Array(repeating: 0, count: sz)
        buf.withUnsafeMutableBufferPointer { (ptr) -> Void in
            h3.kRing(index, k, ptr.baseAddress)
        }
        return rawArrayToClassArray(filterNonZero(buf))
    }
    
    public func kRingDistances(k: Int32) -> Array<Array<H3Index>> {
        let sz: Int = Int(maxKringSize(k))
        var out: Array<h3.H3Index> = Array(repeating: 0, count: sz)
        var distances: Array<Int32> = Array(repeating: 0, count: sz)
        out.withUnsafeMutableBufferPointer { (outPtr) -> Void in
            distances.withUnsafeMutableBufferPointer({ (distPtr) -> Void in
                h3.kRingDistances(index, k, outPtr.baseAddress, distPtr.baseAddress)
            })
        }
        
        var res: Array<Array<H3Index>> = Array()
        for _ in 0...k {
            res.append(Array())
        }
        for i in 0..<sz {
            let nextH3 = out[i]
            if nextH3 != 0 {
                res[Int(distances[i])].append(H3Index(nextH3))
            }
        }
        
        return res
    }
    
    //MARK: private
    private func rawArrayToClassArray(_ list: Array<h3.H3Index>) -> Array<H3Index> {
        return list.map { (raw) -> H3Index in
            H3Index(raw)
        }
    }
    
    private func filterNonZero(_ list: Array<h3.H3Index>) -> Array<h3.H3Index> {
        return list.filter { (raw) -> Bool in
            return raw != 0
        }
    }
    
    //MARK: Equatable
    public static func ==(lhs: H3Index, rhs: H3Index) -> Bool {
        return lhs.index == rhs.index
    }
    
    //MARK: Hashable
    public var hashValue: Int {
        return self.index.hashValue
    }
}
