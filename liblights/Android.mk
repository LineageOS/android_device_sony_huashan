ifeq ($(TARGET_PROVIDES_LIBLIGHT),true)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    lights.cpp

LOCAL_SHARED_LIBRARIES := \
    libhardware \
    liblog \
    libutils \
    libcutils

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_MODULE := lights.msm8960

LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS += -Wunused-variable
ifneq ($(BOARD_AOSP_BASED),)
    LOCAL_CFLAGS += -DBOARD_AOSP_BASED
endif

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
    external/tinyalsa/include

LOCAL_SRC_FILES := \
    lights_effects.cpp

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libtinyalsa

LOCAL_MODULE := lights_effects

LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS += -Wunused-variable

include $(BUILD_EXECUTABLE)

endif
