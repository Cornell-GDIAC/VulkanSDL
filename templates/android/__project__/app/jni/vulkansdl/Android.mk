LOCAL_PATH  := $(call my-dir)
CURR_DEPTH  := ..
SDL3_OFFSET := __SDL3_PATH__

SDL3_BUILD_STATIC := true
SDL3_BUILD_SHARED := false

# Access the buildfiles folder, avoiding long paths
SDL3_PATH := $(LOCAL_PATH)/$(CURR_DEPTH)/$(SDL3_OFFSET)
SDL_MAKE := $(SDL3_PATH)/buildfiles/android/jni/sdl3/Android.mk
IMG_MAKE := $(SDL3_PATH)/buildfiles/android/jni/sdl3app/Android.mk
TTF_MAKE := $(SDL3_PATH)/buildfiles/android/jni/sdl3image/Android.mk
APP_MAKE := $(SDL3_PATH)/buildfiles/android/jni/sdl3ttf/Android.mk
include $(SDL_MAKE)
include $(IMG_MAKE)
include $(TTF_MAKE)
include $(APP_MAKE)

