//
//  AudioRecorder.h
//  CNN_VAD
//
//  Created by Abhishek Sehgal on 9/22/17.
//  Modified by Nasim Alamdari on 3/23/2019
//  Copyright © 2017 SIPLab. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AudioUnit/AudioUnit.h>
#import "MovingAverageBuffer.h"
#import "Settings.h"



@interface AudioRecorder : NSObject {
    AudioUnit au;
    MovingAverageBuffer* timeBuffer;
    float speechPrediction;
    Settings* settings; // holds previously global settings
    int clusterLabel;
    int totalDetectedClusters;
}
@property AudioUnit au;
@property MovingAverageBuffer* timeBuffer;
@property MovingAverageBuffer* predictBuffer;
@property float speechPrediction;
@property (readwrite) Settings* settings;
@property int clusterLabel; // For noise classifier
@property int totalDetectedClusters; // For noise classifier
@property int IsNoiseDetected;   // For noise classifier
@property float dbpower;

- (void) predict;
- (void) processAudio;
- (void) start;
- (void) stop;
- (void) destroySettings;

@end

extern AudioRecorder* audioRecorder;
