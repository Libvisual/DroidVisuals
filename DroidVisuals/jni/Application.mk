APP_ABI := armeabi-v7a
APP_PLATFORM := android-15
APP_STL := gnustl_shared
#STLPORT_FORCE_REBUILD := true
APP_OPTIM := $(BUILD_TYPE)
APP_CPPFLAGS := -std=gnu++0x 
APP_CFLAGS := -Iinclude/ -Wno-psabi -DBUILD_GLU32 -DUSE_OPENGL_ES
