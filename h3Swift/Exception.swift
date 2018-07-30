//
//  Exception.swift
//  h3Swift
//
//  Created by 十亀眞怜 on 2018/07/30.
//

import Foundation

public struct PentagonEncounteredException: Error {}
public struct BadInputToCompact: Error {}
public struct BadInputToUncompact: Error {}
public struct ResolutionOutOfRange: Error {
    let resolution: Int32
}
