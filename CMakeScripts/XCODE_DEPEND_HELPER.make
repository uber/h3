# DO NOT EDIT
# This makefile makes sure all linkable targets are
# up-to-date with anything they link to
default:
	echo "Do not invoke directly"

# Rules to remove targets that are older than anything to which they
# link.  This forces Xcode to relink the targets from scratch.  It
# does not seem to check these dependencies itself.
PostBuild.benchmarkH3Api.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/benchmarkH3Api
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/benchmarkH3Api:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/benchmarkH3Api
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/benchmarkH3Api.build/Objects-normal/armv7/benchmarkH3Api
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/benchmarkH3Api.build/Objects-normal/armv7s/benchmarkH3Api
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/benchmarkH3Api.build/Objects-normal/arm64/benchmarkH3Api


PostBuild.benchmarkPolyfill.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/benchmarkPolyfill
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/benchmarkPolyfill:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/benchmarkPolyfill
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/benchmarkPolyfill.build/Objects-normal/armv7/benchmarkPolyfill
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/benchmarkPolyfill.build/Objects-normal/armv7s/benchmarkPolyfill
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/benchmarkPolyfill.build/Objects-normal/arm64/benchmarkPolyfill


PostBuild.generateBaseCellNeighbors.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/generateBaseCellNeighbors
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/generateBaseCellNeighbors:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/generateBaseCellNeighbors
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/generateBaseCellNeighbors.build/Objects-normal/armv7/generateBaseCellNeighbors
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/generateBaseCellNeighbors.build/Objects-normal/armv7s/generateBaseCellNeighbors
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/generateBaseCellNeighbors.build/Objects-normal/arm64/generateBaseCellNeighbors


PostBuild.generateFaceCenterPoint.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/generateFaceCenterPoint
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/generateFaceCenterPoint:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/generateFaceCenterPoint
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/generateFaceCenterPoint.build/Objects-normal/armv7/generateFaceCenterPoint
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/generateFaceCenterPoint.build/Objects-normal/armv7s/generateFaceCenterPoint
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/generateFaceCenterPoint.build/Objects-normal/arm64/generateFaceCenterPoint


PostBuild.generateNumHexagons.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/generateNumHexagons
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/generateNumHexagons:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/generateNumHexagons
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/generateNumHexagons.build/Objects-normal/armv7/generateNumHexagons
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/generateNumHexagons.build/Objects-normal/armv7s/generateNumHexagons
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/generateNumHexagons.build/Objects-normal/arm64/generateNumHexagons


PostBuild.geoToH3.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/geoToH3
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/geoToH3:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/geoToH3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/geoToH3.build/Objects-normal/armv7/geoToH3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/geoToH3.build/Objects-normal/armv7s/geoToH3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/geoToH3.build/Objects-normal/arm64/geoToH3


PostBuild.h3.Debug:
/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3:\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/h3.build/Objects-normal/armv7/h3.framework/h3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/h3.build/Objects-normal/armv7s/h3.framework/h3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/h3.build/Objects-normal/arm64/h3.framework/h3


PostBuild.h3ToComponents.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/h3ToComponents
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/h3ToComponents:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/h3ToComponents
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/h3ToComponents.build/Objects-normal/armv7/h3ToComponents
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/h3ToComponents.build/Objects-normal/armv7s/h3ToComponents
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/h3ToComponents.build/Objects-normal/arm64/h3ToComponents


PostBuild.h3ToGeo.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/h3ToGeo
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/h3ToGeo:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/h3ToGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/h3ToGeo.build/Objects-normal/armv7/h3ToGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/h3ToGeo.build/Objects-normal/armv7s/h3ToGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/h3ToGeo.build/Objects-normal/arm64/h3ToGeo


PostBuild.h3ToGeoBoundary.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/h3ToGeoBoundary
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/h3ToGeoBoundary:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/h3ToGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/h3ToGeoBoundary.build/Objects-normal/armv7/h3ToGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/h3ToGeoBoundary.build/Objects-normal/armv7s/h3ToGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/h3ToGeoBoundary.build/Objects-normal/arm64/h3ToGeoBoundary


PostBuild.h3ToGeoBoundaryHier.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/h3ToGeoBoundaryHier
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/h3ToGeoBoundaryHier:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/h3ToGeoBoundaryHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/h3ToGeoBoundaryHier.build/Objects-normal/armv7/h3ToGeoBoundaryHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/h3ToGeoBoundaryHier.build/Objects-normal/armv7s/h3ToGeoBoundaryHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/h3ToGeoBoundaryHier.build/Objects-normal/arm64/h3ToGeoBoundaryHier


PostBuild.h3ToGeoHier.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/h3ToGeoHier
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/h3ToGeoHier:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/h3ToGeoHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/h3ToGeoHier.build/Objects-normal/armv7/h3ToGeoHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/h3ToGeoHier.build/Objects-normal/armv7s/h3ToGeoHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/h3ToGeoHier.build/Objects-normal/arm64/h3ToGeoHier


PostBuild.h3ToHier.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/h3ToHier
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/h3ToHier:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/h3ToHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/h3ToHier.build/Objects-normal/armv7/h3ToHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/h3ToHier.build/Objects-normal/armv7s/h3ToHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/h3ToHier.build/Objects-normal/arm64/h3ToHier


PostBuild.h3ToIjk.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/h3ToIjk
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/h3ToIjk:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/h3ToIjk
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/h3ToIjk.build/Objects-normal/armv7/h3ToIjk
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/h3ToIjk.build/Objects-normal/armv7s/h3ToIjk
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/h3ToIjk.build/Objects-normal/arm64/h3ToIjk


PostBuild.hexRange.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/hexRange
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/hexRange:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/hexRange
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/hexRange.build/Objects-normal/armv7/hexRange
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/hexRange.build/Objects-normal/armv7s/hexRange
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/hexRange.build/Objects-normal/arm64/hexRange


PostBuild.kRing.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/kRing
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/kRing:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/kRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/kRing.build/Objects-normal/armv7/kRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/kRing.build/Objects-normal/armv7s/kRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/kRing.build/Objects-normal/arm64/kRing


PostBuild.mkRandGeo.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/mkRandGeo
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/mkRandGeo:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/mkRandGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/mkRandGeo.build/Objects-normal/armv7/mkRandGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/mkRandGeo.build/Objects-normal/armv7s/mkRandGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/mkRandGeo.build/Objects-normal/arm64/mkRandGeo


PostBuild.mkRandGeoBoundary.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/mkRandGeoBoundary
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/mkRandGeoBoundary:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/mkRandGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/mkRandGeoBoundary.build/Objects-normal/armv7/mkRandGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/mkRandGeoBoundary.build/Objects-normal/armv7s/mkRandGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/mkRandGeoBoundary.build/Objects-normal/arm64/mkRandGeoBoundary


PostBuild.testBBox.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testBBox
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testBBox:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testBBox
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testBBox.build/Objects-normal/armv7/testBBox
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testBBox.build/Objects-normal/armv7s/testBBox
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testBBox.build/Objects-normal/arm64/testBBox


PostBuild.testCompact.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testCompact
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testCompact:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testCompact
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testCompact.build/Objects-normal/armv7/testCompact
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testCompact.build/Objects-normal/armv7s/testCompact
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testCompact.build/Objects-normal/arm64/testCompact


PostBuild.testGeoCoord.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testGeoCoord
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testGeoCoord:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testGeoCoord
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testGeoCoord.build/Objects-normal/armv7/testGeoCoord
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testGeoCoord.build/Objects-normal/armv7s/testGeoCoord
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testGeoCoord.build/Objects-normal/arm64/testGeoCoord


PostBuild.testGeoToH3.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testGeoToH3
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testGeoToH3:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testGeoToH3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testGeoToH3.build/Objects-normal/armv7/testGeoToH3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testGeoToH3.build/Objects-normal/armv7s/testGeoToH3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testGeoToH3.build/Objects-normal/arm64/testGeoToH3


PostBuild.testH3Api.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3Api
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3Api:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3Api
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3Api.build/Objects-normal/armv7/testH3Api
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3Api.build/Objects-normal/armv7s/testH3Api
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3Api.build/Objects-normal/arm64/testH3Api


PostBuild.testH3Index.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3Index
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3Index:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3Index
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3Index.build/Objects-normal/armv7/testH3Index
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3Index.build/Objects-normal/armv7s/testH3Index
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3Index.build/Objects-normal/arm64/testH3Index


PostBuild.testH3NeighborRotations.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3NeighborRotations
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3NeighborRotations:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3NeighborRotations
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3NeighborRotations.build/Objects-normal/armv7/testH3NeighborRotations
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3NeighborRotations.build/Objects-normal/armv7s/testH3NeighborRotations
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3NeighborRotations.build/Objects-normal/arm64/testH3NeighborRotations


PostBuild.testH3SetToLinkedGeo.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3SetToLinkedGeo
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3SetToLinkedGeo:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3SetToLinkedGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3SetToLinkedGeo.build/Objects-normal/armv7/testH3SetToLinkedGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3SetToLinkedGeo.build/Objects-normal/armv7s/testH3SetToLinkedGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3SetToLinkedGeo.build/Objects-normal/arm64/testH3SetToLinkedGeo


PostBuild.testH3SetToVertexGraph.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3SetToVertexGraph
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3SetToVertexGraph:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3SetToVertexGraph
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3SetToVertexGraph.build/Objects-normal/armv7/testH3SetToVertexGraph
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3SetToVertexGraph.build/Objects-normal/armv7s/testH3SetToVertexGraph
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3SetToVertexGraph.build/Objects-normal/arm64/testH3SetToVertexGraph


PostBuild.testH3ToChildren.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3ToChildren
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3ToChildren:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3ToChildren
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3ToChildren.build/Objects-normal/armv7/testH3ToChildren
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3ToChildren.build/Objects-normal/armv7s/testH3ToChildren
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3ToChildren.build/Objects-normal/arm64/testH3ToChildren


PostBuild.testH3ToGeo.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3ToGeo
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3ToGeo:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3ToGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3ToGeo.build/Objects-normal/armv7/testH3ToGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3ToGeo.build/Objects-normal/armv7s/testH3ToGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3ToGeo.build/Objects-normal/arm64/testH3ToGeo


PostBuild.testH3ToGeoBoundary.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3ToGeoBoundary
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3ToGeoBoundary:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3ToGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3ToGeoBoundary.build/Objects-normal/armv7/testH3ToGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3ToGeoBoundary.build/Objects-normal/armv7s/testH3ToGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3ToGeoBoundary.build/Objects-normal/arm64/testH3ToGeoBoundary


PostBuild.testH3ToIjk.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3ToIjk
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3ToIjk:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3ToIjk
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3ToIjk.build/Objects-normal/armv7/testH3ToIjk
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3ToIjk.build/Objects-normal/armv7s/testH3ToIjk
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3ToIjk.build/Objects-normal/arm64/testH3ToIjk


PostBuild.testH3ToParent.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3ToParent
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3ToParent:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3ToParent
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3ToParent.build/Objects-normal/armv7/testH3ToParent
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3ToParent.build/Objects-normal/armv7s/testH3ToParent
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3ToParent.build/Objects-normal/arm64/testH3ToParent


PostBuild.testH3UniEdge.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3UniEdge
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3UniEdge:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testH3UniEdge
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3UniEdge.build/Objects-normal/armv7/testH3UniEdge
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3UniEdge.build/Objects-normal/armv7s/testH3UniEdge
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testH3UniEdge.build/Objects-normal/arm64/testH3UniEdge


PostBuild.testHexRanges.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testHexRanges
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testHexRanges:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testHexRanges
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testHexRanges.build/Objects-normal/armv7/testHexRanges
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testHexRanges.build/Objects-normal/armv7s/testHexRanges
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testHexRanges.build/Objects-normal/arm64/testHexRanges


PostBuild.testHexRing.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testHexRing
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testHexRing:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testHexRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testHexRing.build/Objects-normal/armv7/testHexRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testHexRing.build/Objects-normal/armv7s/testHexRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testHexRing.build/Objects-normal/arm64/testHexRing


PostBuild.testKRing.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testKRing
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testKRing:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testKRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testKRing.build/Objects-normal/armv7/testKRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testKRing.build/Objects-normal/armv7s/testKRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testKRing.build/Objects-normal/arm64/testKRing


PostBuild.testLinkedGeo.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testLinkedGeo
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testLinkedGeo:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testLinkedGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testLinkedGeo.build/Objects-normal/armv7/testLinkedGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testLinkedGeo.build/Objects-normal/armv7s/testLinkedGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testLinkedGeo.build/Objects-normal/arm64/testLinkedGeo


PostBuild.testMaxH3ToChildrenSize.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testMaxH3ToChildrenSize
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testMaxH3ToChildrenSize:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testMaxH3ToChildrenSize
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testMaxH3ToChildrenSize.build/Objects-normal/armv7/testMaxH3ToChildrenSize
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testMaxH3ToChildrenSize.build/Objects-normal/armv7s/testMaxH3ToChildrenSize
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testMaxH3ToChildrenSize.build/Objects-normal/arm64/testMaxH3ToChildrenSize


PostBuild.testPolyfill.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testPolyfill
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testPolyfill:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testPolyfill
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testPolyfill.build/Objects-normal/armv7/testPolyfill
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testPolyfill.build/Objects-normal/armv7s/testPolyfill
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testPolyfill.build/Objects-normal/arm64/testPolyfill


PostBuild.testVec2d.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testVec2d
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testVec2d:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testVec2d
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testVec2d.build/Objects-normal/armv7/testVec2d
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testVec2d.build/Objects-normal/armv7s/testVec2d
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testVec2d.build/Objects-normal/arm64/testVec2d


PostBuild.testVec3d.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testVec3d
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testVec3d:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testVec3d
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testVec3d.build/Objects-normal/armv7/testVec3d
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testVec3d.build/Objects-normal/armv7s/testVec3d
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testVec3d.build/Objects-normal/arm64/testVec3d


PostBuild.testVertexGraph.Debug:
PostBuild.h3.Debug: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testVertexGraph
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testVertexGraph:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Debug/testVertexGraph
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testVertexGraph.build/Objects-normal/armv7/testVertexGraph
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testVertexGraph.build/Objects-normal/armv7s/testVertexGraph
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Debug/testVertexGraph.build/Objects-normal/arm64/testVertexGraph


PostBuild.benchmarkH3Api.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/benchmarkH3Api
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/benchmarkH3Api:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/benchmarkH3Api
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/benchmarkH3Api.build/Objects-normal/armv7/benchmarkH3Api
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/benchmarkH3Api.build/Objects-normal/armv7s/benchmarkH3Api
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/benchmarkH3Api.build/Objects-normal/arm64/benchmarkH3Api


PostBuild.benchmarkPolyfill.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/benchmarkPolyfill
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/benchmarkPolyfill:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/benchmarkPolyfill
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/benchmarkPolyfill.build/Objects-normal/armv7/benchmarkPolyfill
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/benchmarkPolyfill.build/Objects-normal/armv7s/benchmarkPolyfill
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/benchmarkPolyfill.build/Objects-normal/arm64/benchmarkPolyfill


PostBuild.generateBaseCellNeighbors.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/generateBaseCellNeighbors
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/generateBaseCellNeighbors:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/generateBaseCellNeighbors
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/generateBaseCellNeighbors.build/Objects-normal/armv7/generateBaseCellNeighbors
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/generateBaseCellNeighbors.build/Objects-normal/armv7s/generateBaseCellNeighbors
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/generateBaseCellNeighbors.build/Objects-normal/arm64/generateBaseCellNeighbors


PostBuild.generateFaceCenterPoint.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/generateFaceCenterPoint
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/generateFaceCenterPoint:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/generateFaceCenterPoint
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/generateFaceCenterPoint.build/Objects-normal/armv7/generateFaceCenterPoint
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/generateFaceCenterPoint.build/Objects-normal/armv7s/generateFaceCenterPoint
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/generateFaceCenterPoint.build/Objects-normal/arm64/generateFaceCenterPoint


PostBuild.generateNumHexagons.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/generateNumHexagons
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/generateNumHexagons:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/generateNumHexagons
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/generateNumHexagons.build/Objects-normal/armv7/generateNumHexagons
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/generateNumHexagons.build/Objects-normal/armv7s/generateNumHexagons
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/generateNumHexagons.build/Objects-normal/arm64/generateNumHexagons


PostBuild.geoToH3.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/geoToH3
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/geoToH3:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/geoToH3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/geoToH3.build/Objects-normal/armv7/geoToH3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/geoToH3.build/Objects-normal/armv7s/geoToH3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/geoToH3.build/Objects-normal/arm64/geoToH3


PostBuild.h3.Release:
/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3:\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/h3.build/Objects-normal/armv7/h3.framework/h3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/h3.build/Objects-normal/armv7s/h3.framework/h3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/h3.build/Objects-normal/arm64/h3.framework/h3


PostBuild.h3ToComponents.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/h3ToComponents
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/h3ToComponents:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/h3ToComponents
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/h3ToComponents.build/Objects-normal/armv7/h3ToComponents
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/h3ToComponents.build/Objects-normal/armv7s/h3ToComponents
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/h3ToComponents.build/Objects-normal/arm64/h3ToComponents


PostBuild.h3ToGeo.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/h3ToGeo
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/h3ToGeo:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/h3ToGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/h3ToGeo.build/Objects-normal/armv7/h3ToGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/h3ToGeo.build/Objects-normal/armv7s/h3ToGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/h3ToGeo.build/Objects-normal/arm64/h3ToGeo


PostBuild.h3ToGeoBoundary.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/h3ToGeoBoundary
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/h3ToGeoBoundary:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/h3ToGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/h3ToGeoBoundary.build/Objects-normal/armv7/h3ToGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/h3ToGeoBoundary.build/Objects-normal/armv7s/h3ToGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/h3ToGeoBoundary.build/Objects-normal/arm64/h3ToGeoBoundary


PostBuild.h3ToGeoBoundaryHier.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/h3ToGeoBoundaryHier
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/h3ToGeoBoundaryHier:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/h3ToGeoBoundaryHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/h3ToGeoBoundaryHier.build/Objects-normal/armv7/h3ToGeoBoundaryHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/h3ToGeoBoundaryHier.build/Objects-normal/armv7s/h3ToGeoBoundaryHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/h3ToGeoBoundaryHier.build/Objects-normal/arm64/h3ToGeoBoundaryHier


PostBuild.h3ToGeoHier.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/h3ToGeoHier
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/h3ToGeoHier:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/h3ToGeoHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/h3ToGeoHier.build/Objects-normal/armv7/h3ToGeoHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/h3ToGeoHier.build/Objects-normal/armv7s/h3ToGeoHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/h3ToGeoHier.build/Objects-normal/arm64/h3ToGeoHier


PostBuild.h3ToHier.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/h3ToHier
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/h3ToHier:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/h3ToHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/h3ToHier.build/Objects-normal/armv7/h3ToHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/h3ToHier.build/Objects-normal/armv7s/h3ToHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/h3ToHier.build/Objects-normal/arm64/h3ToHier


PostBuild.h3ToIjk.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/h3ToIjk
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/h3ToIjk:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/h3ToIjk
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/h3ToIjk.build/Objects-normal/armv7/h3ToIjk
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/h3ToIjk.build/Objects-normal/armv7s/h3ToIjk
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/h3ToIjk.build/Objects-normal/arm64/h3ToIjk


PostBuild.hexRange.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/hexRange
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/hexRange:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/hexRange
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/hexRange.build/Objects-normal/armv7/hexRange
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/hexRange.build/Objects-normal/armv7s/hexRange
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/hexRange.build/Objects-normal/arm64/hexRange


PostBuild.kRing.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/kRing
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/kRing:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/kRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/kRing.build/Objects-normal/armv7/kRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/kRing.build/Objects-normal/armv7s/kRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/kRing.build/Objects-normal/arm64/kRing


PostBuild.mkRandGeo.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/mkRandGeo
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/mkRandGeo:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/mkRandGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/mkRandGeo.build/Objects-normal/armv7/mkRandGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/mkRandGeo.build/Objects-normal/armv7s/mkRandGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/mkRandGeo.build/Objects-normal/arm64/mkRandGeo


PostBuild.mkRandGeoBoundary.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/mkRandGeoBoundary
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/mkRandGeoBoundary:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/mkRandGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/mkRandGeoBoundary.build/Objects-normal/armv7/mkRandGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/mkRandGeoBoundary.build/Objects-normal/armv7s/mkRandGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/mkRandGeoBoundary.build/Objects-normal/arm64/mkRandGeoBoundary


PostBuild.testBBox.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testBBox
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testBBox:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testBBox
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testBBox.build/Objects-normal/armv7/testBBox
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testBBox.build/Objects-normal/armv7s/testBBox
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testBBox.build/Objects-normal/arm64/testBBox


PostBuild.testCompact.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testCompact
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testCompact:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testCompact
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testCompact.build/Objects-normal/armv7/testCompact
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testCompact.build/Objects-normal/armv7s/testCompact
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testCompact.build/Objects-normal/arm64/testCompact


PostBuild.testGeoCoord.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testGeoCoord
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testGeoCoord:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testGeoCoord
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testGeoCoord.build/Objects-normal/armv7/testGeoCoord
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testGeoCoord.build/Objects-normal/armv7s/testGeoCoord
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testGeoCoord.build/Objects-normal/arm64/testGeoCoord


PostBuild.testGeoToH3.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testGeoToH3
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testGeoToH3:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testGeoToH3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testGeoToH3.build/Objects-normal/armv7/testGeoToH3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testGeoToH3.build/Objects-normal/armv7s/testGeoToH3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testGeoToH3.build/Objects-normal/arm64/testGeoToH3


PostBuild.testH3Api.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3Api
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3Api:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3Api
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3Api.build/Objects-normal/armv7/testH3Api
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3Api.build/Objects-normal/armv7s/testH3Api
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3Api.build/Objects-normal/arm64/testH3Api


PostBuild.testH3Index.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3Index
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3Index:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3Index
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3Index.build/Objects-normal/armv7/testH3Index
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3Index.build/Objects-normal/armv7s/testH3Index
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3Index.build/Objects-normal/arm64/testH3Index


PostBuild.testH3NeighborRotations.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3NeighborRotations
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3NeighborRotations:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3NeighborRotations
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3NeighborRotations.build/Objects-normal/armv7/testH3NeighborRotations
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3NeighborRotations.build/Objects-normal/armv7s/testH3NeighborRotations
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3NeighborRotations.build/Objects-normal/arm64/testH3NeighborRotations


PostBuild.testH3SetToLinkedGeo.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3SetToLinkedGeo
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3SetToLinkedGeo:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3SetToLinkedGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3SetToLinkedGeo.build/Objects-normal/armv7/testH3SetToLinkedGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3SetToLinkedGeo.build/Objects-normal/armv7s/testH3SetToLinkedGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3SetToLinkedGeo.build/Objects-normal/arm64/testH3SetToLinkedGeo


PostBuild.testH3SetToVertexGraph.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3SetToVertexGraph
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3SetToVertexGraph:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3SetToVertexGraph
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3SetToVertexGraph.build/Objects-normal/armv7/testH3SetToVertexGraph
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3SetToVertexGraph.build/Objects-normal/armv7s/testH3SetToVertexGraph
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3SetToVertexGraph.build/Objects-normal/arm64/testH3SetToVertexGraph


PostBuild.testH3ToChildren.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3ToChildren
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3ToChildren:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3ToChildren
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3ToChildren.build/Objects-normal/armv7/testH3ToChildren
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3ToChildren.build/Objects-normal/armv7s/testH3ToChildren
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3ToChildren.build/Objects-normal/arm64/testH3ToChildren


PostBuild.testH3ToGeo.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3ToGeo
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3ToGeo:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3ToGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3ToGeo.build/Objects-normal/armv7/testH3ToGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3ToGeo.build/Objects-normal/armv7s/testH3ToGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3ToGeo.build/Objects-normal/arm64/testH3ToGeo


PostBuild.testH3ToGeoBoundary.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3ToGeoBoundary
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3ToGeoBoundary:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3ToGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3ToGeoBoundary.build/Objects-normal/armv7/testH3ToGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3ToGeoBoundary.build/Objects-normal/armv7s/testH3ToGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3ToGeoBoundary.build/Objects-normal/arm64/testH3ToGeoBoundary


PostBuild.testH3ToIjk.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3ToIjk
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3ToIjk:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3ToIjk
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3ToIjk.build/Objects-normal/armv7/testH3ToIjk
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3ToIjk.build/Objects-normal/armv7s/testH3ToIjk
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3ToIjk.build/Objects-normal/arm64/testH3ToIjk


PostBuild.testH3ToParent.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3ToParent
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3ToParent:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3ToParent
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3ToParent.build/Objects-normal/armv7/testH3ToParent
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3ToParent.build/Objects-normal/armv7s/testH3ToParent
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3ToParent.build/Objects-normal/arm64/testH3ToParent


PostBuild.testH3UniEdge.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3UniEdge
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3UniEdge:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testH3UniEdge
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3UniEdge.build/Objects-normal/armv7/testH3UniEdge
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3UniEdge.build/Objects-normal/armv7s/testH3UniEdge
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testH3UniEdge.build/Objects-normal/arm64/testH3UniEdge


PostBuild.testHexRanges.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testHexRanges
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testHexRanges:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testHexRanges
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testHexRanges.build/Objects-normal/armv7/testHexRanges
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testHexRanges.build/Objects-normal/armv7s/testHexRanges
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testHexRanges.build/Objects-normal/arm64/testHexRanges


PostBuild.testHexRing.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testHexRing
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testHexRing:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testHexRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testHexRing.build/Objects-normal/armv7/testHexRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testHexRing.build/Objects-normal/armv7s/testHexRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testHexRing.build/Objects-normal/arm64/testHexRing


PostBuild.testKRing.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testKRing
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testKRing:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testKRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testKRing.build/Objects-normal/armv7/testKRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testKRing.build/Objects-normal/armv7s/testKRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testKRing.build/Objects-normal/arm64/testKRing


PostBuild.testLinkedGeo.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testLinkedGeo
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testLinkedGeo:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testLinkedGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testLinkedGeo.build/Objects-normal/armv7/testLinkedGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testLinkedGeo.build/Objects-normal/armv7s/testLinkedGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testLinkedGeo.build/Objects-normal/arm64/testLinkedGeo


PostBuild.testMaxH3ToChildrenSize.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testMaxH3ToChildrenSize
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testMaxH3ToChildrenSize:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testMaxH3ToChildrenSize
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testMaxH3ToChildrenSize.build/Objects-normal/armv7/testMaxH3ToChildrenSize
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testMaxH3ToChildrenSize.build/Objects-normal/armv7s/testMaxH3ToChildrenSize
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testMaxH3ToChildrenSize.build/Objects-normal/arm64/testMaxH3ToChildrenSize


PostBuild.testPolyfill.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testPolyfill
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testPolyfill:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testPolyfill
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testPolyfill.build/Objects-normal/armv7/testPolyfill
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testPolyfill.build/Objects-normal/armv7s/testPolyfill
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testPolyfill.build/Objects-normal/arm64/testPolyfill


PostBuild.testVec2d.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testVec2d
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testVec2d:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testVec2d
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testVec2d.build/Objects-normal/armv7/testVec2d
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testVec2d.build/Objects-normal/armv7s/testVec2d
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testVec2d.build/Objects-normal/arm64/testVec2d


PostBuild.testVec3d.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testVec3d
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testVec3d:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testVec3d
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testVec3d.build/Objects-normal/armv7/testVec3d
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testVec3d.build/Objects-normal/armv7s/testVec3d
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testVec3d.build/Objects-normal/arm64/testVec3d


PostBuild.testVertexGraph.Release:
PostBuild.h3.Release: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testVertexGraph
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testVertexGraph:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/Release/testVertexGraph
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testVertexGraph.build/Objects-normal/armv7/testVertexGraph
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testVertexGraph.build/Objects-normal/armv7s/testVertexGraph
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/Release/testVertexGraph.build/Objects-normal/arm64/testVertexGraph


PostBuild.benchmarkH3Api.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/benchmarkH3Api
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/benchmarkH3Api:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/benchmarkH3Api
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/benchmarkH3Api.build/Objects-normal/armv7/benchmarkH3Api
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/benchmarkH3Api.build/Objects-normal/armv7s/benchmarkH3Api
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/benchmarkH3Api.build/Objects-normal/arm64/benchmarkH3Api


PostBuild.benchmarkPolyfill.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/benchmarkPolyfill
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/benchmarkPolyfill:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/benchmarkPolyfill
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/benchmarkPolyfill.build/Objects-normal/armv7/benchmarkPolyfill
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/benchmarkPolyfill.build/Objects-normal/armv7s/benchmarkPolyfill
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/benchmarkPolyfill.build/Objects-normal/arm64/benchmarkPolyfill


PostBuild.generateBaseCellNeighbors.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/generateBaseCellNeighbors
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/generateBaseCellNeighbors:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/generateBaseCellNeighbors
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/generateBaseCellNeighbors.build/Objects-normal/armv7/generateBaseCellNeighbors
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/generateBaseCellNeighbors.build/Objects-normal/armv7s/generateBaseCellNeighbors
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/generateBaseCellNeighbors.build/Objects-normal/arm64/generateBaseCellNeighbors


PostBuild.generateFaceCenterPoint.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/generateFaceCenterPoint
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/generateFaceCenterPoint:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/generateFaceCenterPoint
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/generateFaceCenterPoint.build/Objects-normal/armv7/generateFaceCenterPoint
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/generateFaceCenterPoint.build/Objects-normal/armv7s/generateFaceCenterPoint
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/generateFaceCenterPoint.build/Objects-normal/arm64/generateFaceCenterPoint


PostBuild.generateNumHexagons.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/generateNumHexagons
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/generateNumHexagons:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/generateNumHexagons
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/generateNumHexagons.build/Objects-normal/armv7/generateNumHexagons
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/generateNumHexagons.build/Objects-normal/armv7s/generateNumHexagons
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/generateNumHexagons.build/Objects-normal/arm64/generateNumHexagons


PostBuild.geoToH3.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/geoToH3
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/geoToH3:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/geoToH3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/geoToH3.build/Objects-normal/armv7/geoToH3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/geoToH3.build/Objects-normal/armv7s/geoToH3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/geoToH3.build/Objects-normal/arm64/geoToH3


PostBuild.h3.MinSizeRel:
/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3:\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/h3.build/Objects-normal/armv7/h3.framework/h3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/h3.build/Objects-normal/armv7s/h3.framework/h3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/h3.build/Objects-normal/arm64/h3.framework/h3


PostBuild.h3ToComponents.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/h3ToComponents
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/h3ToComponents:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/h3ToComponents
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/h3ToComponents.build/Objects-normal/armv7/h3ToComponents
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/h3ToComponents.build/Objects-normal/armv7s/h3ToComponents
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/h3ToComponents.build/Objects-normal/arm64/h3ToComponents


PostBuild.h3ToGeo.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/h3ToGeo
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/h3ToGeo:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/h3ToGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/h3ToGeo.build/Objects-normal/armv7/h3ToGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/h3ToGeo.build/Objects-normal/armv7s/h3ToGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/h3ToGeo.build/Objects-normal/arm64/h3ToGeo


PostBuild.h3ToGeoBoundary.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/h3ToGeoBoundary
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/h3ToGeoBoundary:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/h3ToGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/h3ToGeoBoundary.build/Objects-normal/armv7/h3ToGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/h3ToGeoBoundary.build/Objects-normal/armv7s/h3ToGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/h3ToGeoBoundary.build/Objects-normal/arm64/h3ToGeoBoundary


PostBuild.h3ToGeoBoundaryHier.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/h3ToGeoBoundaryHier
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/h3ToGeoBoundaryHier:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/h3ToGeoBoundaryHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/h3ToGeoBoundaryHier.build/Objects-normal/armv7/h3ToGeoBoundaryHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/h3ToGeoBoundaryHier.build/Objects-normal/armv7s/h3ToGeoBoundaryHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/h3ToGeoBoundaryHier.build/Objects-normal/arm64/h3ToGeoBoundaryHier


PostBuild.h3ToGeoHier.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/h3ToGeoHier
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/h3ToGeoHier:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/h3ToGeoHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/h3ToGeoHier.build/Objects-normal/armv7/h3ToGeoHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/h3ToGeoHier.build/Objects-normal/armv7s/h3ToGeoHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/h3ToGeoHier.build/Objects-normal/arm64/h3ToGeoHier


PostBuild.h3ToHier.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/h3ToHier
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/h3ToHier:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/h3ToHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/h3ToHier.build/Objects-normal/armv7/h3ToHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/h3ToHier.build/Objects-normal/armv7s/h3ToHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/h3ToHier.build/Objects-normal/arm64/h3ToHier


PostBuild.h3ToIjk.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/h3ToIjk
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/h3ToIjk:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/h3ToIjk
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/h3ToIjk.build/Objects-normal/armv7/h3ToIjk
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/h3ToIjk.build/Objects-normal/armv7s/h3ToIjk
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/h3ToIjk.build/Objects-normal/arm64/h3ToIjk


PostBuild.hexRange.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/hexRange
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/hexRange:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/hexRange
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/hexRange.build/Objects-normal/armv7/hexRange
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/hexRange.build/Objects-normal/armv7s/hexRange
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/hexRange.build/Objects-normal/arm64/hexRange


PostBuild.kRing.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/kRing
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/kRing:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/kRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/kRing.build/Objects-normal/armv7/kRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/kRing.build/Objects-normal/armv7s/kRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/kRing.build/Objects-normal/arm64/kRing


PostBuild.mkRandGeo.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/mkRandGeo
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/mkRandGeo:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/mkRandGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/mkRandGeo.build/Objects-normal/armv7/mkRandGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/mkRandGeo.build/Objects-normal/armv7s/mkRandGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/mkRandGeo.build/Objects-normal/arm64/mkRandGeo


PostBuild.mkRandGeoBoundary.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/mkRandGeoBoundary
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/mkRandGeoBoundary:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/mkRandGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/mkRandGeoBoundary.build/Objects-normal/armv7/mkRandGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/mkRandGeoBoundary.build/Objects-normal/armv7s/mkRandGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/mkRandGeoBoundary.build/Objects-normal/arm64/mkRandGeoBoundary


PostBuild.testBBox.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testBBox
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testBBox:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testBBox
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testBBox.build/Objects-normal/armv7/testBBox
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testBBox.build/Objects-normal/armv7s/testBBox
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testBBox.build/Objects-normal/arm64/testBBox


PostBuild.testCompact.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testCompact
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testCompact:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testCompact
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testCompact.build/Objects-normal/armv7/testCompact
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testCompact.build/Objects-normal/armv7s/testCompact
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testCompact.build/Objects-normal/arm64/testCompact


PostBuild.testGeoCoord.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testGeoCoord
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testGeoCoord:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testGeoCoord
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testGeoCoord.build/Objects-normal/armv7/testGeoCoord
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testGeoCoord.build/Objects-normal/armv7s/testGeoCoord
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testGeoCoord.build/Objects-normal/arm64/testGeoCoord


PostBuild.testGeoToH3.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testGeoToH3
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testGeoToH3:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testGeoToH3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testGeoToH3.build/Objects-normal/armv7/testGeoToH3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testGeoToH3.build/Objects-normal/armv7s/testGeoToH3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testGeoToH3.build/Objects-normal/arm64/testGeoToH3


PostBuild.testH3Api.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3Api
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3Api:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3Api
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3Api.build/Objects-normal/armv7/testH3Api
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3Api.build/Objects-normal/armv7s/testH3Api
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3Api.build/Objects-normal/arm64/testH3Api


PostBuild.testH3Index.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3Index
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3Index:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3Index
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3Index.build/Objects-normal/armv7/testH3Index
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3Index.build/Objects-normal/armv7s/testH3Index
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3Index.build/Objects-normal/arm64/testH3Index


PostBuild.testH3NeighborRotations.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3NeighborRotations
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3NeighborRotations:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3NeighborRotations
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3NeighborRotations.build/Objects-normal/armv7/testH3NeighborRotations
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3NeighborRotations.build/Objects-normal/armv7s/testH3NeighborRotations
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3NeighborRotations.build/Objects-normal/arm64/testH3NeighborRotations


PostBuild.testH3SetToLinkedGeo.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3SetToLinkedGeo
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3SetToLinkedGeo:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3SetToLinkedGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3SetToLinkedGeo.build/Objects-normal/armv7/testH3SetToLinkedGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3SetToLinkedGeo.build/Objects-normal/armv7s/testH3SetToLinkedGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3SetToLinkedGeo.build/Objects-normal/arm64/testH3SetToLinkedGeo


PostBuild.testH3SetToVertexGraph.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3SetToVertexGraph
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3SetToVertexGraph:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3SetToVertexGraph
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3SetToVertexGraph.build/Objects-normal/armv7/testH3SetToVertexGraph
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3SetToVertexGraph.build/Objects-normal/armv7s/testH3SetToVertexGraph
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3SetToVertexGraph.build/Objects-normal/arm64/testH3SetToVertexGraph


PostBuild.testH3ToChildren.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3ToChildren
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3ToChildren:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3ToChildren
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3ToChildren.build/Objects-normal/armv7/testH3ToChildren
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3ToChildren.build/Objects-normal/armv7s/testH3ToChildren
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3ToChildren.build/Objects-normal/arm64/testH3ToChildren


PostBuild.testH3ToGeo.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3ToGeo
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3ToGeo:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3ToGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3ToGeo.build/Objects-normal/armv7/testH3ToGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3ToGeo.build/Objects-normal/armv7s/testH3ToGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3ToGeo.build/Objects-normal/arm64/testH3ToGeo


PostBuild.testH3ToGeoBoundary.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3ToGeoBoundary
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3ToGeoBoundary:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3ToGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3ToGeoBoundary.build/Objects-normal/armv7/testH3ToGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3ToGeoBoundary.build/Objects-normal/armv7s/testH3ToGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3ToGeoBoundary.build/Objects-normal/arm64/testH3ToGeoBoundary


PostBuild.testH3ToIjk.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3ToIjk
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3ToIjk:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3ToIjk
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3ToIjk.build/Objects-normal/armv7/testH3ToIjk
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3ToIjk.build/Objects-normal/armv7s/testH3ToIjk
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3ToIjk.build/Objects-normal/arm64/testH3ToIjk


PostBuild.testH3ToParent.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3ToParent
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3ToParent:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3ToParent
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3ToParent.build/Objects-normal/armv7/testH3ToParent
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3ToParent.build/Objects-normal/armv7s/testH3ToParent
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3ToParent.build/Objects-normal/arm64/testH3ToParent


PostBuild.testH3UniEdge.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3UniEdge
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3UniEdge:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testH3UniEdge
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3UniEdge.build/Objects-normal/armv7/testH3UniEdge
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3UniEdge.build/Objects-normal/armv7s/testH3UniEdge
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testH3UniEdge.build/Objects-normal/arm64/testH3UniEdge


PostBuild.testHexRanges.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testHexRanges
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testHexRanges:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testHexRanges
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testHexRanges.build/Objects-normal/armv7/testHexRanges
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testHexRanges.build/Objects-normal/armv7s/testHexRanges
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testHexRanges.build/Objects-normal/arm64/testHexRanges


PostBuild.testHexRing.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testHexRing
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testHexRing:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testHexRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testHexRing.build/Objects-normal/armv7/testHexRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testHexRing.build/Objects-normal/armv7s/testHexRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testHexRing.build/Objects-normal/arm64/testHexRing


PostBuild.testKRing.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testKRing
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testKRing:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testKRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testKRing.build/Objects-normal/armv7/testKRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testKRing.build/Objects-normal/armv7s/testKRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testKRing.build/Objects-normal/arm64/testKRing


PostBuild.testLinkedGeo.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testLinkedGeo
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testLinkedGeo:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testLinkedGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testLinkedGeo.build/Objects-normal/armv7/testLinkedGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testLinkedGeo.build/Objects-normal/armv7s/testLinkedGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testLinkedGeo.build/Objects-normal/arm64/testLinkedGeo


PostBuild.testMaxH3ToChildrenSize.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testMaxH3ToChildrenSize
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testMaxH3ToChildrenSize:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testMaxH3ToChildrenSize
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testMaxH3ToChildrenSize.build/Objects-normal/armv7/testMaxH3ToChildrenSize
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testMaxH3ToChildrenSize.build/Objects-normal/armv7s/testMaxH3ToChildrenSize
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testMaxH3ToChildrenSize.build/Objects-normal/arm64/testMaxH3ToChildrenSize


PostBuild.testPolyfill.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testPolyfill
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testPolyfill:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testPolyfill
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testPolyfill.build/Objects-normal/armv7/testPolyfill
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testPolyfill.build/Objects-normal/armv7s/testPolyfill
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testPolyfill.build/Objects-normal/arm64/testPolyfill


PostBuild.testVec2d.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testVec2d
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testVec2d:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testVec2d
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testVec2d.build/Objects-normal/armv7/testVec2d
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testVec2d.build/Objects-normal/armv7s/testVec2d
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testVec2d.build/Objects-normal/arm64/testVec2d


PostBuild.testVec3d.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testVec3d
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testVec3d:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testVec3d
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testVec3d.build/Objects-normal/armv7/testVec3d
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testVec3d.build/Objects-normal/armv7s/testVec3d
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testVec3d.build/Objects-normal/arm64/testVec3d


PostBuild.testVertexGraph.MinSizeRel:
PostBuild.h3.MinSizeRel: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testVertexGraph
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testVertexGraph:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/MinSizeRel/testVertexGraph
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testVertexGraph.build/Objects-normal/armv7/testVertexGraph
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testVertexGraph.build/Objects-normal/armv7s/testVertexGraph
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/MinSizeRel/testVertexGraph.build/Objects-normal/arm64/testVertexGraph


PostBuild.benchmarkH3Api.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/benchmarkH3Api
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/benchmarkH3Api:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/benchmarkH3Api
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/benchmarkH3Api.build/Objects-normal/armv7/benchmarkH3Api
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/benchmarkH3Api.build/Objects-normal/armv7s/benchmarkH3Api
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/benchmarkH3Api.build/Objects-normal/arm64/benchmarkH3Api


PostBuild.benchmarkPolyfill.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/benchmarkPolyfill
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/benchmarkPolyfill:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/benchmarkPolyfill
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/benchmarkPolyfill.build/Objects-normal/armv7/benchmarkPolyfill
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/benchmarkPolyfill.build/Objects-normal/armv7s/benchmarkPolyfill
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/benchmarkPolyfill.build/Objects-normal/arm64/benchmarkPolyfill


PostBuild.generateBaseCellNeighbors.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/generateBaseCellNeighbors
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/generateBaseCellNeighbors:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/generateBaseCellNeighbors
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/generateBaseCellNeighbors.build/Objects-normal/armv7/generateBaseCellNeighbors
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/generateBaseCellNeighbors.build/Objects-normal/armv7s/generateBaseCellNeighbors
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/generateBaseCellNeighbors.build/Objects-normal/arm64/generateBaseCellNeighbors


PostBuild.generateFaceCenterPoint.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/generateFaceCenterPoint
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/generateFaceCenterPoint:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/generateFaceCenterPoint
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/generateFaceCenterPoint.build/Objects-normal/armv7/generateFaceCenterPoint
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/generateFaceCenterPoint.build/Objects-normal/armv7s/generateFaceCenterPoint
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/generateFaceCenterPoint.build/Objects-normal/arm64/generateFaceCenterPoint


PostBuild.generateNumHexagons.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/generateNumHexagons
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/generateNumHexagons:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/generateNumHexagons
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/generateNumHexagons.build/Objects-normal/armv7/generateNumHexagons
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/generateNumHexagons.build/Objects-normal/armv7s/generateNumHexagons
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/generateNumHexagons.build/Objects-normal/arm64/generateNumHexagons


PostBuild.geoToH3.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/geoToH3
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/geoToH3:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/geoToH3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/geoToH3.build/Objects-normal/armv7/geoToH3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/geoToH3.build/Objects-normal/armv7s/geoToH3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/geoToH3.build/Objects-normal/arm64/geoToH3


PostBuild.h3.RelWithDebInfo:
/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3:\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/h3.build/Objects-normal/armv7/h3.framework/h3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/h3.build/Objects-normal/armv7s/h3.framework/h3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/h3.build/Objects-normal/arm64/h3.framework/h3


PostBuild.h3ToComponents.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/h3ToComponents
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/h3ToComponents:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/h3ToComponents
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/h3ToComponents.build/Objects-normal/armv7/h3ToComponents
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/h3ToComponents.build/Objects-normal/armv7s/h3ToComponents
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/h3ToComponents.build/Objects-normal/arm64/h3ToComponents


PostBuild.h3ToGeo.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/h3ToGeo
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/h3ToGeo:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/h3ToGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/h3ToGeo.build/Objects-normal/armv7/h3ToGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/h3ToGeo.build/Objects-normal/armv7s/h3ToGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/h3ToGeo.build/Objects-normal/arm64/h3ToGeo


PostBuild.h3ToGeoBoundary.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/h3ToGeoBoundary
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/h3ToGeoBoundary:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/h3ToGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/h3ToGeoBoundary.build/Objects-normal/armv7/h3ToGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/h3ToGeoBoundary.build/Objects-normal/armv7s/h3ToGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/h3ToGeoBoundary.build/Objects-normal/arm64/h3ToGeoBoundary


PostBuild.h3ToGeoBoundaryHier.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/h3ToGeoBoundaryHier
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/h3ToGeoBoundaryHier:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/h3ToGeoBoundaryHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/h3ToGeoBoundaryHier.build/Objects-normal/armv7/h3ToGeoBoundaryHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/h3ToGeoBoundaryHier.build/Objects-normal/armv7s/h3ToGeoBoundaryHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/h3ToGeoBoundaryHier.build/Objects-normal/arm64/h3ToGeoBoundaryHier


PostBuild.h3ToGeoHier.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/h3ToGeoHier
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/h3ToGeoHier:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/h3ToGeoHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/h3ToGeoHier.build/Objects-normal/armv7/h3ToGeoHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/h3ToGeoHier.build/Objects-normal/armv7s/h3ToGeoHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/h3ToGeoHier.build/Objects-normal/arm64/h3ToGeoHier


PostBuild.h3ToHier.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/h3ToHier
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/h3ToHier:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/h3ToHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/h3ToHier.build/Objects-normal/armv7/h3ToHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/h3ToHier.build/Objects-normal/armv7s/h3ToHier
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/h3ToHier.build/Objects-normal/arm64/h3ToHier


PostBuild.h3ToIjk.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/h3ToIjk
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/h3ToIjk:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/h3ToIjk
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/h3ToIjk.build/Objects-normal/armv7/h3ToIjk
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/h3ToIjk.build/Objects-normal/armv7s/h3ToIjk
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/h3ToIjk.build/Objects-normal/arm64/h3ToIjk


PostBuild.hexRange.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/hexRange
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/hexRange:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/hexRange
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/hexRange.build/Objects-normal/armv7/hexRange
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/hexRange.build/Objects-normal/armv7s/hexRange
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/hexRange.build/Objects-normal/arm64/hexRange


PostBuild.kRing.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/kRing
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/kRing:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/kRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/kRing.build/Objects-normal/armv7/kRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/kRing.build/Objects-normal/armv7s/kRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/kRing.build/Objects-normal/arm64/kRing


PostBuild.mkRandGeo.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/mkRandGeo
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/mkRandGeo:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/mkRandGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/mkRandGeo.build/Objects-normal/armv7/mkRandGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/mkRandGeo.build/Objects-normal/armv7s/mkRandGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/mkRandGeo.build/Objects-normal/arm64/mkRandGeo


PostBuild.mkRandGeoBoundary.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/mkRandGeoBoundary
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/mkRandGeoBoundary:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/mkRandGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/mkRandGeoBoundary.build/Objects-normal/armv7/mkRandGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/mkRandGeoBoundary.build/Objects-normal/armv7s/mkRandGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/mkRandGeoBoundary.build/Objects-normal/arm64/mkRandGeoBoundary


PostBuild.testBBox.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testBBox
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testBBox:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testBBox
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testBBox.build/Objects-normal/armv7/testBBox
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testBBox.build/Objects-normal/armv7s/testBBox
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testBBox.build/Objects-normal/arm64/testBBox


PostBuild.testCompact.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testCompact
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testCompact:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testCompact
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testCompact.build/Objects-normal/armv7/testCompact
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testCompact.build/Objects-normal/armv7s/testCompact
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testCompact.build/Objects-normal/arm64/testCompact


PostBuild.testGeoCoord.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testGeoCoord
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testGeoCoord:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testGeoCoord
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testGeoCoord.build/Objects-normal/armv7/testGeoCoord
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testGeoCoord.build/Objects-normal/armv7s/testGeoCoord
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testGeoCoord.build/Objects-normal/arm64/testGeoCoord


PostBuild.testGeoToH3.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testGeoToH3
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testGeoToH3:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testGeoToH3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testGeoToH3.build/Objects-normal/armv7/testGeoToH3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testGeoToH3.build/Objects-normal/armv7s/testGeoToH3
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testGeoToH3.build/Objects-normal/arm64/testGeoToH3


PostBuild.testH3Api.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3Api
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3Api:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3Api
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3Api.build/Objects-normal/armv7/testH3Api
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3Api.build/Objects-normal/armv7s/testH3Api
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3Api.build/Objects-normal/arm64/testH3Api


PostBuild.testH3Index.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3Index
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3Index:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3Index
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3Index.build/Objects-normal/armv7/testH3Index
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3Index.build/Objects-normal/armv7s/testH3Index
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3Index.build/Objects-normal/arm64/testH3Index


PostBuild.testH3NeighborRotations.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3NeighborRotations
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3NeighborRotations:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3NeighborRotations
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3NeighborRotations.build/Objects-normal/armv7/testH3NeighborRotations
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3NeighborRotations.build/Objects-normal/armv7s/testH3NeighborRotations
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3NeighborRotations.build/Objects-normal/arm64/testH3NeighborRotations


PostBuild.testH3SetToLinkedGeo.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3SetToLinkedGeo
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3SetToLinkedGeo:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3SetToLinkedGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3SetToLinkedGeo.build/Objects-normal/armv7/testH3SetToLinkedGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3SetToLinkedGeo.build/Objects-normal/armv7s/testH3SetToLinkedGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3SetToLinkedGeo.build/Objects-normal/arm64/testH3SetToLinkedGeo


PostBuild.testH3SetToVertexGraph.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3SetToVertexGraph
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3SetToVertexGraph:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3SetToVertexGraph
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3SetToVertexGraph.build/Objects-normal/armv7/testH3SetToVertexGraph
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3SetToVertexGraph.build/Objects-normal/armv7s/testH3SetToVertexGraph
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3SetToVertexGraph.build/Objects-normal/arm64/testH3SetToVertexGraph


PostBuild.testH3ToChildren.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3ToChildren
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3ToChildren:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3ToChildren
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3ToChildren.build/Objects-normal/armv7/testH3ToChildren
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3ToChildren.build/Objects-normal/armv7s/testH3ToChildren
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3ToChildren.build/Objects-normal/arm64/testH3ToChildren


PostBuild.testH3ToGeo.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3ToGeo
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3ToGeo:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3ToGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3ToGeo.build/Objects-normal/armv7/testH3ToGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3ToGeo.build/Objects-normal/armv7s/testH3ToGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3ToGeo.build/Objects-normal/arm64/testH3ToGeo


PostBuild.testH3ToGeoBoundary.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3ToGeoBoundary
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3ToGeoBoundary:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3ToGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3ToGeoBoundary.build/Objects-normal/armv7/testH3ToGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3ToGeoBoundary.build/Objects-normal/armv7s/testH3ToGeoBoundary
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3ToGeoBoundary.build/Objects-normal/arm64/testH3ToGeoBoundary


PostBuild.testH3ToIjk.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3ToIjk
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3ToIjk:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3ToIjk
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3ToIjk.build/Objects-normal/armv7/testH3ToIjk
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3ToIjk.build/Objects-normal/armv7s/testH3ToIjk
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3ToIjk.build/Objects-normal/arm64/testH3ToIjk


PostBuild.testH3ToParent.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3ToParent
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3ToParent:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3ToParent
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3ToParent.build/Objects-normal/armv7/testH3ToParent
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3ToParent.build/Objects-normal/armv7s/testH3ToParent
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3ToParent.build/Objects-normal/arm64/testH3ToParent


PostBuild.testH3UniEdge.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3UniEdge
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3UniEdge:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testH3UniEdge
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3UniEdge.build/Objects-normal/armv7/testH3UniEdge
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3UniEdge.build/Objects-normal/armv7s/testH3UniEdge
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testH3UniEdge.build/Objects-normal/arm64/testH3UniEdge


PostBuild.testHexRanges.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testHexRanges
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testHexRanges:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testHexRanges
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testHexRanges.build/Objects-normal/armv7/testHexRanges
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testHexRanges.build/Objects-normal/armv7s/testHexRanges
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testHexRanges.build/Objects-normal/arm64/testHexRanges


PostBuild.testHexRing.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testHexRing
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testHexRing:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testHexRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testHexRing.build/Objects-normal/armv7/testHexRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testHexRing.build/Objects-normal/armv7s/testHexRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testHexRing.build/Objects-normal/arm64/testHexRing


PostBuild.testKRing.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testKRing
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testKRing:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testKRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testKRing.build/Objects-normal/armv7/testKRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testKRing.build/Objects-normal/armv7s/testKRing
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testKRing.build/Objects-normal/arm64/testKRing


PostBuild.testLinkedGeo.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testLinkedGeo
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testLinkedGeo:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testLinkedGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testLinkedGeo.build/Objects-normal/armv7/testLinkedGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testLinkedGeo.build/Objects-normal/armv7s/testLinkedGeo
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testLinkedGeo.build/Objects-normal/arm64/testLinkedGeo


PostBuild.testMaxH3ToChildrenSize.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testMaxH3ToChildrenSize
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testMaxH3ToChildrenSize:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testMaxH3ToChildrenSize
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testMaxH3ToChildrenSize.build/Objects-normal/armv7/testMaxH3ToChildrenSize
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testMaxH3ToChildrenSize.build/Objects-normal/armv7s/testMaxH3ToChildrenSize
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testMaxH3ToChildrenSize.build/Objects-normal/arm64/testMaxH3ToChildrenSize


PostBuild.testPolyfill.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testPolyfill
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testPolyfill:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testPolyfill
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testPolyfill.build/Objects-normal/armv7/testPolyfill
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testPolyfill.build/Objects-normal/armv7s/testPolyfill
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testPolyfill.build/Objects-normal/arm64/testPolyfill


PostBuild.testVec2d.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testVec2d
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testVec2d:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testVec2d
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testVec2d.build/Objects-normal/armv7/testVec2d
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testVec2d.build/Objects-normal/armv7s/testVec2d
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testVec2d.build/Objects-normal/arm64/testVec2d


PostBuild.testVec3d.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testVec3d
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testVec3d:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testVec3d
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testVec3d.build/Objects-normal/armv7/testVec3d
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testVec3d.build/Objects-normal/armv7s/testVec3d
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testVec3d.build/Objects-normal/arm64/testVec3d


PostBuild.testVertexGraph.RelWithDebInfo:
PostBuild.h3.RelWithDebInfo: /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testVertexGraph
/Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testVertexGraph:\
	/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3\
	/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/bin/RelWithDebInfo/testVertexGraph
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testVertexGraph.build/Objects-normal/armv7/testVertexGraph
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testVertexGraph.build/Objects-normal/armv7s/testVertexGraph
	/bin/rm -f /Users/masato/dev/src/github.com/pocket7878/h3-framework/h3.build/RelWithDebInfo/testVertexGraph.build/Objects-normal/arm64/testVertexGraph




# For each target create a dummy ruleso the target does not have to exist
/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.4.sdk/usr/lib/libm.tbd:
/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Debug/h3.framework/h3:
/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/MinSizeRel/h3.framework/h3:
/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/RelWithDebInfo/h3.framework/h3:
/Users/masato/dev/src/github.com/pocket7878/h3-framework/lib/Release/h3.framework/h3:
