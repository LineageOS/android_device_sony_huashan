/*
 * Copyright (C) 2020 The LineageOS Project
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

#include <android/log.h>

#include <utils/String8.h>
#include <utils/String16.h>

/* bool android::checkCallingPermission(const String16& permission) */
extern "C" char _ZN7android22checkCallingPermissionERKNS_8String16E(void* context, const android::String16* __attribute__((unused)) permission)
{
    // Variables
    std::string request = "";

    // Acquire requested permission
    if (context) {
        request = std::string(android::String8(*(const android::String16*)context).string());
    }

    // Allow requested permission
    if (request == "android.permission.CAMERA" || request == "com.sonyericsson.permission.CAMERA_EXTENDED") {
        __android_log_print(LOG_ID_CRASH, "ServiceManager", "Permission forced: %s (SHIM library)", request.c_str());
        return 1;
    }

    // Ignore requested permission
    __android_log_print(LOG_ID_CRASH, "ServiceManager", "Permission ignored: %s (SHIM library)", request.c_str());
    return 0;
}
