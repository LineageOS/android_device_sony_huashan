# SHIM
TARGET_LD_SHIM_LIBS := \
    /system/bin/mediaserver|libshim_unwind.so \
    /system/lib/libcald_pal.so|libshim_cald.so \
    /system/lib/hw/camera.vendor.qcom.so|libshim_camera.so
