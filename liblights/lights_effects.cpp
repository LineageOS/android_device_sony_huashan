/*
 * Copyright (C) 2017 Adrian DC
 *           (C) 2017 The LineageOS Project
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

/* === Module Debug === */
#define LOG_TAG "lights_effects.huashan"

/* === Module Libraries === */
#include <cutils/log.h>
#include <cutils/properties.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <tinyalsa/asoundlib.h>

/* === Module Header === */
#include "lights.h"

/* === Module read_uint === */
static unsigned long read_uint(char const* path) {

    int fd, amt;
    char buffer[20];
    char* ptr;

    /* Path to unsigned int */
    fd = open(path, O_RDONLY);
    if (fd >= 0) {
        amt = read(fd, buffer, sizeof(buffer) / sizeof(buffer[0]));
        close(fd);
        if (amt != -1) {
            return strtoul(buffer, &ptr, 10);
        }
    }

    /* Failed read */
    ALOGE("read_int failed to open %s (%s)\n", path, strerror(errno));
    return 0;
}

/* === Module write_int === */
static int write_int(char const* path, int value) {

    int fd, amt, bytes;
    char buffer[20];

    /* Int output to path */
    fd = open(path, O_WRONLY);
    if (fd >= 0) {
        bytes = snprintf(buffer, sizeof(buffer), "%d\n", value);
        amt = write(fd, buffer, bytes);
        close(fd);
        return (amt == -1 ? -errno : 0);
    }
    else {
        ALOGE("write_int failed to open %s (%s)\n", path, strerror(errno));
        return -errno;
    }
}

/* === Module write_string === */
static int write_string(char const* path, const char *value)
{
    int fd, amt, bytes;
    char buffer[20];

    /* String output to path */
    fd = open(path, O_WRONLY);
    if (fd >= 0) {
        bytes = snprintf(buffer, sizeof(buffer), "%s\n", value);
        amt = write(fd, buffer, bytes);
        close(fd);
        return (amt == -1 ? -errno : 0);
    }
    else {
        ALOGE("write_string failed to open %s (%s)\n", path, strerror(errno));
        return -errno;
    }
}

/* === Module mixer_set_int === */
static void mixer_set_int(struct mixer* mixer, const char* name, int value)
{
    /* Acquire control by name */
    struct mixer_ctl* ctl = mixer_get_ctl_by_name(mixer, name);
    if (!ctl) {
        ALOGE("Mixer: Invalid mixer control (%s)\n", name);
        return;
    }

    /* Set value to control */
    if (mixer_ctl_set_value(ctl, 0, value)) {
        ALOGE("Mixer: Invalid value for index 0\n");
        return;
    }
}

/* === Module mixer_set_enum === */
static void mixer_set_enum(struct mixer* mixer, const char* name,
        const char* value)
{
    /* Acquire control by name */
    struct mixer_ctl* ctl = mixer_get_ctl_by_name(mixer, name);
    if (!ctl) {
        ALOGE("Mixer: Invalid mixer control (%s)\n", name);
        return;
    }

    /* Set value to control */
    if (mixer_ctl_set_enum_by_string(ctl, value)) {
        ALOGE("Mixer: Invalid enum value\n");
        return;
    }
}

/* === Module set_light_mixer_music === */
static void set_light_mixer_music(bool enable)
{
    /* Acquire mixer card */
    struct mixer* mixer = mixer_open(0);
    if (!mixer) {
        ALOGE("Mixer: Failed to open mixer\n");
        return;
    }

    /* Apply Huashan LEDs music routes */
    mixer_set_int(mixer, "RX5 Digital Volume", enable ? 82 : 84);
    mixer_set_int(mixer, "LINEOUT2 Volume", enable ? 12 : 0);
    mixer_set_enum(mixer, "RX5 MIX1 INP2", enable ? "RX2" : "ZERO");
    mixer_set_enum(mixer, "RX5 MIX1 INP1", enable ? "RX1" : "ZERO");
    mixer_set_int(mixer, "SLIMBUS_0_RX Audio Mixer MultiMedia5",
            enable ? 1 : 0);

    /* Release mixer card */
    mixer_close(mixer);
}

/* === Module Entrypoint === */
int main(int __attribute__((unused)) argc, char __attribute__((unused)) **argv)
{
    /* Variables */
    int lights_effects_buf_gain = LEDS_AUDIO_BUF_GAIN_ON;
    bool lights_effects_enabled = false;
    char lights_effects_gain[PROP_VALUE_MAX];
    char lights_effects_value[PROP_VALUE_MAX];
    char* ptr;

    /* Lights effects update property */
    property_get("sys.lights_effects_value", lights_effects_value, "");
    if (strcmp(lights_effects_value, "") &&
            strtoul(lights_effects_value, &ptr, 16) > 0) {
        lights_effects_enabled = true;
    }

    /* Lights effects gain property */
    property_get("sys.lights_effects_gain", lights_effects_gain, "");
    if (strcmp(lights_effects_gain, "")) {
        lights_effects_buf_gain = strtoul(lights_effects_gain, &ptr, 16);
    }

    /* Lights effects information */
    ALOGI("Updating lights effects: %s, value = %s, gain = %d", lights_effects_enabled ?
            "enabled" : "disabled", lights_effects_value, lights_effects_buf_gain);

    /* Lights effects enabled */
    if (lights_effects_enabled) {
        write_int(LEDS_AUDIO_EN_FILE, LEDS_AUDIO_EN_OFF);
        write_int(LEDS_AUDIO_BUF_GAIN_FILE, lights_effects_buf_gain);
        write_int(LEDS_AUDIO_AGC_CTRL_FILE, LEDS_AUDIO_AGC_CTRL_ON);
        write_string(LEDS_LIGHTS_EFFECTS_FILE, lights_effects_value);
        write_int(LEDS_AUDIO_EN_FILE, LEDS_AUDIO_EN_ON);
        set_light_mixer_music(true);
    }
    /* Lights effects disabled */
    else {
        set_light_mixer_music(false);
        write_string(LEDS_LIGHTS_EFFECTS_FILE, LEDS_LIGHTS_EFFECTS_OFF);
        write_int(LEDS_AUDIO_EN_FILE, LEDS_AUDIO_EN_OFF);
        write_int(LEDS_AUDIO_BUF_GAIN_FILE, LEDS_AUDIO_BUF_GAIN_OFF);
        write_int(LEDS_AUDIO_AGC_CTRL_FILE, LEDS_AUDIO_AGC_CTRL_OFF);
    }

    return 0;
}
