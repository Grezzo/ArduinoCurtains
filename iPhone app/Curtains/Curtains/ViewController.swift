//
//  ViewController.swift
//  Curtains
//
//  Created by Graeme Robinson on 24/12/2014.
//  Copyright (c) 2014 Graeme Robinson. All rights reserved.
//

import UIKit

class ViewController: UIViewController {

    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view, typically from a nib.
        let tint = UIColor.yellowColor()
        view.tintColor = tint
        UISwitch.appearance().onTintColor = tint
    }
    
    override func viewDidAppear(animated: Bool) {
        super.viewDidAppear(animated)
        
        //Swap min/max track images for right slider
        let minimumImage = rightSlider.minimumTrackImageForState(UIControlState.Normal)
        let maximumImage = rightSlider.maximumTrackImageForState(UIControlState.Normal)
        rightSlider.setMaximumTrackImage(minimumImage, forState: .Normal)
        rightSlider.setMinimumTrackImage(maximumImage, forState: .Normal)
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    func moveSliders(value: Float) {
        UIView.animateWithDuration(0.25, {
            self.leftSlider.setValue(value, animated: true)
            self.rightSlider.setValue(100 - value, animated: true)
        })
        moveRightCurtain()
    }
    
    func moveRightCurtain() {
        let percent = 100 - Int(roundf(rightSlider.value))
        RESTcommand("moveto?position=" + "\(percent)")
    }
    
    func RESTcommand(command: String) {
        let URLSession = NSURLSession(configuration: NSURLSessionConfiguration.defaultSessionConfiguration())
        let task = URLSession.dataTaskWithRequest(NSURLRequest(URL: NSURL(string: "http://gr-macbook.lan:1500/" + command)!))
        task.resume()
    }

    @IBAction func leftSliderChanged(sender: UISlider) {
        if (lockSwitch.on) {
            rightSlider.value = 100 - leftSlider.value
        }
    }
    
    @IBAction func leftSliderFinished(sender: UISlider) {
        if (lockSwitch.on) {
            moveRightCurtain()
        }
    }
    
    @IBAction func rightSliderChanged(sender: UISlider) {
        if (lockSwitch.on) {
            leftSlider.value = 100 - rightSlider.value
        }
        
    }
    
    @IBAction func rightSliderFinished(sender: UISlider) {
        moveRightCurtain()
    }
    
    @IBAction func lockSwitchChanged(sender: UISwitch) {
        let averageValue = (leftSlider.value + (100 - rightSlider.value)) / 2
        moveSliders(averageValue)
    }
    
    @IBAction func OpenCurtains(sender: UIButton) {
        moveSliders(0)
    }
    
    @IBAction func CloseCurtains(sender: UIButton) {
        moveSliders(100)
    }
    

    
    @IBOutlet weak var leftSlider: UISlider!
    @IBOutlet weak var rightSlider: UISlider!
    @IBOutlet weak var lockSwitch: UISwitch!

}
