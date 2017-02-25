# TWRP (Optional)
ifeq ($(WITH_TWRP),true)

# Recovery files
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/rootdir/twrp.fstab:recovery/root/etc/twrp.fstab

endif
