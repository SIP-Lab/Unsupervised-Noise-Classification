#include <jni.h>
#include <stdlib.h>
#include <string.h>
#include <SuperpoweredFrequencyDomain.h>
#include <AndroidIO/SuperpoweredAndroidAudioIO.h>
#include <SuperpoweredSimple.h>
#include <SuperpoweredAdvancedAudioPlayer.h>
#include <SuperpoweredCPU.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_AndroidConfiguration.h>
#include <sstream>
#include <fstream>
#include <String>
#include <iterator>
#include <vector>
extern "C"{
#include "audioProcessing.h"
#include "Other/Timer.h"
}

static float *inputBufferFloat, *left, *right, *data;
static float **features; // Nasim: for saving features
SuperpoweredAdvancedAudioPlayer *audioPlayer;
bool outputEnabled;
const char *pathHybridDirSVDD; // for clustering
const char *pathHybridDirArt2; // for clustering
const char* audioFilePath;
const char* SaveFeatFilePath;
const char* className;
Variables* memoryPointer;
bool featureStore = false;
FILE* featureStoreFile;
Timer* timer;
SuperpoweredAndroidAudioIO *audioIO;

std::string Convert (float number){
    std::ostringstream buff;
    buff<<number;
    return buff.str();
}

std::string Convert2 (int number){
    std::ostringstream buff;
    buff<<number;
    return buff.str();
}


static void playerEventCallback(void * __unused clientData, SuperpoweredAdvancedAudioPlayerEvent event, void *value) {
    switch (event) {
        case SuperpoweredAdvancedAudioPlayerEvent_LoadSuccess: audioPlayer->play(false); break;
        case SuperpoweredAdvancedAudioPlayerEvent_LoadError: __android_log_print(ANDROID_LOG_DEBUG,
                                                                                 "frequencydomain",
                                                                                 "Open error: %s",
                                                                                 (char *)value); break;
        case SuperpoweredAdvancedAudioPlayerEvent_EOF: audioPlayer->togglePlayback(); break;
        default:;
    };
}

// This is called periodically by the media server.
static bool audioProcessing(void * __unused clientdata, short int *audioInputOutput, int numberOfSamples, int __unused samplerate) {
    start(timer);
    SuperpoweredShortIntToFloat(audioInputOutput, inputBufferFloat, (unsigned int)numberOfSamples); // Converting the 16-bit integer samples to 32-bit floating point.
    SuperpoweredDeInterleave(inputBufferFloat, left, right, (unsigned int)numberOfSamples);
    compute(memoryPointer, left);
    stop(timer);

    //Storing Features
    if(featureStore){
        std::ofstream out;
        out.open(SaveFeatFilePath,std::ios::app);
        copyArray(memoryPointer,features);
        for (int i = 0; i < getRowElements(memoryPointer); i++) {
            for (int j = 0; j < getColElements(memoryPointer)-2; j++) {
                out << Convert(features[i][j]);
                out << ",";
            }
            for (int j = getColElements(memoryPointer)-2; j < getColElements(memoryPointer); j++) {
                out << Convert2((int)features[i][j]);
                out << ",";
            }
            out << ";\n";
        }
        out << "\n";
        out.close();
    }
    return true;
}

static bool audioFileProcessing(void * __unused clientdata, short int *audioInputOutput, int numberOfSamples, int __unused samplerate){

    if (audioPlayer->process(inputBufferFloat, false, (unsigned int) numberOfSamples)){
        start(timer);
        SuperpoweredFloatToShortInt(inputBufferFloat, audioInputOutput, (unsigned int)numberOfSamples);
        SuperpoweredDeInterleave(inputBufferFloat, left, right, (unsigned int)numberOfSamples);
        compute(memoryPointer, left);
        stop(timer);

        //Storing Features
        if(featureStore){
            std::ofstream out;
            out.open(SaveFeatFilePath,std::ios::app);
            copyArray(memoryPointer,features);
            for (int i = 0; i < getRowElements(memoryPointer); i++) {
                for (int j = 0; j < getColElements(memoryPointer)-2; j++) {
                    out << Convert(features[i][j]);
                    out << ",";
                }
                for (int j = getColElements(memoryPointer)-2; j < getColElements(memoryPointer); j++) {
                    out << Convert2((int)features[i][j]);
                    out << ",";
                }
                out << ";\n";
            }
            out << "\n";
            out.close();
        }

        if(outputEnabled){
            return true;
        }
        else {
            return false;
        }

    } else return false;

}

extern "C" JNIEXPORT void Java_com_superpowered_frequencydomain_MainActivity_FrequencyDomain(JNIEnv * __unused javaEnvironment,
                                                                                             jobject __unused obj,
                                                                                             jint samplerate,
                                                                                             jint buffersize,
                                                                                             jint IsNoiseDetected,
                                                                                             jstring HybridSVDDFileName,
                                                                                             jstring HybridArt2FileName,
                                                                                             jstring StoreFeaturesFileName,
                                                                                             jfloat decisionSmoothingBuffer,
                                                                                             jint NewclusterCreationBuffer,
                                                                                             jint FeatAvgBufferLength,
                                                                                             jboolean HybridButton,
                                                                                             jboolean SavingClassifDataButton,
                                                                                             jboolean storeFeaturesButton,
                                                                                             jfloat VigilanceVal1,
                                                                                             jfloat VigilanceVal2) {
    inputBufferFloat = (float *)malloc(buffersize * sizeof(float) * 2 + 128);
    left             = (float *)malloc(buffersize * sizeof(float) + 64);
    right            = (float *)malloc(buffersize * sizeof(float) + 64);
    data             = (float*)malloc(sizeof(float)*1600);


    if ( ((int) HybridButton == 1) |  ((int) SavingClassifDataButton == 1) ) {
    pathHybridDirArt2 = javaEnvironment->GetStringUTFChars(HybridArt2FileName, NULL); // convert jstring to char*
    }

    memoryPointer = initialize(samplerate, buffersize, IsNoiseDetected, pathHybridDirArt2, decisionSmoothingBuffer, NewclusterCreationBuffer,FeatAvgBufferLength,(int) HybridButton, (int) SavingClassifDataButton, VigilanceVal1,VigilanceVal2, className);
    timer = newTimer();

    featureStore = storeFeaturesButton; //boolean StoreFeatures
    if (storeFeaturesButton) {
        SaveFeatFilePath = javaEnvironment->GetStringUTFChars(StoreFeaturesFileName,JNI_FALSE);
        features = (float **)calloc((size_t) getRowElements(memoryPointer), sizeof(float*));
        for ( size_t i = 0; i < getRowElements(memoryPointer) ; i++){
            features[i] = (float *)malloc( (size_t)getColElements(memoryPointer) * sizeof(float));}
    }
    SuperpoweredCPU::setSustainedPerformanceMode(true);
    audioIO = new SuperpoweredAndroidAudioIO(samplerate, buffersize, true, true, audioProcessing, NULL, -1, SL_ANDROID_STREAM_MEDIA, buffersize * 2); // Start audio input/output.
}

extern "C" JNIEXPORT void Java_com_superpowered_frequencydomain_MainActivity_ReadFile(JNIEnv *javaEnvironment,
                                                                                jobject __unused obj,
                                                                                jint samplerate,
                                                                                jint buffersize,
                                                                                jint IsNoiseDetected,
                                                                                jstring HybridSVDDFileName,
                                                                                jstring HybridArt2FileName,
                                                                                jstring StoreFeaturesFileName,
                                                                                jfloat decisionSmoothingBuffer,
                                                                                jint  NewclusterCreationBuffer,
                                                                                jint FeatAvgBufferLength,
                                                                                jboolean HybridButton,
                                                                                jboolean SavingClassifDataButton,
                                                                                jboolean storeFeaturesButton,
                                                                                jfloat VigilanceVal1,
                                                                                jfloat VigilanceVal2,
                                                                                jstring audioFileName) {
    inputBufferFloat = (float *)malloc(buffersize * sizeof(float) * 2 + 128);
    left             = (float *)malloc(buffersize * sizeof(float) + 64);
    right            = (float *)malloc(buffersize * sizeof(float) + 64);
    data             = (float*)malloc(sizeof(float)*1600);


    if ( ((int) HybridButton == 1) |  ((int) SavingClassifDataButton == 1) ) {
        pathHybridDirArt2 = javaEnvironment->GetStringUTFChars(HybridArt2FileName, NULL); // convert jstring to char*
    }

    memoryPointer = initialize(samplerate, buffersize, IsNoiseDetected,pathHybridDirArt2, decisionSmoothingBuffer, NewclusterCreationBuffer, FeatAvgBufferLength,(int) HybridButton, (int) SavingClassifDataButton, VigilanceVal1,VigilanceVal2, className);
    timer = newTimer();

    featureStore = storeFeaturesButton; //boolean StoreFeatures
    if (storeFeaturesButton) {
        SaveFeatFilePath = javaEnvironment->GetStringUTFChars(StoreFeaturesFileName,JNI_FALSE);
        features = (float **)calloc((size_t) getRowElements(memoryPointer), sizeof(float*));
        for ( size_t i = 0; i < getRowElements(memoryPointer) ; i++){
            features[i] = (float *)malloc( (size_t)getColElements(memoryPointer) * sizeof(float));}
    }
    //SuperpoweredCPU::setSustainedPerformanceMode(true);
    //audioIO = new SuperpoweredAndroidAudioIO(samplerate, buffersize, true, true, audioProcessing, NULL, -1, SL_ANDROID_STREAM_MEDIA, buffersize * 2); // Start audio input/output.

    outputEnabled = true; //playAudio;
    audioPlayer = new SuperpoweredAdvancedAudioPlayer(NULL, playerEventCallback, (unsigned int)samplerate, 0);
    audioIO = new SuperpoweredAndroidAudioIO(samplerate, buffersize, false, true, audioFileProcessing, javaEnvironment, -1, SL_ANDROID_STREAM_MEDIA, buffersize * 2); // Start audio input/output.
    audioFilePath = javaEnvironment->GetStringUTFChars(audioFileName, JNI_FALSE);
    audioPlayer->open(audioFilePath);


}

extern "C" JNIEXPORT jfloatArray Java_com_superpowered_frequencydomain_MainActivity_GetMelImage(JNIEnv * __unused javaEnvironment, jobject __unused obj){
    jfloatArray result;
    result = javaEnvironment->NewFloatArray(1600);


    for (int i = 0; i < 40; i++) {
        for (int j = 0; j < 40; j++) {
            data[40*i+j] = memoryPointer->melSpectrogram->melSpectrogramImage[i][j];
        }
    }

    javaEnvironment->SetFloatArrayRegion(result, 0, 1600, data);
    return result;
}

extern "C" JNIEXPORT float Java_com_superpowered_frequencydomain_MainActivity_getProcessingTime(JNIEnv * __unused javaEnvironment, jobject __unused obj){
    return getMS(timer);
}

extern "C" JNIEXPORT void Java_com_superpowered_frequencydomain_MainActivity_StopAudio(JNIEnv * javaEnvironment, jobject __unused obj, jstring StoreFeaturesFileName){

    if(inputBufferFloat != NULL){
        delete audioIO;
        free(inputBufferFloat);
        free(left);
        free(right);
        inputBufferFloat = NULL;
        if(featureStore) {
            javaEnvironment->ReleaseStringUTFChars(StoreFeaturesFileName, SaveFeatFilePath); // javaEnvironment->ReleaseStringUTFChars(JavaString,nativeString );
        }
    }
}

extern "C" JNIEXPORT float Java_com_superpowered_frequencydomain_MainActivity_getdbPower(JNIEnv * __unused javaEnvironment, jobject __unused obj) {
    return getdbPower(memoryPointer);
}

extern "C" JNIEXPORT int Java_com_superpowered_frequencydomain_MainActivity_getClusterLabel(JNIEnv * __unused javaEnvironment, jobject __unused obj) {
    return getClusterLabel(memoryPointer);
}

extern "C" JNIEXPORT int Java_com_superpowered_frequencydomain_MainActivity_getTotalClusters(JNIEnv * __unused javaEnvironment, jobject __unused obj) {
    return getTotalClusters(memoryPointer);
}