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

package org.lineageos.settings.device;

import android.os.Bundle;
import android.provider.Settings;
import android.support.v14.preference.PreferenceFragment;
import android.support.v7.preference.Preference;
import android.support.v14.preference.SwitchPreference;

public class GloveModePreferenceFragment extends PreferenceFragment {

    private static final String KEY_GLOVE_MODE_ENABLE = "glove_mode_enable";

    private SwitchPreference mGloveModeEnable;

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        getActivity().getActionBar().setDisplayHomeAsUpEnabled(true);
    }

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        addPreferencesFromResource(R.xml.glove_mode_panel);

        mGloveModeEnable = (SwitchPreference) findPreference(KEY_GLOVE_MODE_ENABLE);
        mGloveModeEnable.setChecked(getGloveModeEnabled());
        mGloveModeEnable.setOnPreferenceChangeListener(mGloveModePrefListener);
    }

    @Override
    public void onResume() {
        super.onResume();
        getListView().setPadding(0, 0, 0, 0);
    }

    private void setGloveModeEnabled(boolean enable) {
        SettingsUtils.putInt(getContext(), SettingsUtils.HIGH_TOUCH_SENSITIVITY_ENABLE,
                enable ? 1 : 0);
    }

    private boolean getGloveModeEnabled() {
        return SettingsUtils.getInt(getContext(),
                SettingsUtils.HIGH_TOUCH_SENSITIVITY_ENABLE, 0) != 0;
    }

    private Preference.OnPreferenceChangeListener mGloveModePrefListener =
        new Preference.OnPreferenceChangeListener() {
        @Override
        public boolean onPreferenceChange(Preference preference, Object value) {
            final String key = preference.getKey();

            if (KEY_GLOVE_MODE_ENABLE.equals(key)) {
                setGloveModeEnabled((boolean)value);
            }

            return true;
        }
    };
}
