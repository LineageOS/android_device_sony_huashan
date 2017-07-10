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

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.media.MediaMetadata;
import android.media.session.MediaController;
import android.media.session.MediaSessionManager;
import android.media.session.PlaybackState;
import android.os.AsyncTask;
import android.os.IBinder;
import android.os.SystemProperties;
import android.support.v7.graphics.Palette;
import android.util.Log;

import java.lang.System;
import java.util.ArrayList;
import java.util.List;

public class LightsEffectsService extends Service {

    public static final String TAG = "LightsEffectsService";

    private static final String LIGHTS_EFFECTS_GAIN_PROPERTY = "sys.lights_effects_gain";
    private static final String LIGHTS_EFFECTS_VALUE_PROPERTY = "sys.lights_effects_value";

    private Context mContext;
    private SharedPreferences.OnSharedPreferenceChangeListener mPreferencesListener;

    private int mLightsEffectsMusicColor = 0xFFFFFF;
    private boolean mLightsEffectsMusicAlways = false;
    private boolean mLightsEffectsMusicAwake = false;
    private boolean mLightsEffectsMusicEnable = false;
    private int mLightsEffectsMusicGain = -1;

    private boolean mAudioMusicPlaying;
    private boolean mDisplayState;
    private IntentFilter mIntentScreen;
    private boolean mIntentScreenRegistered = false;
    private boolean mSessionMusicRegistered = false;
    private AsyncTask<Bitmap, Void, Palette> mPaletteTask = null;

    private MediaSessionManager mMediaSessionManager;
    private List<MediaController> mMediaActiveSessions;
    private MediaController.Callback mMediaSessionCallback;
    private MediaSessionManager.OnActiveSessionsChangedListener mMediaSessionsListener;

    public void onCreate() {
        super.onCreate();

        Log.d(TAG, "Creating service");

        mContext = this;

        mMediaSessionManager = (MediaSessionManager)mContext.getSystemService(
                Context.MEDIA_SESSION_SERVICE);
    }

    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "Starting service");

        mAudioMusicPlaying = false;
        mDisplayState = true;

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
            updateLightsEffectsGain();
        }

        if (mLightsEffectsMusicEnable != lightsEffectsMusicEnable ||
                mLightsEffectsMusicAwake != lightsEffectsMusicAwake ||
                mLightsEffectsMusicAlways != lightsEffectsMusicAlways) {
            mLightsEffectsMusicEnable = lightsEffectsMusicEnable;
            mLightsEffectsMusicAwake = lightsEffectsMusicAwake;
            mLightsEffectsMusicAlways = lightsEffectsMusicAlways;

            setIntentScreen(mLightsEffectsMusicEnable && mLightsEffectsMusicAwake);
            setSessionMusic(mLightsEffectsMusicEnable);

            Log.d(TAG, "Applied lights effects settings: music [ enabled = " +
                    mLightsEffectsMusicEnable + ", only awake = " +
                    mLightsEffectsMusicAwake + ", always " +
                    mLightsEffectsMusicAlways + " ]");

            updateLightsEffectsValue();
        }
    }

    private void updateLightsEffectsGain() {
        Log.d(TAG, "Updating lights effects: gain = " + mLightsEffectsMusicGain);

        SystemProperties.set(LIGHTS_EFFECTS_GAIN_PROPERTY,
                String.valueOf(mLightsEffectsMusicGain));
    }

    private void updateLightsEffectsValue() {
        Log.d(TAG, "Updating lights effects: playing = " + mAudioMusicPlaying +
                ", display = " + mDisplayState);

        if (mLightsEffectsMusicEnable && (mAudioMusicPlaying || mLightsEffectsMusicAlways) &&
                (!mLightsEffectsMusicAwake || mDisplayState)) {
            String colorString = String.format("0x%06x", 0xFFFFFF & mLightsEffectsMusicColor);
            SystemProperties.set(LIGHTS_EFFECTS_VALUE_PROPERTY, colorString);
        } else {
            SystemProperties.set(LIGHTS_EFFECTS_VALUE_PROPERTY, "0");
        }
    }

    private void setIntentScreen(boolean enable) {
        if (enable == mIntentScreenRegistered) {
            return;
        }

        if (enable) {
            mContext.registerReceiver(mScreenStateReceiver, mIntentScreen);
        } else {
            mContext.unregisterReceiver(mScreenStateReceiver);
        }

        mIntentScreenRegistered = enable;
    }

    private BroadcastReceiver mScreenStateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(Intent.ACTION_SCREEN_ON)) {
                if (!mDisplayState) {
                    mDisplayState = true;
                    updateLightsEffectsValue();
                }
            }
            else if (intent.getAction().equals(Intent.ACTION_SCREEN_OFF)) {
                if (mDisplayState) {
                    mDisplayState = false;
                    updateLightsEffectsValue();
                }
            }
        }
    };

    private void setSessionMusic(boolean enable) {
        if (enable == mSessionMusicRegistered) {
            return;
        }

        if (enable) {
            ComponentName componentName = new ComponentName(this, LightsEffectsService.class);
            mMediaSessionsListener = new MediaSessionManager.OnActiveSessionsChangedListener() {
                @Override
                public void onActiveSessionsChanged(List<MediaController> controllers) {
                    synchronized (this) {
                        mMediaActiveSessions = controllers;
                        registerSessionCallbacks();
                    }
                }
            };
            mMediaSessionManager.addOnActiveSessionsChangedListener(mMediaSessionsListener,
                    componentName);
            synchronized (this) {
                mMediaActiveSessions = mMediaSessionManager.getActiveSessions(componentName);
                registerSessionCallbacks();
            }
        } else {
            if (mMediaSessionsListener != null) {
                mMediaSessionManager.removeOnActiveSessionsChangedListener(mMediaSessionsListener);
                mMediaSessionsListener = null;
            }
            synchronized (this) {
                unregisterSessionCallbacks();
                mMediaActiveSessions = new ArrayList<>();
            }
        }

        mSessionMusicRegistered = enable;
    }

    private boolean isPlaybackActive(int state) {
        if (state != PlaybackState.STATE_ERROR &&
                state != PlaybackState.STATE_NONE &&
                state != PlaybackState.STATE_PAUSED &&
                state != PlaybackState.STATE_STOPPED) {
            return true;
        }
        return false;
    }

    private int getMusicColorFromMetadata(MediaMetadata metadata) {
        if (metadata == null || metadata.getString(MediaMetadata.METADATA_KEY_TITLE) == null) {
            return Color.WHITE;
        }

        float hue = metadata.getString(MediaMetadata.METADATA_KEY_TITLE).hashCode() % 360;
        float saturation = 1.0f;
        float brightness = 1.0f;
        return Color.HSVToColor(new float[] { hue, saturation, brightness });
    }

    private Palette.PaletteAsyncListener mPaletteListener = new Palette.PaletteAsyncListener() {
        @Override
        public void onGenerated(Palette palette) {
            int color = palette.getVibrantColor(Color.WHITE);
            if (color == Color.WHITE) {
                color = palette.getLightVibrantColor(Color.WHITE);
                if (color == Color.WHITE) {
                    color = palette.getDarkVibrantColor(Color.WHITE);
                }
            }
            mLightsEffectsMusicColor = color;
            updateLightsEffectsValue();
            mPaletteTask = null;
        }
    };

    private void registerSessionCallbacks() {
        if (mMediaSessionCallback == null) {
            mMediaSessionCallback = new MediaController.Callback() {
                @Override
                public void onPlaybackStateChanged(PlaybackState state) {
                    super.onPlaybackStateChanged(state);
                    if (state != null) {
                        mAudioMusicPlaying = isPlaybackActive(state.getState());
                        updateLightsEffectsValue();
                    }
                }

                @Override
                public void onMetadataChanged(MediaMetadata metadata) {
                    super.onMetadataChanged(metadata);
                    if (metadata == null) {
                        return;
                    }

                    Bitmap artworkBitmap = metadata.getBitmap(MediaMetadata.METADATA_KEY_ART);
                    if (artworkBitmap == null) {
                        artworkBitmap = metadata.getBitmap(MediaMetadata.METADATA_KEY_ALBUM_ART);
                    }

                    if (mPaletteTask != null) {
                        if (!mPaletteTask.isCancelled()) {
                            mPaletteTask.cancel(true);
                        }
                        mPaletteTask = null;
                    }

                    if (artworkBitmap != null) {
                        mPaletteTask = Palette.generateAsync(artworkBitmap, mPaletteListener);
                    } else {
                        mLightsEffectsMusicColor = getMusicColorFromMetadata(metadata);
                        updateLightsEffectsValue();
                    }
                }
            };
        }

        for (MediaController controller : mMediaActiveSessions) {
            controller.registerCallback(mMediaSessionCallback);
        }
    }

    private void unregisterSessionCallbacks() {
        if (mMediaSessionCallback != null) {
            for (MediaController controller : mMediaActiveSessions) {
                controller.unregisterCallback(mMediaSessionCallback);
            }
            mMediaSessionCallback = null;
        }
    }
}
