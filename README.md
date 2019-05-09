# Unsupervised Background Noise Classification on Smartphones

This GitHub repository is the code accompaniment of the following paper:
> **A Real-Time Smartphone App for Unsupervised Noise Classification in Realistic Audio Environments**<br>
> Nasim Alamdari, and Nasser Kehtarnavaz - University of Texas at Dallas<br>
> https://ieeexplore.ieee.org/abstract/document/8662052<br>
>
> **Abstract:** This paper presents a real-time unsupervised noise classifier smartphone app which is designed to operate in realistic audio environments. This app addresses the two limitations of a previously developed smartphone app for unsupervised noise classification. A voice activity detection is added to separate the presence of speech frames from noise frames and thus to lower misclassifications when operating in realistic audio environments. In addition, buffers are added to allow a stable operation of the noise classifier in the field. The unsupervised noise classification is achieved by fusing the decisions of two adaptive resonance theory unsupervised classifiers running in parallel. One classifier operates on subband features and the other operates on mel- frequency spectral coefficients. The results of field testing indicate the effectiveness of this unsupervised noise classifier app when used in realistic audio environments.

## Resources

Supporting materials related to this work are available via the following links:

|**Link**|Description
|:-------|:----------
|https://ieeexplore.ieee.org/abstract/document/8662052| IEEE Manuscript
|http://www.utdallas.edu/%7Ekehtar/UnsupervisedNoiseClassifierApp-ART2Fusion.mp4| Videoclip showing the operation of the developed Unsupervised Noise Classifier smartphone app 


## Getting Started

A [User's Guide](Users-Guide-UnsupervisedNoiseClassificationArt2.pdf) is provided which describes how to run the codes of the Unsupervised Noise Classifier app on smartphone platforms.

## Requirement
1. To run the Android version of the Unsupervised Noise Classifier app, it is necessary to have Superpowered SDK which can be obtained from the following link: https://superpowered.com.
Then, need to add the path of Superpowered in gradle/local.properties:
    
        superpowered.dir = /.../SuperpoweredSDK/Superpowered

2. To run the iOS version of the Unsupervised Noise Classifier app, it is necessary to have Tensorflow C++ API for the Voice Activity Detection (VAD). The Tensorflow API can be downloaded or cloned from this link: https://www.tensorflow.org/install/


## License and Citation
The codes are licensed under MIT license.

For any utilization of the code content of this repository, the following paper needs to get cited by the user:

- N. Alamdari and N. Kehtarnavaz, “A Real-Time Smartphone App for Unsupervised Noise Classification in Realistic Audio Environments,” Proceedings of IEEE International Conference on Consumer Electronics, Las Vegas, NV, Jan 2019.
