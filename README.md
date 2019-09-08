# Introduction [中文][chinese]

<p align="center">
  <img src="https://github.com/VRTRIX/VRTRIXGlove_Unity3D_SDK/blob/master/docs/img/digital_glove.png"/>
</p>


VRTRIX Data Glove is a product based on high accuracy IMU modules that populated on each finger & hand. Each IMU result can detect rotation of one joint. Combined with sophisticated Inverse-Kenetic Algorithm,  VRTRIX Data Glove is able to detect & simulate all gestures of human hands with six IMUs per hand (12 for a pair). 

VRTRIX IMU modules includes 9-axis sensors (gyro, accel & mag), where rotation quaternions are calculated with adaptive multi-state Kalman Filter algorithms and the output of the data stream is up to 400Hz while the latency of this module remain under 5ms. All IMU modules are connected to RF processor and the sensor tracking data packets are transmitted through 2.4GHz proprietary protocol. Thanks to the strict power control during the system design, the glove's battery can last at least 16 hours even when heavily used. The time latency between rotating your hands and the virtual hand gesture rendered in VR headset is less than 10ms. Lightning fast real-time response, precise tracking and full finger gesture detection brings the most impressive interactive experience to Virtual Reality!

# VRTRIXGlove_UE4_SDK

This repository contains the VRTRIX Glove UE4 Plugin, which includes assets that you can use to develop applications in UE4 on your Windows PC while using our VRTRIX Data Glove. These assets include models, buleprints and some simple examples to help you quickly get the hand on development in both 3D & VR/AR environment. 

**Please note that this repo is based on UE 4.18, if you are using other UE version, please rebuild the plugin yourself from source or download the pre-build plugins in [our release page][devsite].  Currently, UE4.18-UE4.21 pre-build plugins are available for download.**

## Support
- VRTRIXGlove_UE4_SDK supports UE4.18 and up, Window 10 OS**

- Before using this UE4 SDK, please install the [VRTRIX client software][driver] first to install hardwaere drivers properly.

- For detailed documentation about how to use this SDK, please checek the [doc] file.

- Note that this repository may contains code for work-in-progress modules, tentative modules, or older modules that may be unsupported.We recommend using the release version packages available on the [our release page][devsite].

[chinese]: https://github.com/VRTRIX/VRTRIXGlove_UE4_SDK/blob/master/README_CN.md "chinese"
[devsite]: https://github.com/VRTRIX/VRTRIXGlove_UE4_SDK/releases "VRTRIX Glove UE4 Release site"
[doc]: https://github.com/VRTRIX/VRTRIXGlove_UE4_SDK/blob/master/docs/VRTRIX%20Data%20Glove%20UE4%20SDK%20Tutorial.pdf "VRTRIX Glove UE4 Doc"
[driver]: https://github.com/VRTRIX/VRTRIXGlove_UE4_SDK/tree/master/drivers
