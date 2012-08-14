LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := ./include
LOCAL_MODULE	:= actor_dancingparticles
LOCAL_SRC_FILES := $(addprefix /, $(notdir $(wildcard $(LOCAL_PATH)/*.c) $(wildcard $(LOCAL_PATH)/*.cpp)))
LOCAL_CFLAGS    += $(ARCH_CFLAGS) -fexceptions -frtti
LOCAL_SHARED_LIBRARIES := gnustl visual GLU
LOCAL_STATIC_LIBRARIES := t1lib
LOCAL_ARM_MODE := arm
LOCAL_LDLIBS += -lGLESv1_CM
include $(BUILD_SHARED_LIBRARY)



