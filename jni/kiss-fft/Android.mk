LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)

LOCAL_ARM_MODE  := arm
LOCAL_MODULE    := kissfft


LOCAL_SRC_FILES := kiss_fft.c kiss_fftr.c

LOCAL_SHARED_LIBRARIES := libkiss_fft

LOCAL_CFLAGS += -O4 -std=c99

LOCAL_LDLIBS := -llog -landroid

include $(BUILD_SHARED_LIBRARY) 
