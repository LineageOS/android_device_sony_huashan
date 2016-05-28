/*
 * Copyright (c) 2017 The LineageOS Project
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

import android.content.ContentResolver;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.util.Log;

import java.lang.reflect.Field;
import java.lang.reflect.Method;

public class SettingsUtils {

    public static final String TAG = "SettingsUtils";

    public static final String PREFERENCES = "SettingsUtilsPreferences";
    public static final String SETTINGS_CLASS = "cyanogenmod.providers.CMSettings$System";

    public static final String HIGH_TOUCH_SENSITIVITY_ENABLE =
            "HIGH_TOUCH_SENSITIVITY_ENABLE";
    public static final String TOUCHSCREEN_GESTURE_HAPTIC_FEEDBACK =
            "TOUCHSCREEN_GESTURE_HAPTIC_FEEDBACK";

    public static int getInt(Context context, ContentResolver cr, String name, int def) {
        int ret;

        try {
            Class systemSettings = Class.forName(SETTINGS_CLASS);
            Method getInt = systemSettings.getMethod("getInt", ContentResolver.class,
                    String.class, int.class);
            String sdkName = (String)systemSettings.getDeclaredField(name).get(null);
            ret = (int)getInt.invoke(systemSettings, cr, sdkName, def);
        } catch (Exception e) {
            Log.i(TAG, "CMSettings not found. Using application settings for getInt");
            SharedPreferences settings = context.getSharedPreferences(PREFERENCES, 0);
            ret = settings.getInt(name, def);
        }

        return ret;
    }

    public static boolean putInt(Context context, ContentResolver cr, String name, int value) {
        boolean ret;

        try {
            Class systemSettings = Class.forName(SETTINGS_CLASS);
            Method putInt = systemSettings.getMethod("putInt", ContentResolver.class,
                    String.class, int.class);
            String sdkName = (String)systemSettings.getDeclaredField(name).get(null);
            ret = (boolean)putInt.invoke(systemSettings, cr, sdkName, value);
        } catch (Exception e) {
            Log.i(TAG, "CMSettings not found. Using application settings for putInt");
            SharedPreferences settings = context.getSharedPreferences(PREFERENCES, 0);
            SharedPreferences.Editor editor = settings.edit();
            editor.putInt(name, value);
            ret = editor.commit();
        }

        return ret;
    }
}
