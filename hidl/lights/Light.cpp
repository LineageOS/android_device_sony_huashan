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

#define LOG_TAG "light"

#include "Light.h"

#include <log/log.h>

namespace {
using android::hardware::light::V2_0::LightState;

static uint32_t rgbToBrightness(const LightState& state) {
    uint32_t color = state.color & 0x00ffffff;
    return ((77 * ((color >> 16) & 0xff)) + (150 * ((color >> 8) & 0xff)) +
            (29 * (color & 0xff))) >> 8;
}

static bool isLit(const LightState& state) {
    return (state.color & 0x00ffffff);
}
} // anonymous namespace

namespace android {
namespace hardware {
namespace light {
namespace V2_0 {
namespace implementation {

// === AS3665 Constants ===
const static uint32_t LEDS_COLORS_COUNT = 3;
const static uint32_t LEDS_UNIT_COUNT = 3;

// === AS3665 Constants ===
const static uint32_t LEDS_SEQ_BLINK_NONE = 0;
const static uint32_t LEDS_SEQ_BLINK_RAMPUP_SMOOTH = 2;
const static uint32_t LEDS_SEQ_BLINK_RAMPDOWN_SMOOTH = 3;
const static float LEDS_SEQ_SECOND_TIME = 13.0f;

// === AS3665 LEDs ===
const static std::string LEDS_COLORS_BRIGHTNESS_FILE = "/sys/class/leds/LED%d_%c/brightness";
const static std::string LEDS_COLORS_COLOR_ID_FILE = "/sys/class/leds/LED%d_%c/color_id";
const static std::string LEDS_COLORS_CURRENT_FILE = "/sys/class/leds/LED%d_%c/led_current";
const static std::string LEDS_COLORS_EFFECTS_CURRENT_FILE = "/sys/class/leds/LED%d_%c/effects_current";
const static uint32_t LEDS_COLORS_BRIGHTNESS_MAXIMUM = 255;
const static uint32_t LEDS_COLORS_CURRENT_CHARGING = 127;
const static uint32_t LEDS_COLORS_CURRENT_NOTIFICATIONS = 255;
const static uint32_t LEDS_COLORS_CURRENT_MAXIMUM = 255;
const static uint32_t LEDS_COLORS_CURRENT_RATIO = 255;
const static uint32_t LEDS_COLORS_EFFECTS_CURRENT_RATIO = 85;
const static uint32_t LEDS_CHARGED_DELAY_OFF = 5000;
const static uint32_t LEDS_CHARGED_DELAY_ON = 5000;
const static uint32_t LEDS_CHARGED_LEVEL = 100;

// === AS3665 Paths ===
const static std::string LEDS_SEQ1_MODE_FILE = "/sys/devices/i2c-10/10-0047/sequencer1_mode";
const static std::string LEDS_SEQ2_MODE_FILE = "/sys/devices/i2c-10/10-0047/sequencer2_mode";
const static std::string LEDS_SEQ3_MODE_FILE = "/sys/devices/i2c-10/10-0047/sequencer3_mode";
const static std::string LEDS_SEQ1_RUN_FILE = "/sys/devices/i2c-10/10-0047/sequencer1_run_mode";
const static std::string LEDS_SEQ2_RUN_FILE = "/sys/devices/i2c-10/10-0047/sequencer2_run_mode";
const static std::string LEDS_SEQ3_RUN_FILE = "/sys/devices/i2c-10/10-0047/sequencer3_run_mode";
const static std::string LEDS_SEQ_LOAD_FILE = "/sys/devices/i2c-10/10-0047/sequencer_load";

// === AS3665 Presets ===
const char leds_colors[3]                = { 'R', 'G', 'B' };
const int leds_currents[3][3]            = { { 52, 66, 48 }, { 54, 66, 52 }, { 50, 66, 46 } };
const int leds_map[3][3]                 = { { 6, 3, 0 }, { 7, 4, 1 }, { 8, 5, 2 } };

// === AS3665 Sequencer ===
const static std::string LEDS_SEQ_MODE_DISABLED = "disabled";
const static std::string LEDS_SEQ_MODE_ACTIVATED = "reload";
const static std::string LEDS_SEQ_RUN_DISABLED = "hold";
const static std::string LEDS_SEQ_RUN_ACTIVATED = "run";
const static std::string LEDS_SEQ_LOAD_PROGRAM = "000e0e9d009c0e9c8f9d80%02xff9dc0%02xff9d80%02xff9dc0%02xff9d80a004c00000000%03x";
const static std::string LEDS_SEQ_MUSIC_PROGRAM = "0021219d009c219ca340009d8089aa8f0a906e94008a2295018c429d80400014009dc090009b0a8a128c2240008a1284629d80900088249a32846214009dc08e0a8a2fa00501ff004901b6";

// === AS3677 LCD ===
const static uint32_t LCD_BRIGHTNESS_MAX = 255;
const static uint32_t LCD_BRIGHTNESS_MIN = 1;
const static uint32_t LCD_BRIGHTNESS_OFF = 0;

// === AS3677 LCD Regulations ===
const static uint32_t LCD_BACKLIGHT_SLOWED_SPAN = 128;
const static uint32_t LCD_BACKLIGHT_SLOWED_MAX = (LCD_BACKLIGHT_SLOWED_SPAN / 2);
const static uint32_t LCD_BACKLIGHT_ACCELERATED_REAL = (LCD_BRIGHTNESS_MAX - LCD_BACKLIGHT_SLOWED_MAX - LCD_BRIGHTNESS_MIN);
const static uint32_t LCD_BACKLIGHT_ACCELERATED_SPAN = (LCD_BRIGHTNESS_MAX - LCD_BACKLIGHT_SLOWED_SPAN);

// === Framework Constants ===
const static uint32_t LIGHT_MODE_MULTIPLE_LEDS = 0x01;

// === Module Constants ===
const static uint32_t LIGHT_BRIGHTNESS_MAXIMUM = 0xFF;

Light::Light(std::ofstream&& backlight1, std::ofstream&& backlight2)
    : mBacklight1(std::move(backlight1))
    , mBacklight2(std::move(backlight2))
    , mSequencer1Mode(LEDS_SEQ1_MODE_FILE)
    , mSequencer2Mode(LEDS_SEQ2_MODE_FILE)
    , mSequencer3Mode(LEDS_SEQ3_MODE_FILE)
    , mSequencer1Run(LEDS_SEQ1_RUN_FILE)
    , mSequencer2Run(LEDS_SEQ2_RUN_FILE)
    , mSequencer3Run(LEDS_SEQ3_RUN_FILE)
    , mSequencerLoad(LEDS_SEQ_LOAD_FILE, std::ios::binary)
    , mLightsARGB(0)
    , mLightsBrightness(0)
    , mLightsDelayOn(-1)
    , mLightsDelayOff(-1)
    , mLightsState(LEDS_OFF)
    , mSequencer1State(LEDS_SEQ_UNKNOWN)
    , mSequencer2State(LEDS_SEQ_UNKNOWN)
    , mSequencer3State(LEDS_SEQ_UNKNOWN)
    , mSequencerProgramTarget(LEDS_SEQ_BLINK_NONE)
{
    // Variables
    char path[100];

    // Module paths
    for (size_t i = 1; i <= LEDS_UNIT_COUNT; ++i) {
        for (size_t c = 0; c < LEDS_COLORS_COUNT; ++c) {

            // Initialize notification brightess paths
            snprintf(path, LEDS_COLORS_BRIGHTNESS_FILE.length(), LEDS_COLORS_BRIGHTNESS_FILE.c_str(), i, leds_colors[c]);
            mNotificationBrightness[(i - 1) * LEDS_COLORS_COUNT + c].open(path);

            // Initialize notification current paths
            snprintf(path, LEDS_COLORS_CURRENT_FILE.length(), LEDS_COLORS_CURRENT_FILE.c_str(), i, leds_colors[c]);
            mNotificationCurrent[(i - 1) * LEDS_COLORS_COUNT + c].open(path);

            // Initialize color_id controls
            snprintf(path, LEDS_COLORS_COLOR_ID_FILE.length(), LEDS_COLORS_COLOR_ID_FILE.c_str(), i, leds_colors[c]);
            char color_id[2];
            color_id[0] = leds_colors[c];
            color_id[1] = 0;
            std::ofstream(path).write(color_id, 2);

            // Initialize effects_current controls
            snprintf(path, LEDS_COLORS_EFFECTS_CURRENT_FILE.length(), LEDS_COLORS_EFFECTS_CURRENT_FILE.c_str(), i, leds_colors[c]);
            int value = (LEDS_COLORS_EFFECTS_CURRENT_RATIO * leds_currents[i-1][c]) / LEDS_COLORS_CURRENT_MAXIMUM;
            std::ofstream(path) << value << std::endl;
        }
    }

    auto attnFn(std::bind(&Light::setAttentionLight, this, std::placeholders::_1));
    auto backlightFn(std::bind(&Light::setBacklight, this, std::placeholders::_1));
    auto batteryFn(std::bind(&Light::setBatteryLight, this, std::placeholders::_1));
    auto notifFn(std::bind(&Light::setNotificationLight, this, std::placeholders::_1));
    mLights.emplace(std::make_pair(Type::ATTENTION, attnFn));
    mLights.emplace(std::make_pair(Type::BACKLIGHT, backlightFn));
    mLights.emplace(std::make_pair(Type::BATTERY, batteryFn));
    mLights.emplace(std::make_pair(Type::NOTIFICATIONS, notifFn));
}

// Methods from ::android::hardware::light::V2_0::ILight follow.
Return<Status> Light::setLight(Type type, const LightState& state) {
    if (mLights.find(type) != mLights.end()) {
        mLights.at(type)(state);
        return Status::SUCCESS;
    }
    return Status::LIGHT_NOT_SUPPORTED;
}

Return<void> Light::getSupportedTypes(getSupportedTypes_cb _hidl_cb) {
    Type *types = new Type[mLights.size()];
    int idx = 0;

    for (auto const &kv : mLights) {
        Type t = kv.first;
        types[idx++] = t;
    }

    {
        hidl_vec<Type> hidl_types{};
        hidl_types.setToExternal(types, mLights.size());

        _hidl_cb(hidl_types);
    }

    delete[] types;

    return Void();
}

void Light::setAttentionLight(const LightState& state) {
    std::lock_guard<std::mutex> lock(mLock);
    mAttentionState = state;
    setSpeakerBatteryLightLocked();
}

void Light::setBacklight(const LightState& state) {
    std::lock_guard<std::mutex> lock(mLock);

    uint32_t brightness = rgbToBrightness(state);

    // LCD brightness limitations
    if (brightness <= LCD_BRIGHTNESS_OFF) {
        brightness = LCD_BRIGHTNESS_OFF;
    }
    else if (brightness < LCD_BRIGHTNESS_MIN) {
        brightness = LCD_BRIGHTNESS_MIN;
    }
    else if (brightness > LCD_BRIGHTNESS_MAX) {
        brightness = LCD_BRIGHTNESS_MAX;
    }

    // LCD backlight incremental override
    if (brightness != LCD_BRIGHTNESS_OFF) {
        if (brightness < LCD_BACKLIGHT_SLOWED_SPAN)  {
            brightness = LCD_BRIGHTNESS_MIN + brightness / 2;
        }
        else {
            brightness = LCD_BRIGHTNESS_MIN + LCD_BACKLIGHT_SLOWED_MAX +
                    ((brightness - LCD_BACKLIGHT_SLOWED_SPAN)
                    * LCD_BACKLIGHT_ACCELERATED_REAL)
                    / LCD_BACKLIGHT_ACCELERATED_SPAN;
        }
    }

    // LCD brightness update
    mBacklight1 << brightness << std::endl;
    mBacklight2 << brightness << std::endl;
}

void Light::setBatteryLight(const LightState& state) {
    std::lock_guard<std::mutex> lock(mLock);
    mBatteryState = state;
    setSpeakerBatteryLightLocked();
}

void Light::setNotificationLight(const LightState& state) {
    std::lock_guard<std::mutex> lock(mLock);
    mNotificationState = state;
    setSpeakerBatteryLightLocked();
}

void Light::setSpeakerBatteryLightLocked() {
    if (isLit(mNotificationState)) {
        mLightsState = LEDS_NOTIFICATIONS;
        setSpeakerLightLocked(mNotificationState);
    } else if (isLit(mAttentionState)) {
        setSpeakerLightLocked(mAttentionState);
    } else {
        mLightsState = LEDS_BATTERY;
        setSpeakerLightLocked(mBatteryState);
    }
}

void Light::setSpeakerLightLocked(const LightState& state) {
    // Variables
    int32_t i, c;
    Flash flashMode;
    int32_t leds_modes;
    leds_rgbupdate leds_rgb_update;
    int32_t leds_unit_minid;
    int32_t leds_unit_maxid;
    int32_t leds_program_target;
    leds_pupdate leds_program_update;
    int32_t delayOn, delayOff;
    uint32_t colorARGB;
    uint32_t leds_brightness;
    uint32_t led_rgb[3];

    // LEDs variables processing
    colorARGB = state.color;
    leds_brightness = (colorARGB & 0xFF000000) >> 24;
    leds_modes = LIGHT_MODE_MULTIPLE_LEDS;
    leds_unit_minid = 1;
    leds_unit_maxid = LEDS_UNIT_COUNT;
    delayOn = state.flashOnMs;
    delayOff = state.flashOffMs;
    flashMode = state.flashMode;
    led_rgb[0] = (colorARGB >> 16) & 0xFF;
    led_rgb[1] = (colorARGB >> 8) & 0xFF;
    led_rgb[2] = colorARGB & 0xFF;
    leds_program_update = LEDS_PROGRAM_WRITE;
    leds_rgb_update = LEDS_RGB_KEEP;

    // Avoid flashing programs with an empty delay
    if (delayOn == 0 || delayOff == 0) {
        flashMode = Flash::NONE;
    }

    // Use multiple LEDs
    if (leds_modes & LIGHT_MODE_MULTIPLE_LEDS) {

        // LEDs charging witness mode
        if (isLit(mBatteryState)) {

            // LED charging update
            if (mLightsState == LEDS_BATTERY) {
                leds_unit_maxid = 1;

                // LEDs finished notification reset
                if (!isLit(mNotificationState)) {

                    // Side LEDs are disabled
                    uint32_t led_rgb_off[3] = {0,0,0};
                    for (i = 2; i <= LEDS_UNIT_COUNT; ++i) {
                        writeLightsLED(i, led_rgb_off, 0, LEDS_RGB_WRITE);
                    }
                    setLightsProgram(LEDS_PROGRAM_KEEP, LEDS_SEQ_BLINK_NONE, flashMode, 0, 0);

                    // Framework supported, notify 100% charged device
                    if (leds_brightness == LEDS_CHARGED_LEVEL) {
                        delayOff = LEDS_CHARGED_DELAY_OFF;
                        delayOn = LEDS_CHARGED_DELAY_ON;
                        flashMode = Flash::TIMED;
                    }
                }
            }
            // LED charging kept in the middle
            else {
                leds_unit_minid = 2;

                // LEDs reset battery state
                uint32_t led_rgb_bat[3];
                int current_leds_state = mLightsState;
                mLightsState = LEDS_BATTERY;
                led_rgb_bat[0] = (mBatteryState.color >> 16) & 0xFF;
                led_rgb_bat[1] = (mBatteryState.color >> 8) & 0xFF;
                led_rgb_bat[2] = mBatteryState.color & 0xFF;
                writeLightsLED(1, led_rgb_bat, leds_brightness, LEDS_RGB_WRITE);
                mLightsState = current_leds_state;
            }
        }
    }
    // Use single LED
    else {
        uint32_t led_rgb_off[3] = {0,0,0};
        leds_unit_minid = 1;
        leds_unit_maxid = 1;
        for (i = 2; i <= LEDS_UNIT_COUNT; ++i) {
            writeLightsLED(i, led_rgb_off, 0, LEDS_RGB_WRITE);
        }
        setLightsProgram(LEDS_PROGRAM_KEEP, LEDS_SEQ_BLINK_NONE, flashMode, 0, 0);
    }

    // Detection of the delays update
    if (delayOn != mLightsDelayOn || delayOff != mLightsDelayOff) {

        // Write the new sequencer
        if (flashMode == Flash::TIMED) {
          leds_program_update = LEDS_PROGRAM_WRITE;
        }
        // Update the colors and keep the sequencer in memory
        else {
          leds_program_update = LEDS_PROGRAM_KEEP;
          leds_rgb_update = LEDS_RGB_WRITE;
        }

        // Store new delays
        mLightsDelayOn = delayOn;
        mLightsDelayOff = delayOff;
    }
    // Keep the same sequencer in memory
    else {
        leds_program_update = LEDS_PROGRAM_KEEP;
    }

    // Build the LEDs program target
    leds_program_target = LEDS_SEQ_BLINK_NONE;
    for (c = 0; c < LEDS_COLORS_COUNT; ++c) {
        if (led_rgb[c] > 0) {
            for (i = leds_unit_minid; i <= leds_unit_maxid; ++i) {
                leds_program_target |= 1 << leds_map[i-1][c];
            }
        }
    }

    // Detection of the LEDs program target update
    if (leds_program_target != mSequencerProgramTarget) {
        leds_program_update = LEDS_PROGRAM_WRITE;
        leds_rgb_update = LEDS_RGB_WRITE;
    }
    // Detection of the brightness update
    else if (leds_brightness != mLightsBrightness) {
        leds_rgb_update = LEDS_RGB_WRITE;
    }
    // Detection of the LEDs ARGB update
    else if (colorARGB != mLightsARGB) {
        leds_rgb_update = LEDS_RGB_WRITE;
    }

    // Update global LEDs variables
    mLightsBrightness = leds_brightness;
    mSequencerProgramTarget = leds_program_target;
    mLightsARGB = colorARGB;

    // LEDs units individual activation
    for (i = leds_unit_minid; i <= leds_unit_maxid; ++i) {
        writeLightsLED(i, led_rgb, leds_brightness, leds_rgb_update);
    }

    // LEDs pattern programming
    setLightsProgram(leds_program_update, leds_program_target, flashMode, delayOn, delayOff);

    // LEDs debug text
    ALOGV("LEDs : %d %d %d - delayOn : %d, delayOff : %d - Flash : %d - "
            "Update : %d/%d - Brightness : %d - LEDs Mode : %d - "
            "Mode : %d (Not. 1 / Bat. 2)",
            led_rgb[0], led_rgb[1], led_rgb[2], delayOn, delayOff, flashMode,
            leds_rgb_update, leds_program_update, leds_brightness, leds_modes,
            mLightsState);
}

void Light::writeLightsLED(int i, unsigned int leds_rgb[3],
        unsigned int leds_brightness, leds_rgbupdate leds_rgb_update) {

    int c, il;
    unsigned int led_current;
    unsigned int led_current_ratio = 0;

    // LED current update
    if (leds_rgb_update == LEDS_RGB_KEEP) {
        return;
    }

    // LED unit current limits for battery indications
    if (mLightsState == LEDS_BATTERY) {
        led_current_ratio = LEDS_COLORS_CURRENT_CHARGING;
    }
    // LED unit current limits for all notifications
    else {
        led_current_ratio = LEDS_COLORS_CURRENT_NOTIFICATIONS;

        // Apply the system settings LEDs brightness limit
        if (leds_brightness > 0) {
            led_current_ratio = (led_current_ratio * leds_brightness) /
                    LIGHT_BRIGHTNESS_MAXIMUM;
            if (led_current_ratio > LEDS_COLORS_CURRENT_MAXIMUM) {
                led_current_ratio = LEDS_COLORS_CURRENT_MAXIMUM;
            }
        }
    }

    // LED individual color update
    for (c = 0; c < LEDS_COLORS_COUNT; ++c) {

        // Evaluate current
        il = (i - 1) * LEDS_COLORS_COUNT + c;
        led_current = (leds_rgb[c] * led_current_ratio) /
                LEDS_COLORS_CURRENT_RATIO;
        led_current = (led_current * leds_currents[i-1][c]) /
                LEDS_COLORS_CURRENT_MAXIMUM;
        if (led_current < 1 && leds_rgb[c] > 0) {
            led_current = 1;
        }

        // Apply brightness and current
        mNotificationBrightness[il] << (leds_rgb[c] != 0 ? LEDS_COLORS_BRIGHTNESS_MAXIMUM : 0) << std::endl;
        mNotificationCurrent[il] << led_current << std::endl;
    }
}

void Light::setLightsProgram(leds_pupdate leds_program_update, int leds_program_target,
        Flash flashMode, int delayOn, int delayOff) {

    // Flashing sequencer handler
    switch (flashMode) {

        // LEDs blinking sequence programmer
        case Flash::TIMED:
            if (leds_program_target != LEDS_SEQ_BLINK_NONE) {
                // Initialize the sequencer
                if (mSequencer1State == LEDS_SEQ_UNKNOWN) {
                    leds_program_update = LEDS_PROGRAM_WRITE;
                }
                // Write an updated sequencer
                if (leds_program_update == LEDS_PROGRAM_WRITE) {
                    writeLightsProgram(leds_program_target, delayOn, delayOff);
                    mSequencer1State = LEDS_SEQ_DISABLED;
                }
                // Execute the stored sequencer
                if (mSequencer1State != LEDS_SEQ_ENABLED) {
                    mSequencer1Mode << LEDS_SEQ_MODE_ACTIVATED << std::endl;
                    mSequencer1Run << LEDS_SEQ_RUN_ACTIVATED << std::endl;
                    mSequencer1State = LEDS_SEQ_ENABLED;
                }
            }
            break;

        // LEDs held sequence programmer
        case Flash::NONE:
        default:
            // Clear and disable the sequencer
            if (leds_program_target != LEDS_SEQ_BLINK_NONE &&
                    leds_program_update == LEDS_PROGRAM_WRITE) {
                writeLightsProgram(LEDS_SEQ_BLINK_NONE, delayOn, delayOff);
            }
            mSequencer1Run << LEDS_SEQ_RUN_DISABLED << std::endl;
            mSequencer1Mode << LEDS_SEQ_MODE_DISABLED << std::endl;
            mSequencer1State = LEDS_SEQ_DISABLED;
            break;
    }

    // Deactivate unused sequencers
    if (mSequencer2State != LEDS_SEQ_DISABLED) {
        mSequencer2Run << LEDS_SEQ_RUN_DISABLED << std::endl;
        mSequencer2Mode << LEDS_SEQ_MODE_DISABLED << std::endl;
        mSequencer2State = LEDS_SEQ_DISABLED;
    }
    if (mSequencer3State != LEDS_SEQ_DISABLED) {
        mSequencer3Run << LEDS_SEQ_RUN_DISABLED << std::endl;
        mSequencer3Mode << LEDS_SEQ_MODE_DISABLED << std::endl;
        mSequencer3State = LEDS_SEQ_DISABLED;
    }
}

void Light::writeLightsProgram(int leds_program_target, int delayOn, int delayOff) {

    // Variables
    int bytes;
    int values[4];
    char buffer[180];

    // Values calculation with concern to the precision and overflows
    values[0] = std::min(LEDS_SEQ_BLINK_RAMPUP_SMOOTH, (uint32_t)255) & 0b11111110;
    values[1] = std::min((uint32_t)((LEDS_SEQ_SECOND_TIME * (float)delayOn) / 1000.0), (uint32_t)63);
    values[2] = std::min(LEDS_SEQ_BLINK_RAMPDOWN_SMOOTH, (uint32_t)255) | 0b00000001;
    values[3] = std::min((uint32_t)((LEDS_SEQ_SECOND_TIME * (float)delayOff) / 1000.0), (uint32_t)63);

    // Sequencer string creation
    bytes = snprintf(buffer, sizeof(buffer), LEDS_SEQ_LOAD_PROGRAM.c_str(),
            values[0], values[1], values[2], values[3],
            leds_program_target);
    mSequencerLoad << buffer << std::endl;
    ALOGV("Sequencer : %s", buffer);
}

}  // namespace implementation
}  // namespace V2_0
}  // namespace light
}  // namespace hardware
}  // namespace android
