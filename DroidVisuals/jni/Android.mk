
WARNING_CFLAGS 		:= -Wall -Wstrict-aliasing -Wcast-align -Waddress -Wmissing-braces -Wimplicit -Wunused -Wno-unused-variable

LV_C_INCLUDES  		:= jni/includes/ 
LV_CFLAGS      		:= $(WARNING_CFLAGS)
LV_CXXFLAGS    		:=
LV_CPPFLAGS    		:= -std=gnu++0x -fexceptions
LV_SHARED_LIBRARIES	:= 
LV_STATIC_LIBRARIES	:=
LV_LDFLAGS              :=
LV_LDLIBS               :=


ifeq ($(TARGET_ARCH_ABI),armeabi)
endif
        
ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
    LV_CFLAGS += -DHAVE_NEON=1 -mfloat-abi=softfp -mfpu=neon -ftree-vectorize -ftree-vectorizer-verbose=5
endif
            
ifeq ($(TARGET_ARCH_ABI),x86)
endif
                
                
include $(call all-subdir-makefiles)
