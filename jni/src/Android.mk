LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../SDL2

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include \
	$(LOCAL_PATH)/../libmpg123/src \
	$(LOCAL_PATH)/../SDL2_image-2.0.0 \
	$(LOCAL_PATH)/../SDL2_ttf-2.0.12 \
	$(LOCAL_PATH)/../SDL2_mixer-2.0.0 \
	$(LOCAL_PATH)/../kiss-fft \
	$(LOCAL_PATH)/../libvorbisidec-1.2.1

# Add your application source files here...
LOCAL_SRC_FILES := $(SDL_PATH)/src/main/android/SDL_android_main.c \
	freq_an.cpp \
	music_circle.cpp \
	my_sdl_func.cpp

LOCAL_SHARED_LIBRARIES := SDL2 SDL2_image SDL2_mixer SDL2_ttf mpg123 kissfft

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog

include $(BUILD_SHARED_LIBRARY)
