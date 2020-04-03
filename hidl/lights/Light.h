/*
 * Copyright (C) 2015-2020 Adrian DC
 *           (C) 2015-2016 The CyanogenMod Project
 *           (C) 2017-2020 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_HARDWARE_LIGHT_V2_0_LIGHT_H
#define ANDROID_HARDWARE_LIGHT_V2_0_LIGHT_H

#include <android/hardware/light/2.0/ILight.h>
#include <hidl/Status.h>

#include <fstream>
#include <mutex>
#include <unordered_map>

namespace android {
namespace hardware {
namespace light {
namespace V2_0 {
namespace implementation {

enum leds_program { LEDS_PROGRAM_OFF, LEDS_PROGRAM_RUN };
enum leds_pupdate { LEDS_PROGRAM_KEEP, LEDS_PROGRAM_WRITE };
enum leds_rgbupdate { LEDS_RGB_KEEP, LEDS_RGB_WRITE };
enum leds_sequencers { LEDS_SEQ_UNKNOWN, LEDS_SEQ_DISABLED, LEDS_SEQ_ENABLED };
enum leds_state { LEDS_OFF, LEDS_NOTIFICATIONS, LEDS_BATTERY };

struct Light : public ILight {
    Light(std::ofstream&& backlight1, std::ofstream&& backlight2);

    // Methods from ::android::hardware::light::V2_0::ILight follow.
    Return<Status> setLight(Type type, const LightState& state)  override;
    Return<void> getSupportedTypes(getSupportedTypes_cb _hidl_cb)  override;

private:
    void setAttentionLight(const LightState& state);
    void setBacklight(const LightState& state);
    void setBatteryLight(const LightState& state);
    void setNotificationLight(const LightState& state);
    void setSpeakerBatteryLightLocked();
    void setSpeakerLightLocked(const LightState& state);

    void writeLightsLED(int i, unsigned int leds_rgb[3],
            unsigned int leds_brightness, leds_rgbupdate leds_rgb_update);
    void setLightsProgram(leds_pupdate leds_program_update, int leds_program_target,
            Flash flashMode, int delayOn, int delayOff);
    void writeLightsProgram(int leds_program_target, int delayOn, int delayOff);

    std::ofstream mBacklight1;
    std::ofstream mBacklight2;
    std::ofstream mNotificationBrightness[9];
    std::ofstream mNotificationCurrent[9];
    std::ofstream mSequencer1Mode;
    std::ofstream mSequencer2Mode;
    std::ofstream mSequencer3Mode;
    std::ofstream mSequencer1Run;
    std::ofstream mSequencer2Run;
    std::ofstream mSequencer3Run;
    std::ofstream mSequencerLoad;

    LightState mAttentionState;
    LightState mBatteryState;
    LightState mNotificationState;

    unsigned int mLightsARGB;
    unsigned int mLightsBrightness;
    int mLightsDelayOn;
    int mLightsDelayOff;
    int mLightsState;
    int mSequencer1State;
    int mSequencer2State;
    int mSequencer3State;
    int mSequencerProgramTarget;

    std::unordered_map<Type, std::function<void(const LightState&)>> mLights;
    std::mutex mLock;
};

}  // namespace implementation
}  // namespace V2_0
}  // namespace light
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_LIGHT_V2_0_LIGHT_H
