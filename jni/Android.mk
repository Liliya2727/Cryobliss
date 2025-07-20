LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := Cryx
LOCAL_SRC_FILES := \
    src/freq.c \
    main.c \
    src/cmd_utils.c \
    src/Cryx_log.c \
    src/file_utils.c \
    src/misc_utils.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_CFLAGS := -DNDEBUG -Wall -Wextra -Werror \
                -pedantic-errors -Wpedantic \
                -O2 -std=c23 -fPIC -flto

LOCAL_LDFLAGS := -flto
LOCAL_LDLIBS  += -llog  

include $(BUILD_EXECUTABLE)