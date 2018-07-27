//
//  ViewController.swift
//  h3SwiftTestApp
//
//  Created by 十亀眞怜 on 2018/07/27.
//

import UIKit
import h3Swift
import MapKit

class ViewController: UIViewController {

    let lm = CLLocationManager()
    
    var centerLocation: CLLocationCoordinate2D? = nil
    
    @IBOutlet weak var mapView: MKMapView!
    @IBOutlet weak var tableView: UITableView!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        lm.delegate = self
        lm.requestWhenInUseAuthorization()
        
        mapView.delegate = self
        tableView.dataSource = self
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
}

extension ViewController: CLLocationManagerDelegate {
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

extension ViewController: MKMapViewDelegate {
    func mapView(_ mapView: MKMapView, regionDidChangeAnimated animated: Bool) {
        self.centerLocation = mapView.region.center
        tableView.reloadData()
    }
}

extension ViewController: UITableViewDataSource {
    func numberOfSections(in tableView: UITableView) -> Int {
        return 1
    }
    
    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        switch(section) {
        case 0:
            return 16
        default:
            return 0
        }
    }
    
    func tableView(_ tableView: UITableView, titleForFooterInSection section: Int) -> String? {
        switch(section) {
        case 0:
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
        if (indexPath.section == 0) {
            cell.textLabel?.text = "Res: \(indexPath.row)"
            var coord = GeoCoord(lat: degsToRads(loc.latitude), lon: degsToRads(loc.longitude))
            cell.detailTextLabel?.text = "\(coord.toH3(res: Int32(indexPath.row)))"
        }
        return cell
    }
}
