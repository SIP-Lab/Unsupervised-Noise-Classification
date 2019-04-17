//
//  Settings.c
//
//
//  Created by Akshay Chitale on 6/25/17.
//  Modified by Nasim Alamdari on 6/10/2018.
//  Copyright © 2017 UT Dallas. All rights reserved.
//

#import "Settings.h"

Settings* newSettings() {
    
    Settings* newSettings = (Settings*)malloc(sizeof(Settings));
    
    // Set defaults
    newSettings->fs =  48000;
    newSettings->frameSize = 1200; //with 50% overlapp
    newSettings->stepSize = 600; // overlapp size
    
    ///***************** unsupervised Noise Classifier ***********
    newSettings->vigilanceVal1            = 0.01;
    newSettings->vigilanceVal2            = 0.01;
    newSettings->FeatAvgBufferLength      = 80;
    newSettings->NewclusterCreationBuffer = 3;
    newSettings->decisionSmoothingBuffer  = 3;
    newSettings->LoadingClassButton       = 0; // this is for hybrid option, default is for loading previously detected clusters
    newSettings->SavingClassButton        = 0; //  default is for saving
    newSettings->SavingFeatButton         = 0; //saving feature extractions
    //const char* ClassName;
    //*************************************************************
    newSettings->micStatus = 1;
    newSettings->playAudio = 0;
    newSettings->dbpower = 0; // needed for main view
    newSettings->calibration = -93.9794; // needed for main view
    newSettings->dbUpdateInterval = 1.0; // needed for dBpower update
    
    
    return newSettings;
}

void destroySettings(Settings* settings) {
    if(settings != NULL){
        free(settings);
        settings = NULL;
    }
}
