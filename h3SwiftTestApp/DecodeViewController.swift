//
//  DecodeViewController.swift
//  h3SwiftTestApp
//
//  Created by 十亀眞怜 on 2018/07/28.
//

import UIKit
import h3Swift

class DecodeViewController: UIViewController {

    var h3Index: H3Index? = nil
    @IBOutlet weak var h3IndexField: UITextField!
    @IBOutlet weak var decodeTableView: UITableView!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        decodeTableView.dataSource = self
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    @IBAction func onUpdateButtonTouched(_ sender: Any) {
        guard var idx = h3IndexField.text else {
            return
        }
        h3Index = H3Index(string: idx)
        decodeTableView.reloadData()
    }
}

extension DecodeViewController: UITableViewDataSource {
    func numberOfSections(in tableView: UITableView) -> Int {
        return 2
    }

    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        switch(section) {
        case 0:
            return 2
        case 1:
            return 6
        default:
            return 0
        }
    }
    
    func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        switch(section) {
        case 0:
            return "Location"
        case 1:
            return "Hexagon"
        default:
            return nil
        }
    }
    
    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = UITableViewCell(style: .value1, reuseIdentifier: "Cell")
        guard let idx = self.h3Index else {
            return cell
        }
        switch(indexPath.section) {
        case 0:
            let coord = GeoCoord.from(idx)
            if indexPath.row == 0 {
                cell.textLabel?.text = "Latitude"
                cell.detailTextLabel?.text = "\(radsToDegs(coord.lat))"
            } else {
                cell.textLabel?.text = "Longitude"
                cell.detailTextLabel?.text = "\(radsToDegs(coord.lon))"
            }
        case 1:
            var buf: GeoBoundary = GeoBoundary.zero
            idx.geoBoundary(gb: &buf)
            cell.textLabel?.text = "\(indexPath.row)"
            switch(indexPath.row) {
            case 0:
                cell.detailTextLabel?.text = "\(geoCoordToString(buf.verts.0))"
            case 1:
                cell.detailTextLabel?.text = "\(geoCoordToString(buf.verts.1))"
            case 2:
                cell.detailTextLabel?.text = "\(geoCoordToString(buf.verts.2))"
            case 3:
                cell.detailTextLabel?.text = "\(geoCoordToString(buf.verts.3))"
            case 4:
                cell.detailTextLabel?.text = "\(geoCoordToString(buf.verts.4))"
            case 5:
                cell.detailTextLabel?.text = "\(geoCoordToString(buf.verts.5))"
            default:
                break
            }
        default:
            break
        }
        return cell
    }
    
    fileprivate func geoCoordToString(_ coord: GeoCoord) -> String {
        return "\(radsToDegs(coord.lat)),\(radsToDegs(coord.lon))"
    }
}
