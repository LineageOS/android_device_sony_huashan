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

#define LOG_TAG "android.hardware.light@2.0-service.huashan"

#include <hidl/HidlTransportSupport.h>
#include <hwbinder/ProcessState.h>

#include "Light.h"

// libhwbinder:
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;

// Generated HIDL files
using android::hardware::light::V2_0::ILight;
using android::hardware::light::V2_0::implementation::Light;

const static std::string kBacklight1Path = "/sys/devices/i2c-10/10-0040/leds/lcd-backlight1/brightness";
const static std::string kBacklight2Path = "/sys/devices/i2c-10/10-0040/leds/lcd-backlight2/brightness";

int main() {
    android::hardware::ProcessState::initWithMmapSize((size_t)(32768));

    std::ofstream backlight1(kBacklight1Path);
    if (!backlight1) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", kBacklight1Path.c_str(), error, strerror(error));
        return -error;
    }

    std::ofstream backlight2(kBacklight2Path);
    if (!backlight2) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", kBacklight2Path.c_str(), error, strerror(error));
        return -error;
    }

    android::sp<ILight> service = new Light(std::move(backlight1), std::move(backlight2));

    configureRpcThreadpool(1, true);

    android::status_t status = service->registerAsService();

    if (status != android::OK) {
        ALOGE("Cannot register Light HAL service");
        return 1;
    }

    ALOGI("Light HAL Ready.");
    joinRpcThreadpool();

    // Under normal cases, execution will not reach this line.
    ALOGE("Light HAL failed to join thread pool.");
    return 1;
}
