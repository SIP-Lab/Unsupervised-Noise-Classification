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
#include <math.h>
#include <stdlib.h>
#include "Other/Transforms.h"
#include "Feature Extraction/MelSpectrogram.h"
#include "Other/FIRFilter.h"
#include "Feature Extraction/SubbandFeatures.h"
#include "Art2Classifier/Art2FusionClassifier.h"



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
    //--------- Art2 Classifier --------------
    Art2FusionParallel* Art2Fusion;
    float vigilanceVal1;
    float vigilanceVal2;
    const char* className;
    //-------- Feature extraction ------------
    int count;
    int SubbandFeatureNum ;
    int  warmUp;
    SubbandFeatures* sbf;
    int decisionBufferLength;
    int firstFrame;
    float* AllFeaturesVector;
    int AllFeaturesSize;
    float* sumBufferMFSC;
    //-----------------------------------------

} Variables;

Variables* initialize(int frequency, int stepsize, int IsNoiseDetected, const char* pathHybridDirArt2,
                      int decisionSmoothingBuffer, int NewclusterCreationBuffer, int FeatAvgBufferLength,
                      int HybridButton, int SavingClassifDataButton, float VigilanceVal1,float VigilanceVal2,  const char* className);
void compute(Variables* memoryPointer, float* input);
void getMelImage(Variables* memoryPointer, float** melImage);
float getdbPower(Variables* memoryPointer);
int getClusterLabel (Variables* memoryPointer); //added for clustering
int getTotalClusters(Variables* memoryPointer); //added for lcustering
void copyArray(Variables* memoryPointer, float ** array);
int getColElements(Variables* memoryPointer);
int getRowElements(Variables* memoryPointer);

#endif /* audioProcessing_h */
