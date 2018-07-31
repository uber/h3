//
//  PolyfillViewController.swift
//  h3SwiftTestApp
//
//  Created by 十亀眞怜 on 2018/07/31.
//

import UIKit
import MapKit
import CoreLocation
import h3Swift

class PolyfillViewController: UIViewController {

    @IBOutlet weak var mapView: MKMapView!
    
    let h3Resolution = 9
    
    override func viewDidLoad() {
        super.viewDidLoad()
        mapView.delegate = self
        DispatchQueue.global(qos: .background).async { [weak self] in
            guard let strongSelf = self else {
                return
            }
            var acc: [MKPolygon] = []
            var seen: Set<H3Index> = Set()
            for lat in stride(from: 35.5, to: 35.85, by: 0.002){
                for lng in stride(from: 139.5, to: 139.95, by: 0.002) {
                    var coord = GeoCoord(lat: degsToRads(lat), lon: degsToRads(lng))
                    let h3Idx = coord.toH3(res: Int32(strongSelf.h3Resolution))
                    if !(seen.contains(h3Idx)) {
                        seen.insert(h3Idx)
                        acc.append(strongSelf.h3IndexPolygon(h3Idx))
                    }
                }
            }
            DispatchQueue.main.async { [weak self] in
                guard let strongSelf = self else {
                    return
                }
                strongSelf.mapView.addOverlays(acc)
            }
        }
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        for o in mapView.overlays {
            mapView.remove(o)
        }
    }
    
    private func h3IndexPolygon(_ idx: H3Index) -> MKPolygon {
        let coords = idx.geoBoundary().map {$0.toLocation2D()}
        return MKPolygon(coordinates: coords, count: coords.count)
    }
}

fileprivate extension GeoCoord {
    func toLocation2D() -> CLLocationCoordinate2D {
        return CLLocationCoordinate2D(
            latitude: radsToDegs(self.lat),
            longitude: radsToDegs(self.lon))
    }
}

extension PolyfillViewController: MKMapViewDelegate {
    func mapView(_ mapView: MKMapView, rendererFor overlay: MKOverlay) -> MKOverlayRenderer {
        let poly = MKPolygonRenderer(overlay: overlay)
        poly.fillColor = UIColor(displayP3Red: CGFloat(Float(arc4random_uniform(256)) / 255.0), green: 0, blue: 0, alpha: 0.5)
        return poly
    }
}
