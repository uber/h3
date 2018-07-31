//
//  MainTableViewController.swift
//  h3SwiftTestApp
//
//  Created by 十亀眞怜 on 2018/07/28.
//

import UIKit

class MainTableViewController: UITableViewController {

    override func viewDidLoad() {
        super.viewDidLoad()

        // Uncomment the following line to preserve selection between presentations
        // self.clearsSelectionOnViewWillAppear = false

        // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
        // self.navigationItem.rightBarButtonItem = self.editButtonItem
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }

    // MARK: - Table view data source

    override func numberOfSections(in tableView: UITableView) -> Int {
        // #warning Incomplete implementation, return the number of sections
        return 1
    }

    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        // #warning Incomplete implementation, return the number of rows
        return 4
    }

    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = UITableViewCell(style: .default, reuseIdentifier: "Cell")
        switch(indexPath.section) {
        case 0:
            switch(indexPath.row) {
            case 0:
                cell.textLabel?.text = "Map"
            case 1:
                cell.textLabel?.text = "Decode"
            case 2:
                cell.textLabel?.text = "Draw Hexagon"
            case 3:
                cell.textLabel?.text = "Polyfill"
            default:
                break
            }
        default:
            break
        }
        return cell
    }

    override func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        switch(indexPath.section) {
        case 0:
            switch(indexPath.row) {
            case 0:
                performSegue(withIdentifier: "showMapView", sender: self)
            case 1:
                performSegue(withIdentifier: "showDecodeView", sender: self)
            case 2:
                performSegue(withIdentifier: "showDrawHexagonView", sender: self)
            case 3:
                performSegue(withIdentifier: "showPolyfillView", sender: self)
            default:
                break
            }
        default:
            break
        }
    }
}
