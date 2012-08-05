LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := ./include
LOCAL_MODULE	:= morph_tentacle
LOCAL_SRC_FILES := morph_tentacle.c
LOCAL_CFLAGS    += $(ARCH_CFLAGS) -fexceptions -frtti
LOCAL_SHARED_LIBRARIES := gnustl visual
LOCAL_ARM_MODE := arm
include $(BUILD_SHARED_LIBRARY)



