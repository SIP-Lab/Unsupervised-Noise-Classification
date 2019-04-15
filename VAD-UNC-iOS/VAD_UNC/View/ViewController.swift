//
//  ViewController.swift
//  CNN_VAD
//
//  Created by Abhishek Sehgal on 9/22/17.
//  Modified by Nasim Alamdari on 6/10/2018.
//  Copyright © 2017 SIPLab. All rights reserved.
//

import UIKit
import Foundation

class ViewController: UIViewController{
    
    //UI Elements
    @IBOutlet weak var uiTitle: UILabel!
    
    @IBOutlet weak var labelSpeech: UILabel!
    @IBOutlet weak var labelNoise: UILabel!
    @IBOutlet weak var labelProcessingTime: UILabel!
    @IBOutlet weak var labelNoisePower: UILabel!
    @IBOutlet weak var labelVigilance1: UILabel!
    @IBOutlet weak var labelVigilance2: UILabel!
    @IBOutlet weak var labelFramesinFeatAveraging: UILabel!
    @IBOutlet weak var labelNewClassCreationTime: UILabel!
    @IBOutlet weak var labelDecisionSmoothingTime: UILabel!
    @IBOutlet weak var utdLogoLabel: UILabel!
    
    
    @IBOutlet weak var TextVigilance1: UITextField!
    @IBOutlet weak var TextVigilance2: UITextField!
    @IBOutlet weak var TextFramesinFeatAveraging: UITextField!
    @IBOutlet weak var TextNewClassCreationTime: UITextField!
    @IBOutlet weak var TextDecisionSmoothingTime: UITextField!
    @IBOutlet weak var TextVeiwScreen: UITextView! // For displaying resutls of noise classifier
    
    @IBOutlet weak var buttonStart: UIButton!
    @IBOutlet weak var buttonReadFile: UIButton!
    @IBOutlet weak var buttonStop: UIButton!
    
    @IBOutlet weak var switchHybrid: UISwitch!
    @IBOutlet weak var switchSavingClassRep: UISwitch!
    @IBOutlet weak var switchStore: UISwitch!
 
    @IBOutlet weak var utdLogo: UIImageView!
    
    
    // local vars
    var predictTimer = Timer()
    var prediction = float_t()
    var count = 0
    var IsNoiseDetected = 1;
    var refresh: Timer!
    var timer:Timer!
    
    //User Defined Functions
    
    func buttonEnable(enable: Bool){
        //sliderUpdateRate.isEnabled          = enable
        TextVigilance1.isEnabled            = enable
        TextVigilance2.isEnabled            = enable
        TextFramesinFeatAveraging.isEnabled = enable
        TextNewClassCreationTime.isEnabled  = enable
        TextDecisionSmoothingTime.isEnabled = enable
        switchHybrid.isEnabled              = enable
        switchSavingClassRep.isEnabled      = enable
        switchStore.isEnabled               = enable
        buttonStart.isEnabled               = enable
        buttonStop.isEnabled                = !enable
        
        if(enable){
            buttonStart.backgroundColor = UIColor(red: 51.0/255.0, green: 153.0/255.0, blue: 0, alpha: 1)
            buttonStop.backgroundColor  = UIColor.lightGray
        } else {
            buttonStart.backgroundColor = UIColor.lightGray
            buttonStop.backgroundColor  = UIColor(red: 204.0/255.0, green: 0, blue: 0, alpha: 1)
        }
        
    }
    
    
    @IBAction func buttonStartClick(_ sender: Any) {
        buttonEnable(enable: false)
        audioRecorder.start()
        
        let image: UIImage = UIImage(named: "utd_logo.png")!
        utdLogoLabel.text = "Signal and Image Processing Lab"
        utdLogo.image =  image;
        
        
        predictTimer = Timer.scheduledTimer(withTimeInterval: 0.0625,
                                            repeats: true,
                                            block: {
                                                _ in audioRecorder.predict();
                                                self.count = self.count + 1
                                                
                                                if( audioRecorder.predictBuffer.count > 10){
                                                    self.labelProcessingTime.text = "Frame Processing Time:                                  \(round(100000*audioRecorder.timeBuffer.movingAverage)/100) ms"
                                                    self.labelNoisePower.text = "Noise Power:                                                     \(floor(audioRecorder.dbpower)) dB"
                                                    if(audioRecorder.predictBuffer.movingAverage > 0.5){
                                                        self.labelNoise.backgroundColor = UIColor(red: 1.0, green: 1.0, blue: 1.0, alpha: 1);
                                                        self.labelSpeech.backgroundColor = UIColor(red: 51.0/255.0, green: 153.0/255.0, blue: 0, alpha: 1);
                                                        self.IsNoiseDetected = 0;
                                                    } else {
                                                        self.labelNoise.backgroundColor = UIColor(red: 204.0/255.0, green: 0, blue: 0, alpha: 1);
                                                        self.labelSpeech.backgroundColor = UIColor(red: 1.0, green: 1.0, blue: 1.0, alpha: 1);
                                                        self.IsNoiseDetected = 1;
                                                    }
                                                    self.count = 0;
                                                }
        })
        
        timer = Timer.scheduledTimer(timeInterval: 1.0, // updating resutls of noise classifier every 1 second
            target: self,
            selector: #selector(self.updateVIEW),
            userInfo: nil,
            repeats: true)
        timer.fire() // start the timer, Causes the timer's message to be sent to its target.
        
    }
    
    
    
    // update inforView screen
    @objc func updateVIEW() {
        if (self.IsNoiseDetected == 1){
            self.TextVeiwScreen.text = self.TextVeiwScreen.text! + "\n Classified class " + String(audioRecorder.clusterLabel) +
                " out of " + String(audioRecorder.totalDetectedClusters)
                + "  classes.";
            let range = NSMakeRange(self.TextVeiwScreen.text.characters.count - 1, 0)
            self.TextVeiwScreen.scrollRangeToVisible(range)
            print("update is running")
        }
    }
    
    
    @IBAction func buttonStopClick(_ sender: Any) {
        predictTimer.invalidate()
        timer.invalidate() // Stops the timer from ever firing again and requests its removal from its run loop
        buttonEnable(enable: true)
        audioRecorder.stop()
    }

    
    //Pre-Defined Functions
    
    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view, typically from a nib.
        
        uiTitle.text = "VAD + Unsupervised Noise Classifier"
        //labName.text = "Signal and Image Processing Lab\nUniversity of Texas at Dallas"
        labelProcessingTime.text = "Frame Processing Time: "
        labelNoisePower.text     = "Noise Power: "
        //labelUpdateRate.text = "GUI Update Rate: 100ms"
        audioRecorder = AudioRecorder()
        buttonEnable(enable: true)
        
        // Notify when app enters foreground for updates
        NotificationCenter.default.addObserver(self,
                                               selector: #selector(appActiveAction),
                                               name: NSNotification.Name.UIApplicationDidBecomeActive,
                                               object: UIApplication.shared)
        
       
        //tableView.beginUpdates()
        //tableView.endUpdates()
        
        // saving/reading switch
        switchHybrid.isOn         = audioController.LoadingClassButton
        switchSavingClassRep.isOn = audioController.SavingClassButton
        switchStore.isOn          = audioController.SavingFeatButton
        
        // vigilance 1
        let Vigilance1:Float = Float(audioController.vigilanceVal1)
        TextVigilance1.text = String(format: "%.2f", Vigilance1)
        
        // vigilance 2
        let Vigilance2: Float = Float(audioController.vigilanceVal2)
        TextVigilance2.text = String(format: "%.2f", Vigilance2)
        
        // #Frames in feature averaging buffer
        let FeatAvgBuff: Int = Int(audioController.FeatAvgBufferLength)
        TextFramesinFeatAveraging.text = String(format: "%.d", FeatAvgBuff)
        
        // new class creation buffer time
        let newClassBuff: Float = Float(audioController.NewclusterCreationBuffer)
        TextNewClassCreationTime.text = String(format: "%.1f", newClassBuff)
        
        // decison smoothing buffer time
        let DecSmoothBuff: Float = Float(audioController.decisionSmoothingBuffer)
        TextDecisionSmoothingTime.text = String(format: "%.1f", DecSmoothBuff)
  
        // Recognize taps to exit keyboard
        let tapToExitKeyboard: UITapGestureRecognizer =
            UITapGestureRecognizer(target: self,
                                   action: #selector(self.dismissKeyboard))
        view.addGestureRecognizer(tapToExitKeyboard)
        
    }
    
    
    
    @IBAction func TextVigilance1EdittingDidEnd(_ sender: UITextField) {
        let Vigilance1: Float = Float(audioController.vigilanceVal1)
        if let text: String = sender.text {
            if let newVigilance1: Float = Float(text) {
                if newVigilance1 < 0 {
                    sender.text = String(format: "%.2f", Vigilance1)
                }
                else {
                    audioController.update(vigilanceVal1: newVigilance1)
                    sender.text = String(format: "%.2f", newVigilance1)
                }
            }
            else {
                sender.text = String(format: "%.2f", Vigilance1)
            }
        }
        else {
            sender.text = String(format: "%.2f", Vigilance1)
        }
    }
    
    
    @IBAction func TextVigilance2EdittingDidEnd(_ sender: UITextField) {
        let Vigilance2: Float = Float(audioController.vigilanceVal2)
        if let text: String = sender.text {
            if let newVigilance2: Float = Float(text) {
                if newVigilance2 < 0 {
                    sender.text = String(format: "%.2f", Vigilance2)
                }
                else {
                    audioController.update(vigilanceVal2: newVigilance2)
                    sender.text = String(format: "%.2f", newVigilance2)
                }
            }
            else {
                sender.text = String(format: "%.2f", Vigilance2)
            }
        }
        else {
            sender.text = String(format: "%.2f", Vigilance2)
        }
    }
    
    @IBAction func FrameFeatAvgBuffEdittingDidEnd(_ sender: UITextField) {
        let FeatAvgBuff: Int = Int(audioController.FeatAvgBufferLength)
        if let text: String = sender.text {
            if let newFeatAvgBuff: Int = Int(text) {
                if newFeatAvgBuff < 0 {
                    sender.text = String(format: "%d", FeatAvgBuff)
                }
                else {
                    audioController.update(FeatAvgBufferLength: newFeatAvgBuff)
                    sender.text = String(format: "%d", newFeatAvgBuff)
                }
            }
            else {
                sender.text = String(format: "%d", FeatAvgBuff)
            }
        }
        else {
            sender.text = String(format: "%d", FeatAvgBuff)
        }
    }
    
    
    @IBAction func NewClassCreationBuffEdittingDidEnd(_ sender: UITextField) {
        let newClassBuff: Float = Float(audioController.NewclusterCreationBuffer)
        if let text: String = sender.text {
            if let newNewClassBuff: Float = Float(text) {
                if newNewClassBuff < 0 {
                    sender.text = String(format: "%.2f", newClassBuff)
                }
                else {
                    audioController.update(NewclusterCreationBuffer: newNewClassBuff)
                    sender.text = String(format: "%.2f", newNewClassBuff)
                }
            }
            else {
                sender.text = String(format: "%.2f", newClassBuff)
            }
        }
        else {
            sender.text = String(format: "%.2f", newClassBuff)
        }
    }
    
    
    @IBAction func DecisionSmoothBuffEdidtingDidEnd(_ sender: UITextField) {
        let DecSmoothBuff: Float = Float(audioController.decisionSmoothingBuffer)
        if let text: String = sender.text {
            if let newDecSmoothBuff: Float = Float(text) {
                if newDecSmoothBuff < 0 {
                    sender.text = String(format: "%.2f", DecSmoothBuff)
                }
                else {
                    audioController.update(decisionSmoothingBuffer: newDecSmoothBuff)
                    sender.text = String(format: "%.2f", newDecSmoothBuff)
                }
            }
            else {
                sender.text = String(format: "%.2f", DecSmoothBuff)
            }
        }
        else {
            sender.text = String(format: "%.2f", DecSmoothBuff)
        }
    }
    
    
    @IBAction func switchHybridValueChanged(_ sender: UISwitch) {
        audioController.update(LoadingClassButton: sender.isOn)
        // Refresh with animation based on changed state
        sender.isOn = audioController.LoadingClassButton
    }
    
    @IBAction func switchSavingRepChanged(_ sender: UISwitch) {
        audioController.update(SavingClassButton: sender.isOn)
        // Refresh with animation based on changed state
        sender.isOn = audioController.SavingClassButton
    }
    
    @IBAction func switchSavingFeaturesChanged(_ sender: UISwitch) {
        audioController.update(SavingFeatButton: sender.isOn)
        // Refresh with animation based on changed state
        sender.isOn = audioController.SavingFeatButton
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
    }
    
    deinit {
        // Remove the notification
        NotificationCenter.default.removeObserver(self)
    }
    
    /// Dismisses the keyboard when the screen is tapped after editing a field.
    @objc func dismissKeyboard() {
        view.endEditing(true) // End editing on tap
    }
    
    @objc func appActiveAction(_ note: NSNotification) {
        if(self.isViewLoaded && self.view.window != nil) {
        }
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }


}

