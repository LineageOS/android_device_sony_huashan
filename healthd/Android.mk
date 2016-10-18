# Copyright 2013 The Android Open Source Project
# Copyright 2015 The CyanogenMod Project
# Copyright 2017 The LineageOS Project

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := healthd_board_msm.cpp
LOCAL_MODULE := libhealthd.board
LOCAL_WHOLE_STATIC_LIBRARIES := libcutils
LOCAL_C_INCLUDES := system/core/healthd/include
LOCAL_CFLAGS := -Werror
include $(BUILD_STATIC_LIBRARY)
