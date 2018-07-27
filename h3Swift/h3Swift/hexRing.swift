//
//  hexRing.swift
//  h3Swift
//
//  Created by 十亀眞怜 on 2018/07/27.
//  Copyright © 2018年 Pocket7878. All rights reserved.
//

import Foundation
import h3

public extension H3Index {
    public func hexRing(k: Int32, out: inout Array<H3Index>) -> Bool {
        return out.withUnsafeMutableBufferPointer({ (outPtr) -> Bool in
            return h3.hexRing(self, k, outPtr.baseAddress) != 0
        })
    }
}
