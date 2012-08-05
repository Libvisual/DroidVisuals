LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := ./include
LOCAL_MODULE	:= morph_slide_up
LOCAL_SRC_FILES := morph_slide_up.c common.c
LOCAL_CFLAGS    += $(ARCH_CFLAGS) -fexceptions -frtti
LOCAL_SHARED_LIBRARIES := gnustl visual
LOCAL_ARM_MODE := arm
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := ./include
LOCAL_MODULE	:= morph_slide_down
LOCAL_SRC_FILES := morph_slide_down.c common.c
LOCAL_CFLAGS    += $(ARCH_CFLAGS) -fexceptions -frtti
LOCAL_SHARED_LIBRARIES := gnustl visual
LOCAL_ARM_MODE := arm
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := ./include
LOCAL_MODULE	:= morph_slide_left
LOCAL_SRC_FILES := morph_slide_left.c common.c
LOCAL_CFLAGS    += $(ARCH_CFLAGS) -fexceptions -frtti
LOCAL_SHARED_LIBRARIES := gnustl visual
LOCAL_ARM_MODE := arm
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := ./include
LOCAL_MODULE	:= morph_slide_right
LOCAL_SRC_FILES := morph_slide_right.c common.c
LOCAL_CFLAGS    += $(ARCH_CFLAGS) -fexceptions -frtti
LOCAL_SHARED_LIBRARIES := gnustl visual
LOCAL_ARM_MODE := arm
include $(BUILD_SHARED_LIBRARY)


CMakeLists.txt  common.h            morph_slide_left.c   morph_slide_up.c
common.c        morph_slide_down.c  morph_slide_right.c


