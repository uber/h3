//
//  H3UnidirectionalEdge.swift
//  h3Swift
//
//  Created by 十亀眞怜 on 2018/07/27.
//  Copyright © 2018年 Pocket7878. All rights reserved.
//

import Foundation
import h3

public extension H3Index {
    var isValidUnidirectionalEdge: Bool {
        return h3.h3UnidirectionalEdgeIsValid(self) == 1
    }
    
    var unidirectionalEdgeOrigin: H3Index {
        return h3.getOriginH3IndexFromUnidirectionalEdge(self)
    }
    
    var unidirectionalEdgeDestination: H3Index {
        return h3.getDestinationH3IndexFromUnidirectionalEdge(self)
    }
    
    public func unidirectionalEdgeH3Indexes(_ buf: inout Array<H3Index>) {
        buf.withUnsafeMutableBufferPointer { (ptr) -> Void in
            h3.getH3IndexesFromUnidirectionalEdge(self, ptr.baseAddress)
        }
    }
    
    public func hexagonUnidirectionalEdges(_ buf: inout Array<H3Index>) {
        buf.withUnsafeMutableBufferPointer { (ptr) -> Void in
            h3.getH3UnidirectionalEdgesFromHexagon(self, ptr.baseAddress)
        }
    }
    
    public func unidirectionalEdgeBoundary(_ gb: inout GeoBoundary) {
        withUnsafeMutablePointer(to: &gb) { (gbPtr) -> Void in
            h3.getH3UnidirectionalEdgeBoundary(self, gbPtr)
        }
    }
}
