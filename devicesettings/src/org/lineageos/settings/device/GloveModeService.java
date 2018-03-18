/*
 * Copyright (c) 2015-2016 The CyanogenMod Project
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

package org.lineageos.settings.device;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.IBinder;
import android.util.Log;

import java.lang.System;
import java.io.FileOutputStream;
import java.io.IOException;

public class GloveModeService extends Service {

    public static final boolean DEBUG = false;
    public static final String PREFERENCES = "GloveModePreferences";
    public static final String TAG = "GloveModeService";

    private static final String GLOVE_PATH = "/sys/devices/i2c-3/3-0024/main_ttsp_core.cyttsp4_i2c_adapter/signal_disparity";
    private static final long GLOVE_UPDATE_DELTA = 900000; // 15 minutes

    private Context mContext;
    private SharedPreferences.OnSharedPreferenceChangeListener mPreferencesListener;

    private long mLastGloveModeUpdate = 0;
    private boolean mGloveModeEnabled = false;

    public void onCreate() {
        if (DEBUG) Log.d(TAG, "Creating service");

        super.onCreate();
        mContext = this;
    }

    public int onStartCommand(Intent intent, int flags, int startId) {
        if (DEBUG) Log.d(TAG, "Starting service");

        IntentFilter intentScreen = new IntentFilter(Intent.ACTION_SCREEN_ON);
        intentScreen.addAction(Intent.ACTION_SCREEN_OFF);
        mContext.registerReceiver(mScreenStateReceiver, intentScreen);

        initializeGloveMode();

        getGloveModeEnabled();
        if (DEBUG) Log.d(TAG, "Glove Mode state: " + mGloveModeEnabled);

        updateGloveMode(true);

        mPreferencesListener = new SharedPreferences.OnSharedPreferenceChangeListener() {
            public void onSharedPreferenceChanged(SharedPreferences prefs, String key) {
                getGloveModeEnabled();
                updateGloveMode(true);
            }
        };
        SettingsUtils.registerPreferenceChangeListener(mContext, mPreferencesListener);

        return START_STICKY;
    }

    public void onDestroy() {
        if (DEBUG) Log.d(TAG, "Destroying service");

        SettingsUtils.unregisterPreferenceChangeListener(mContext, mPreferencesListener);

        super.onDestroy();
    }

    public IBinder onBind(Intent intent) {
        return null;
    }

    private void updateGloveMode(boolean configure) {
        if (DEBUG) Log.d(TAG, "Updating Glove Mode: " + mGloveModeEnabled);

        if (!mGloveModeEnabled && !configure) {
            return;
        }

        long currentTimeMs = System.currentTimeMillis();
        if (currentTimeMs - mLastGloveModeUpdate > GLOVE_UPDATE_DELTA || configure) {
            mLastGloveModeUpdate = currentTimeMs;
            try {
                FileOutputStream fos = new FileOutputStream(GLOVE_PATH);
                fos.write(String.valueOf(mGloveModeEnabled ? 0 : 1).getBytes());
                fos.flush();
                fos.close();
            } catch (IOException e) {
                Log.e(TAG, "Could not write to file " + GLOVE_PATH, e);
            }
        }
    }

    private BroadcastReceiver mScreenStateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(Intent.ACTION_SCREEN_OFF)) {
                getGloveModeEnabled();
                if (DEBUG) Log.d(TAG, "Glove Mode state: " + mGloveModeEnabled);
            } else if (intent.getAction().equals(Intent.ACTION_SCREEN_ON)) {
                updateGloveMode(false);
            }
        }
    };

    private void getGloveModeEnabled() {
        mGloveModeEnabled = SettingsUtils.getIntSystem(mContext, mContext.getContentResolver(),
                SettingsUtils.HIGH_TOUCH_SENSITIVITY_ENABLE, 1) == 1;
    }

    private void initializeGloveMode() {
        SharedPreferences settings = getSharedPreferences(PREFERENCES, 0);
        boolean initiated = settings.getBoolean("glovemode-initiated", false);

        if (!initiated && SettingsUtils.putIntSystem(mContext, mContext.getContentResolver(),
                SettingsUtils.HIGH_TOUCH_SENSITIVITY_ENABLE, 1)) {
            Log.d(TAG, "GloveMode has been enabled by default");
            SharedPreferences.Editor editor = settings.edit();
            editor.putBoolean("glovemode-initiated", true);
            editor.commit();
        }
    }
}
