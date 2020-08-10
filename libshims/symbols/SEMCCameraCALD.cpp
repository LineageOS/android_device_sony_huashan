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

#include <string.h>

#include <cstdarg>
#include <cstdio>

#include <android/log.h>

extern "C" void PAL_LogPrint(const char* path, int line, unsigned int level, unsigned int filter, const char *format, ...)
{
    char* file;
    char str[1024];
    va_list args;

    file = strrchr((char*)path, '/');
    file = file ? file + 1 : (char*)path;

    va_start(args, format);
    vsprintf(str, format, args);
    va_end(args);

    __android_log_print(LOG_ID_CRASH, "caladbolgSHIM", "%40s | @ %-6d | %10d / %-3d | %s", file, line, level, filter, str);
}
