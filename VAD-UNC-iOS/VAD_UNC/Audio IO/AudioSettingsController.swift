//
//  AudioSettingsController.swift
//  SPP Equalizer UI 1
//
//  Created by Akshay Chitale on 6/25/17.
//  Modified by Nasim Alamdari on 3/23/2019
//  Copyright © 2017 UT Dallas. All rights reserved.
//

import Foundation

/// Globally availible audio settings, set in `AppDelegate.swift` on app launch and for use in all view controllers.
var audioController: AudioSettingsController!

/// A controller for starting and stopping the audio, as well as changing the audio settings.
///
/// This class serves as an interface for Swift code to interact with the underlying Objective-C and C audio code.
class AudioSettingsController {
    // Later have private var for IosAudioController, for now is global
    
    
    
    
    
    /// Initializes the `AudioSettingsController` with "Low", "Medium", and "High" gains and default settings.
    init() {
        audioRecorder = AudioRecorder() // Is global for now, make local later
        
    }
    /// Initializes the `AudioSettingsController` with an array of current gains.
    ///
    
    deinit {
        // Deallocate memory
        audioRecorder.destroySettings()
    }
    
    
    /// The current frame size, a.k.a. the window size.
    var frameSize: Int {
        return Int(audioRecorder.settings.pointee.frameSize)
    }
    
    /// The current step size, a.k.a. the overlap size.
    var stepSize: Int {
        return Int(audioRecorder.settings.pointee.stepSize)
    }
    
    /// vigilance value for ART2 using subband features.
    var vigilanceVal1: Float {
        return audioRecorder.settings.pointee.vigilanceVal1
    }
    
    /// vigilance value for ART2 using MFSC features.
    var vigilanceVal2: Float {
        return audioRecorder.settings.pointee.vigilanceVal2
    }
    
    
    /// number of frames in feature averaging buffer
    var FeatAvgBufferLength: Int {
        return Int(audioRecorder.settings.pointee.FeatAvgBufferLength)
    }
    
    /// time for creating new class in ART2 fusion (in seconds)
    var NewclusterCreationBuffer: Float {
        return audioRecorder.settings.pointee.NewclusterCreationBuffer
    }
    
    /// decision smoothing buffer (in seconds)
    var decisionSmoothingBuffer: Float {
        return audioRecorder.settings.pointee.decisionSmoothingBuffer
    }
    
    //if(self.autoGains) {}
    /// Whether the Reading otions are being selected.
    var LoadingClassButton: Bool {
        return audioRecorder.settings.pointee.LoadingClassButton != 0 ? true : false
    }
    
    /// Whether the saving otions are being selected.
    var SavingClassButton: Bool {
        return audioRecorder.settings.pointee.SavingClassButton != 0 ? true : false
    }
    
    /// Whether the saving features option are being selected.
    var SavingFeatButton: Bool {
        return audioRecorder.settings.pointee.SavingFeatButton != 0 ? true : false
    }
    
    /// The user will define the noise type
    //var UserDefinedNoiseType: String {
    //    return String(describing: iosAudio.settings.pointee.UserDefinedNoiseType)
    //}
    
    
    /// The current microphone status. Is using audio input from the microphone if true, or from a file if false.
    var micStatus: Bool {
        return audioRecorder.settings.pointee.micStatus != 0 ? true : false
    }
    
    /// Whether audio is currently playing.
    var playAudio: Bool {
        return audioRecorder.settings.pointee.playAudio != 0 ? true : false
    }
    
    
    /// The audio level, in dB SPL.
    var dbpower: Float {
        return audioRecorder.settings.pointee.dbpower
    }
    
    
    /// The interval to wait before updating the audio level, in seconds.
    var dbUpdateInterval: Float {
        return audioRecorder.settings.pointee.dbUpdateInterval
    }
    
  
    
    
    /// Updates the sampling frequency.
    ///
    
    /// Updates the frame size, a.k.a. the window size.
    ///
    /// - Parameter frameSize: The new frame size, in milliseconds.
    func update(frameSize: Float) {
        audioRecorder.settings.pointee.frameSize = Int32(frameSize * Float(48000) / 1000.0)
    }
    
    /// Updates the step size, a.k.a. the overlap size.
    ///
    /// - Parameter stepSize: The new step size, in milliseconds.
    func update(stepSize: Float) {
        audioRecorder.settings.pointee.stepSize = Int32(stepSize * Float(48000) / 1000.0)
    }
    
    func update(vigilanceVal1 : Float){
        audioRecorder.settings.pointee.vigilanceVal1 = Float(vigilanceVal1)
    }
    
    func update(vigilanceVal2 : Float){
        audioRecorder.settings.pointee.vigilanceVal2 = Float(vigilanceVal2)
    }
    
    func update(FeatAvgBufferLength:Int)
    {
        audioRecorder.settings.pointee.FeatAvgBufferLength = Int32(FeatAvgBufferLength)
    }
    
    func update (NewclusterCreationBuffer:Float){
        audioRecorder.settings.pointee.NewclusterCreationBuffer = Float(NewclusterCreationBuffer)
    }
    
    func update (decisionSmoothingBuffer:Float){
        audioRecorder.settings.pointee.decisionSmoothingBuffer = Float(decisionSmoothingBuffer)
    }
    
    /// Whether the Reading otions are being selected.
    func update (LoadingClassButton: Bool) {
        audioRecorder.settings.pointee.LoadingClassButton = Int32(LoadingClassButton ? 1 : 0)
    }
    
    /// Whether the saving otions are being selected.
    func update (SavingClassButton: Bool) {
        audioRecorder.settings.pointee.SavingClassButton = Int32(SavingClassButton ? 1 : 0)
    }
    
    /// Whether the saving features option are being selected.
    func update (SavingFeatButton: Bool) {
        audioRecorder.settings.pointee.SavingFeatButton = Int32(SavingFeatButton ? 1 : 0)
    }
    
    /// Updates the noise type given by the user.
    /// - Parameter fileName: The name of the noise type,
    //func update(UserDefinedNoiseType: String) {
        // Safe to just point, since inner code never mutates C string
    //    iosAudio.settings.pointee.UserDefinedNoiseType = UnsafePointer<Int8>(UserDefinedNoiseType)
    //}
    
    
    /// Updates the microphone status.
    ///
    /// - Parameter micStatus: If true, use microphone audio input. If false, use audio input from file.
    func update(micStatus: Bool) {
        audioRecorder.settings.pointee.micStatus = Int32(micStatus ? 1 : 0)
    }
    
    

    
    /// Updates the interval to wait before updating the audio level.
    ///
    /// - Parameter dbUpdateInterval: The new interval to wait, in seconds.
    func update(dbUpdateInterval: Float) {
        audioRecorder.settings.pointee.dbUpdateInterval = dbUpdateInterval
    }
    
    
}
