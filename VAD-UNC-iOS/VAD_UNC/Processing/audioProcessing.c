//
//  audioProcessing.c
//  CNNVad
//
//  Created by Abhishek Sehgal on 3/14/17.
//  Copyright © 2017 axs145031. All rights reserved.
//

#include "audioProcessing.h"
#define SHORT2FLOAT         1/32768.0
#define FLOAT2SHORT         32768.0
#define NFILT               40
#define FREQLOW             300
#define FREQHIGH            8000
#define DECIMATION_FACTOR   3
#define EPS 1.0e-7
#define S2F 3.051757812500000e-05f
#define F2S 32768

Variables* initialize(int frequency, int stepsize, Settings* settings, const char *pathHybridDir, FILE *fileSavingFeat) {
    
    Variables* inParam = (Variables*) malloc(sizeof(Variables));
    
    inParam->stepSize = stepsize;
    inParam->decimatedStepSize = stepsize/DECIMATION_FACTOR;
    inParam->samplingFrequency = frequency/DECIMATION_FACTOR;
    
    inParam->inputBuffer = (float*)calloc(stepsize, sizeof(float));
    inParam->downsampled = (float*)calloc(stepsize, sizeof(float));
    inParam->decimated   = (float*)calloc(2*inParam->decimatedStepSize, sizeof(float));
    
    inParam->fft = newTransform(2*inParam->decimatedStepSize, (int)(frequency/stepsize));
    inParam->melSpectrogram = initMelSpectrogram(NFILT, FREQLOW, FREQHIGH, 2*inParam->decimatedStepSize, inParam->samplingFrequency, inParam->fft->points);
    
    inParam->downsampleFilter = initFIR(stepsize);
    
    //// ----------- Feature extraction Initialization-----------------------------------------
    
    inParam->FeatAvgBufferLength  = settings->FeatAvgBufferLength;//FeatAvgBufferLength;
    inParam->count                 = 0;
    inParam->SubbandFeatureNum     = (int)2* 4;//nBands;
    inParam->firstFrame            = 1; // for computing subband features
    inParam->WaitingTime                = inParam->FeatAvgBufferLength; //decisionBufferLength;
    
    inParam->sbf                   = initSubbandFeatures(inParam->fft->points, inParam->decimatedStepSize, inParam->FeatAvgBufferLength); // initialize SubbandFeatures
    inParam->AllFeaturesSize       = (40)+inParam->SubbandFeatureNum;
    inParam->AllFeaturesVector     = (float*)calloc(inParam->AllFeaturesSize ,sizeof(float));// 40 is size of filters in Log Mel
    inParam->sumBufferMFSC         = (float*)calloc(40, sizeof(float));
    
    // -------- Classifier Parameters and initialization ---------------------------------
    inParam->IsNoiseDetected       = 1;//IsNoiseDetected;
    inParam->LoadingButton         = settings->LoadingClassButton;//HybridButton;//LoadingButton;     // for turning on and off the hybrid classification
    inParam->SavingButton          = settings->SavingClassButton;//SavingClassifDataButton;//SavingButton;   //for saving detected clusters for future use of hybrid classification
    inParam->SavingFeatButton      = settings->SavingFeatButton;
    inParam->fileSavingFeat        = fileSavingFeat;
    //----------- Classification resutls ------------------------------------------------------
    inParam-> ClusterLabel         = 0;
    inParam-> totalClusters        = 0;
    //------------------------------------------------------------------------------------------
    
    //--------- Art2 Classification -----------------------------------------------------------
    //inParam->className             = className;
    inParam->vigilanceVal1         = settings->vigilanceVal1;
    inParam->vigilanceVal2         = settings->vigilanceVal2;
    inParam->Art2Fusion            = initArt2ParallelFusion(inParam->SavingButton,
                                                            inParam->LoadingButton,
                                                            inParam->vigilanceVal1,  // vigilance_1
                                                            inParam->vigilanceVal2, // vigilance_2
                                                            pathHybridDir,//pathHybridDirArt2,
                                                            8, //FeatureNum_1 which is number of subband features
                                                            40, // FeatureNum_2 which is number of MSFC feature
                                                            (1000 * settings->NewclusterCreationBuffer)/(12.5*inParam->FeatAvgBufferLength), //newClusterCreationBuffer
                                                            (1000 * settings->decisionSmoothingBuffer)/(12.5*inParam->FeatAvgBufferLength), // DecisionSmoothingBuffer
                                                            10); // MaxClusterCreation
    
    //-----------------------------------------------------------------------------------------
    
    return inParam;
}

void compute(Variables* memoryPointer, short* input, int IsNoiseDetected) {
    Variables* inParam = (Variables*) memoryPointer;
    
    int i, j;
    
    for (i = 0; i < inParam->stepSize; i++) {
        inParam->inputBuffer[i] = input[i] * S2F;
    }
    // Downsample the audio
    processFIRFilter(inParam->downsampleFilter, inParam->inputBuffer, inParam->downsampled);
    
    // Decimate the audio
    for (i = 0, j = 0; i < inParam->decimatedStepSize; i++, j+= 3) {
        inParam->decimated[i] = inParam->decimated[i+inParam->decimatedStepSize];
        inParam->decimated[i+inParam->decimatedStepSize] = inParam->downsampled[j];
    }
    
    ForwardFFT(inParam->fft, inParam->decimated);
    updateImage(inParam->melSpectrogram, inParam->fft->power);
    
    // ##################### Integrating Noise Classifier Here ############################
    float quiet = 65; // you can later on add this to the GUI
    inParam->IsNoiseDetected = IsNoiseDetected;
    if (inParam->IsNoiseDetected == 1){ // perform noise classification
        if (inParam->fft->dbpower > (quiet)) {
            //--------- Feature Extraction ------------------------------
            
            // Extract subband features
            computeSubbandFeatures(inParam->sbf, inParam->fft->power, inParam->firstFrame);
            inParam->firstFrame = 0;
            
            melCalculate(inParam->fft->power, inParam->melSpectrogram->nFFT, inParam->melSpectrogram->filtBank, inParam->melSpectrogram->nFilt, inParam->melSpectrogram->melPower);
            for ( i = 0; i < 40; i++)
            {
                inParam->sumBufferMFSC[i] = inParam->sumBufferMFSC[i] + inParam->melSpectrogram->melPower[i];
            }
            
            //--------- Unsupervised noise classification -------------------
            
            inParam->count++;
            
            //When the count is more than the decision buffer length
            //start warmUp condition
            if (inParam->count > inParam->WaitingTime -1 ) {
                
                
                for (i= 0 ; i < inParam->SubbandFeatureNum; i++) {
                    inParam->AllFeaturesVector[i] = inParam->sbf->subbandFeatureList[i];
                }
                int index = 0;
                for (i= inParam->SubbandFeatureNum; i < inParam->AllFeaturesSize; i++) {
                    inParam->AllFeaturesVector[i] = inParam->sumBufferMFSC[i]/inParam->count;
                    inParam->sumBufferMFSC[index] = 0;
                    index++;
                }
                
                // -------------- Saving the features -----------------
                if (inParam->SavingFeatButton == 1){
                    if (inParam->fileSavingFeat != NULL) {
                            for (int i = 0; i < inParam->AllFeaturesSize; i++){
                                fprintf(inParam->fileSavingFeat, "%.2f,", inParam->AllFeaturesVector[i] );
                            }
                            fprintf(inParam->fileSavingFeat,";\n");
                    }
                    else{
                        printf("\n Error! Can not open the file for writing \n");
                    }
                }
                //-------------------------------------------------------
                
                computeArt2ParallelFusion(inParam->Art2Fusion,inParam->AllFeaturesVector, inParam->className);
                
                inParam-> ClusterLabel = inParam->Art2Fusion->detectedClass;
                inParam->totalClusters = inParam->Art2Fusion->TotalDetectedClass;
                
                
                //inParam-> frameNumber = inParam->Clustering_ptr->frameNumber;
                inParam->count = 0;
            }
            
            //-------------------------------------------------------------
            
        }
    }
    //#######################################################################################
}

void getMelImage(Variables* memoryPointer, float** melImage){
    Variables* inParam = (Variables*) memoryPointer;
    for (size_t i = 0; i < NFILT; i++) {
        for (size_t j = 0; j < NFILT; j++) {
            melImage[i][j] = inParam->melSpectrogram->melSpectrogramImage[i][j];
        }
    }
}

int getClusterLabel (Variables* memoryPointer){
    Variables* inParam = (Variables*)memoryPointer;
    return inParam->ClusterLabel;
}

int getTotalClusters(Variables* memoryPointer){
    Variables* inParam = (Variables*)memoryPointer;
    return inParam->totalClusters;
}

float getdbPower(Variables* memoryPointer) {
    Variables* inParam = (Variables*)memoryPointer;
    return inParam->fft->dbpower;
}





































