//
//  ViewController.swift
//  h3SwiftTestApp
//
//  Created by 十亀眞怜 on 2018/07/27.
//

import UIKit
import h3Swift
import MapKit

class MapViewController: UIViewController {

    let lm = CLLocationManager()
    
    var centerLocation: CLLocationCoordinate2D? = nil
    
    @IBOutlet weak var mapCenterView: UIView!
    @IBOutlet weak var mapView: MKMapView!
    @IBOutlet weak var tableView: UITableView!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        lm.delegate = self
        lm.requestWhenInUseAuthorization()
        
        mapView.delegate = self
        tableView.dataSource = self
        
        mapCenterView.backgroundColor = UIColor.red
        mapCenterView.layer.cornerRadius = 1.0
        mapCenterView.clipsToBounds = true
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
}

extension MapViewController: CLLocationManagerDelegate {
    func locationManager(_ manager: CLLocationManager, didChangeAuthorization status: CLAuthorizationStatus) {
        switch(status) {
        case .authorizedWhenInUse:
            lm.startUpdatingLocation()
        default:
            debugPrint("OTher status")
        }
    }
    
    func locationManager(_ manager: CLLocationManager, didFailWithError error: Error) {
        debugPrint(error)
    }
}

extension MapViewController: MKMapViewDelegate {
    func mapView(_ mapView: MKMapView, regionDidChangeAnimated animated: Bool) {
        self.centerLocation = mapView.region.center
        tableView.reloadData()
    }
}

extension MapViewController: UITableViewDataSource {
    func numberOfSections(in tableView: UITableView) -> Int {
        return 2
    }
    
    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        switch(section) {
        case 0:
            return 2
        case 1:
            return 16
        default:
            return 0
        }
    }
    
    func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        switch(section) {
        case 0:
            return "Location"
        case 1:
            return "H3Index"
        default:
            return nil
        }
    }
    
    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = UITableViewCell(style: .value1, reuseIdentifier: "Cell")
        guard let loc = centerLocation else {
            return cell
        }
        switch(indexPath.section) {
        case 0:
            switch(indexPath.row) {
            case 0:
                cell.textLabel?.text = "Latitude"
                cell.detailTextLabel?.text = "\(loc.latitude)"
            case 1:
                cell.textLabel?.text = "Longitude"
                cell.detailTextLabel?.text = "\(loc.longitude)"
            default:
                break
            }
        case 1:
            cell.textLabel?.text = "Res: \(indexPath.row)"
            var coord = GeoCoord(lat: degsToRads(loc.latitude), lon: degsToRads(loc.longitude))
            cell.detailTextLabel?.text = "\(coord.toH3(res: Int32(indexPath.row)).toString())"
        default:
            break
        }
        return cell
    }
}

extension MapViewController: UITableViewDelegate {
    func tableView(_ tableView: UITableView, shouldShowMenuForRowAt indexPath: IndexPath) -> Bool {
        return indexPath.section == 1
    }
    
    func tableView(_ tableView: UITableView, canPerformAction action: Selector, forRowAt indexPath: IndexPath, withSender sender: Any?) -> Bool {
        return action == #selector(copy(_:))
    }
    
    func tableView(_ tableView: UITableView, performAction action: Selector, forRowAt indexPath: IndexPath, withSender sender: Any?) {
        guard let loc = centerLocation else {
            return
        }
        let res = indexPath.row
        var coord = GeoCoord(lat: degsToRads(loc.latitude), lon: degsToRads(loc.longitude))
        let h3Index = coord.toH3(res: Int32(res)).toString()
        DispatchQueue.global(qos: .background).async {
            UIPasteboard.general.string = h3Index
        }
    }
}
