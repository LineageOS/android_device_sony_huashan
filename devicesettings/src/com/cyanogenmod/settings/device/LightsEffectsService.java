/*
 * Copyright (C) 2017 The LineageOS Project
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

package com.cyanogenmod.settings.device;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.os.IBinder;
import android.os.SystemProperties;
import android.util.Log;

import java.lang.System;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Random;

public class LightsEffectsService extends Service {

    public static final String TAG = "LightsEffectsService";

    private static final String META_CHANGED = "com.android.music.metachanged";
    private static final String PLAYSTATE_CHANGED = "com.android.music.playstatechanged";
    private static final String QUEUE_CHANGED = "com.android.music.queuechanged";

    private static final String LIGHTS_EFFECTS_GAIN_PROPERTY = "sys.lights_effects_gain";
    private static final String LIGHTS_EFFECTS_VALUE_PROPERTY = "sys.lights_effects_value";

    private Context mContext;
    private SharedPreferences.OnSharedPreferenceChangeListener mPreferencesListener;

    private long mLightsEffectsMusicColor = 0x00FFFFFF;
    private boolean mLightsEffectsMusicAlways = false;
    private boolean mLightsEffectsMusicAwake = false;
    private boolean mLightsEffectsMusicEnable = false;
    private int mLightsEffectsMusicGain = -1;

    private boolean mAudioMusicPlaying;
    private boolean mDisplayState;
    private IntentFilter mIntentMusic;
    private IntentFilter mIntentScreen;
    private boolean mIntentMusicRegistered = false;
    private boolean mIntentScreenRegistered = false;
    private final Random mRandom = new Random();

    public void onCreate() {
        super.onCreate();

        Log.d(TAG, "Creating service");

        mContext = this;
    }

    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "Starting service");

        mAudioMusicPlaying = false;
        mDisplayState = true;

        mIntentMusic = new IntentFilter(META_CHANGED);
        mIntentMusic.addAction(PLAYSTATE_CHANGED);
        mIntentMusic.addAction(QUEUE_CHANGED);

        mIntentScreen = new IntentFilter(Intent.ACTION_SCREEN_ON);
        mIntentScreen.addAction(Intent.ACTION_SCREEN_OFF);

        applySettings();

        mPreferencesListener = new SharedPreferences.OnSharedPreferenceChangeListener() {
            public void onSharedPreferenceChanged(SharedPreferences prefs, String key) {
                applySettings();
            }
        };
        SettingsUtils.registerPreferenceChangeListener(mContext, mPreferencesListener);

        return START_STICKY;
    }

    public void onDestroy() {
        Log.d(TAG, "Destroying service");

        SettingsUtils.unregisterPreferenceChangeListener(mContext, mPreferencesListener);

        super.onDestroy();
    }

    public IBinder onBind(Intent intent) {
        return null;
    }

    private void setIntentMusicEnabled(boolean enable) {
        if (enable != mIntentMusicRegistered) {
            if (enable) {
                mContext.registerReceiver(mMusicStateReceiver, mIntentMusic);
            } else {
                mContext.unregisterReceiver(mMusicStateReceiver);
            }
            mIntentMusicRegistered = enable;
        }
    }

    private void setIntentMusicAwake(boolean enable) {
        if (enable != mIntentScreenRegistered) {
            if (enable) {
                mContext.registerReceiver(mScreenStateReceiver, mIntentScreen);
            } else {
                mContext.unregisterReceiver(mScreenStateReceiver);
            }
            mIntentScreenRegistered = enable;
        }
    }

    private void applySettings() {
        boolean lightsEffectsMusicEnable = SettingsUtils.getIntPreference(mContext,
                SettingsUtils.LIGHTS_EFFECTS_MUSIC_ENABLE, 1) != 0;
        boolean lightsEffectsMusicAwake = SettingsUtils.getIntPreference(mContext,
                SettingsUtils.LIGHTS_EFFECTS_MUSIC_AWAKE, 1) != 0;
        boolean lightsEffectsMusicAlways = SettingsUtils.getIntPreference(mContext,
                SettingsUtils.LIGHTS_EFFECTS_MUSIC_ALWAYS, 0) != 0;
        int lightsEffectsMusicGain = SettingsUtils.getIntPreference(mContext,
                SettingsUtils.LIGHTS_EFFECTS_MUSIC_GAIN, 4);

        if (mLightsEffectsMusicGain != lightsEffectsMusicGain) {
            mLightsEffectsMusicGain = lightsEffectsMusicGain;
            SystemProperties.set(LIGHTS_EFFECTS_GAIN_PROPERTY,
                    String.valueOf(mLightsEffectsMusicGain));
        }

        if (mLightsEffectsMusicEnable != lightsEffectsMusicEnable ||
                mLightsEffectsMusicAwake != lightsEffectsMusicAwake ||
                mLightsEffectsMusicAlways != lightsEffectsMusicAlways ||
                mLightsEffectsMusicGain != lightsEffectsMusicGain) {
            mLightsEffectsMusicEnable = lightsEffectsMusicEnable;
            mLightsEffectsMusicAwake = lightsEffectsMusicAwake;
            mLightsEffectsMusicAlways = lightsEffectsMusicAlways;

            setIntentMusicEnabled(mLightsEffectsMusicEnable);
            setIntentMusicAwake(mLightsEffectsMusicEnable && mLightsEffectsMusicAwake);

            Log.d(TAG, "Applied lights effects settings: music [ enabled = " +
                    mLightsEffectsMusicEnable + ", gain = " + mLightsEffectsMusicGain +
                    ", only awake = " + mLightsEffectsMusicAwake + ", always " +
                    mLightsEffectsMusicAlways + " ]");

            updateLightsEffects();
        }
    }

    private void updateLightsEffects() {
        Log.d(TAG, "Updating lights effects: playing = " + mAudioMusicPlaying +
                ", display = " + mDisplayState);

        if (mLightsEffectsMusicEnable && (mAudioMusicPlaying || mLightsEffectsMusicAlways) &&
                (!mLightsEffectsMusicAwake || mDisplayState)) {
            float hue = mRandom.nextInt(360);
            float saturation = 1.0f;
            float brightness = 1.0f;
            mLightsEffectsMusicColor = Color.HSVToColor(new float[] { hue, saturation, brightness});
            String colorString = String.format("0x%06x", 0xFFFFFF & mLightsEffectsMusicColor);
            SystemProperties.set(LIGHTS_EFFECTS_VALUE_PROPERTY, colorString);
        } else {
            SystemProperties.set(LIGHTS_EFFECTS_VALUE_PROPERTY, "0");
        }
    }

    private BroadcastReceiver mMusicStateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.e(TAG, "Intent " + intent.getAction());

            if (intent.getAction().equals(PLAYSTATE_CHANGED)) {
                if (intent.hasExtra("playing")) {
                    if (intent.getBooleanExtra("playing", false)) {
                        if (!mAudioMusicPlaying) {
                            mAudioMusicPlaying = true;
                            updateLightsEffects();
                        }
                    } else {
                        if (mAudioMusicPlaying) {
                           mAudioMusicPlaying = false;
                           updateLightsEffects();
                        }
                    }
                }
            }
        }
    };

    private BroadcastReceiver mScreenStateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(Intent.ACTION_SCREEN_ON)) {
                if (!mDisplayState) {
                    mDisplayState = true;
                    updateLightsEffects();
                }
            } else if (intent.getAction().equals(Intent.ACTION_SCREEN_OFF)) {
                if (mDisplayState) {
                    mDisplayState = false;
                    updateLightsEffects();
                }
            }
        }
    };
}
