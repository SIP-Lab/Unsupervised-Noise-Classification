//
//  audioProcessing.h
//  CNNVad
//
//  Created by Abhishek Sehgal on 3/14/17.
//  Copyright © 2017 axs145031. All rights reserved.
//

#ifndef audioProcessing_h
#define audioProcessing_h

#include <stdio.h>
#include "Transforms.h"
#include "MelSpectrogram.h"
#include "FIRFilter.h"
#include "Art2FusionClassifier.h"
#include "SubbandFeatures.h"
#include "Settings.h"

typedef struct Variables {
    
    FIR* downsampleFilter;
    Transform* fft;
    MelSpectrogram* melSpectrogram;
    
    float* inputBuffer;
    float* downsampled;
    float* decimated;
    float* frame;
    
    int samplingFrequency;
    int stepSize;
    int decimatedStepSize;
    
    //------ Classifer -------------
    int IsNoiseDetected;
    int ClusterLabel;
    int totalClusters;
    double frameNumber;
    int LoadingButton;
    int SavingButton;
    int SavingFeatButton;
    FILE* fileSavingFeat;
    //--------- Art2 Classifier --------------
    Art2FusionParallel* Art2Fusion;
    float vigilanceVal1;
    float vigilanceVal2;
    const char* className;
    //-------- Feature extraction ------------
    int count;
    int SubbandFeatureNum ;
    int  WaitingTime;
    SubbandFeatures* sbf;
    int FeatAvgBufferLength;
    int firstFrame;
    float* AllFeaturesVector;
    int AllFeaturesSize;
    float* sumBufferMFSC;
    //-----------------------------------------
    
} Variables;

Variables* initialize(int frequency, int stepsize, Settings* settings, const char *pathHybridDir, FILE *fileSavingFeat);
void compute(Variables* memoryPointer, short* input, int IsNoiseDetected);
void getMelImage(Variables* memoryPointer, float** melImage);
int getClusterLabel (Variables* memoryPointer); //added for noise classifier
int getTotalClusters(Variables* memoryPointer); //added for noise classifier
float getdbPower(Variables* memoryPointer);
#endif /* audioProcessing_h */
