//
//  DrawHexagonViewController.swift
//  h3SwiftTestApp
//
//  Created by 十亀眞怜 on 2018/07/30.
//

import UIKit
import h3Swift
import MapKit
import CoreLocation

class DrawHexagonViewController: UIViewController {

    @IBOutlet weak var mapView: MKMapView!
    @IBOutlet weak var h3InputField: UITextField!
    override func viewDidLoad() {
        super.viewDidLoad()
        mapView.delegate = self
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    @IBAction func updateBtnTouched(_ sender: Any) {
        guard let rawHex = h3InputField.text else {
            return
        }
        let idx = H3Index(string: rawHex)
        let boundary: [CLLocationCoordinate2D] = idx.geoBoundary().map { geoCoordToCLLoc($0) }
        let poly = MKPolygon(coordinates: boundary, count: boundary.count)
        for o in mapView.overlays {
            mapView.remove(o)
        }
        mapView.add(poly)
    }
    
    //MARK: Utils
    private func geoCoordToCLLoc(_ c: GeoCoord) -> CLLocationCoordinate2D {
        return CLLocationCoordinate2D(latitude: radsToDegs(c.lat), longitude: radsToDegs(c.lon))
    }
}

extension DrawHexagonViewController: MKMapViewDelegate {
    func mapView(_ mapView: MKMapView, rendererFor overlay: MKOverlay) -> MKOverlayRenderer {
        let renderer = MKPolygonRenderer(overlay: overlay)
        renderer.fillColor = UIColor.red
        renderer.alpha = 0.5
        return renderer
    }
}
