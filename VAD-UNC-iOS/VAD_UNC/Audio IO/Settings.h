//
//  Settings.h
//  SPP Equalizer UI 1
//
//  Created by Akshay Chitale on 6/25/17.
//  Modified by Nasim Alamdari on 6/10/2018.
//  Copyright © 2017 UT Dallas. All rights reserved.
//

#import <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct Settings {
    // Core values
    int fs;
    int frameSize;
    int stepSize;
    //******************* unsupervised Noise Classification ***********
    float vigilanceVal1 ;
    float vigilanceVal2;
    int FeatAvgBufferLength ;
    float NewclusterCreationBuffer;
    float decisionSmoothingBuffer;
    int LoadingClassButton ;
    int SavingClassButton ;
    int SavingFeatButton ;
    //newSettings->ClassName                = "NoiseType"; // user will define the noise type
    //***************************************************************
    float dbpower;
    float calibration;
    float dbUpdateInterval; // How long to wait (in seconds) before updating dbpower
    // Audio status flags
    int micStatus;
    int playAudio;
    

} Settings;

Settings* newSettings();
void destroySettings(Settings* settings);

#ifdef __cplusplus
}
#endif
