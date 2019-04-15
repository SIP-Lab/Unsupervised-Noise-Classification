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
#define FREQLOW             0 //300
#define FREQHIGH            8000
#define DECIMATION_FACTOR   3
#define EPS 1.0e-7
#define S2F 3.051757812500000e-05f
#define F2S 32768
static int nBands = 4;
FILE *file2; // for clustering
//static int* smoothingBuffer;
//static int smoothingBufferLength;

Variables* initialize(int frequency, int stepsize, int IsNoiseDetected,
                      const char* pathHybridDirArt2, int decisionSmoothingBuffer,
                      int NewclusterCreationBuffer, int FeatAvgBufferLength,int HybridButton,
                      int SavingClassifDataButton, float VigilanceVal1,float VigilanceVal2,  const char* className) {
    
    Variables* inParam = (Variables*) malloc(sizeof(Variables));
    
    inParam->stepSize = stepsize;
    inParam->decimatedStepSize = stepsize/DECIMATION_FACTOR;
    inParam->samplingFrequency = frequency/DECIMATION_FACTOR;
    
    inParam->inputBuffer        = (float*)calloc(stepsize, sizeof(float));
    inParam->downsampled        = (float*)calloc(stepsize, sizeof(float));
    inParam->decimated          = (float*)calloc(2*inParam->decimatedStepSize, sizeof(float));
    
    inParam->fft                = newTransform(2*inParam->decimatedStepSize, (int)(frequency/stepsize));
    inParam->melSpectrogram     = initMelSpectrogram(NFILT, FREQLOW, FREQHIGH, 2*inParam->decimatedStepSize, inParam->samplingFrequency, inParam->fft->points);
    
    inParam->downsampleFilter   = initFIR(stepsize);

    // ----------- Feature extraction Initialization------------------------------------------------------------
    int i;
    inParam->decisionBufferLength  = FeatAvgBufferLength; //decisionBufferLength;
    inParam->count                 = 0;
    inParam->SubbandFeatureNum     = (int)2* nBands;
    inParam->firstFrame            = 1; // for computing subband features
    inParam->warmUp                = inParam->decisionBufferLength; //decisionBufferLength;

    inParam->sbf                   = initSubbandFeatures(inParam->fft->points, inParam->decimatedStepSize, inParam->decisionBufferLength); // initialize SubbandFeatures
    inParam->AllFeaturesSize       = (40)+inParam->SubbandFeatureNum;
    inParam->AllFeaturesVector     = (float*)calloc(inParam->AllFeaturesSize ,sizeof(float));// 40 is size of filters in Log Mel
    inParam->sumBufferMFSC         = (float*)calloc(40, sizeof(float));

    // -------- Classifier Parameters and initialization ---------------------------------
    inParam->IsNoiseDetected       = IsNoiseDetected;
    inParam->LoadingButton         = HybridButton;//LoadingButton;     // for activating or disactivating hybrid classification
    inParam->SavingButton          = SavingClassifDataButton;//SavingButton;   //for saving detected clusters for future use of hybrid classification

    //----------- Classification resutls ------------------------------------------------------
    inParam-> ClusterLabel         = 0;
    inParam-> totalClusters        = 0;
    inParam-> frameNumber          = 0;
    //------------------------------------------------------------------------------------------

    //--------- Art2 Classification -----------------------------------------------------------
    inParam->className             = className;
    inParam->vigilanceVal1         = VigilanceVal1; //vigilanceVal;
    inParam->vigilanceVal2         = VigilanceVal2; //vigilanceVal;
    inParam->Art2Fusion            = initArt2ParallelFusion(inParam->SavingButton,
                                                            inParam->LoadingButton,
                                                            inParam->vigilanceVal1,  // vigilance_1
                                                            inParam->vigilanceVal2, // vigilance_2
                                                            pathHybridDirArt2,
                                                            8, //FeatureNum_1 which is number of subband features
                                                            40, // FeatureNum_2 which is number of MSFC feature
                                                            NewclusterCreationBuffer, //newClusterCreationBuffer
                                                            decisionSmoothingBuffer, // DecisionSmoothingBuffer
                                                            10); // MaxClusterCreation

    //-----------------------------------------------------------------------------------------
    return inParam;
}

void compute(Variables* memoryPointer, float* input) {
    Variables* inParam = memoryPointer;
    
    int i, j;
    float quiet = 69; // you can add this as parameter to the GUI
    
//    for (i = 0; i < inParam->stepSize; i++) {
//        inParam->inputBuffer[i] = input[i];
//    }
    // Downsample the audio
    processFIRFilter(inParam->downsampleFilter, input, inParam->downsampled);
    
    // Decimate the audio
    for (i = 0, j = 0; i < inParam->decimatedStepSize; i++, j+= 3) {
        inParam->decimated[i] = inParam->decimated[i+inParam->decimatedStepSize];
        inParam->decimated[i+inParam->decimatedStepSize] = inParam->downsampled[j];
    }
    
    ForwardFFT(inParam->fft, inParam->decimated);
    updateImage(inParam->melSpectrogram, inParam->fft->power); // this needs for VAD

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
        if (inParam->count > inParam->warmUp -1 ) {


            for (i= 0 ; i < inParam->SubbandFeatureNum; i++) {
                inParam->AllFeaturesVector[i] = inParam->sbf->subbandFeatureList[i];
            }
            int index = 0;
            for (i= inParam->SubbandFeatureNum; i < inParam->AllFeaturesSize; i++) {
                inParam->AllFeaturesVector[i] = inParam->sumBufferMFSC[i]/inParam->count;
                inParam->sumBufferMFSC[index] = 0;
                index++;
            }
            computeArt2ParallelFusion(inParam->Art2Fusion,inParam->AllFeaturesVector, inParam->className);

            inParam-> ClusterLabel = inParam->Art2Fusion->detectedClass;
            inParam->totalClusters = inParam->Art2Fusion->TotalDetectedClass;


            //inParam-> frameNumber = inParam->Clustering_ptr->frameNumber;
            inParam->count = 0;
        }

        //-------------------------------------------------------------

        }
    }
}

void getMelImage(Variables* memoryPointer, float** melImage){
    Variables* inParam = memoryPointer;
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

void copyArray(Variables* memoryPointer, float** array) {
    Variables* inParam = (Variables*)memoryPointer;

    int nRows, nCols;

    nRows = 1;
    nCols = inParam->AllFeaturesSize;

    array = (float *) malloc(1 * sizeof(float *));
    for (int i = 0; i < 1; i++)
        array[i] = (float *) malloc((nCols + 3) * sizeof(float));

    int i;
    for (i = 0; i< inParam->AllFeaturesSize; i++) {
        array[0][i] = inParam->AllFeaturesVector[i];
    }

    array[0][nCols]     = inParam->fft->dbpower;
    array[0][nCols + 1] = inParam->ClusterLabel;  //added for clustering results
    array[0][nCols + 2] = inParam->totalClusters;  //added for clustering results


}

int getColElements(Variables* memoryPointer) {
    Variables* inParam = (Variables*)memoryPointer;
    return inParam->AllFeaturesSize+3;
}

int getRowElements(Variables* memoryPointer) {
    Variables* inParam = (Variables*)memoryPointer;
    return 1;
}
