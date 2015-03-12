
###############################################################################
#
# THIS FILE IS AUTOGENERATED BY GYP_TO_ANDROID.PY. DO NOT EDIT.
#
# For bugs, please contact scroggo@google.com or djsollen@google.com
#
###############################################################################

BASE_PATH := $(call my-dir)
LOCAL_PATH:= $(call my-dir)

###############################################################################
#
# PROBLEMS WITH SKIA DEBUGGING?? READ THIS...
#
# The debug build results in changes to the Skia headers. This means that those
# using libskia must also be built with the debug version of the Skia headers.
# There are a few scenarios where this comes into play:
#
# (1) You're building debug code that depends on libskia.
#   (a) If libskia is built in release, then define SK_RELEASE when building
#       your sources.
#   (b) If libskia is built with debugging (see step 2), then no changes are
#       needed since your sources and libskia have been built with SK_DEBUG.
# (2) You're building libskia in debug mode.
#   (a) RECOMMENDED: You can build the entire system in debug mode. Do this by
#       updating your build/core/config.mk to include -DSK_DEBUG on the line
#       that defines COMMON_GLOBAL_CFLAGS
#   (b) You can update all the users of libskia to define SK_DEBUG when they are
#       building their sources.
#
# NOTE: If neither SK_DEBUG or SK_RELEASE are defined then Skia checks NDEBUG to
#       determine which build type to use.
###############################################################################

include $(CLEAR_VARS)
LOCAL_FDO_SUPPORT := true
ifneq ($(strip $(TARGET_FDO_CFLAGS)),)
	# This should be the last -Oxxx specified in LOCAL_CFLAGS
	LOCAL_CFLAGS += -O2
endif

LOCAL_ARM_MODE := thumb
ifeq ($(TARGET_ARCH),arm)
	ifeq ($(ARCH_ARM_HAVE_VFP),true)
		LOCAL_CFLAGS += -DANDROID_LARGE_MEMORY_DEVICE
	endif
else
	LOCAL_CFLAGS += -DANDROID_LARGE_MEMORY_DEVICE
endif

# used for testing
#LOCAL_CFLAGS += -g -O0

ifeq ($(NO_FALLBACK_FONT),true)
	LOCAL_CFLAGS += -DNO_FALLBACK_FONT
endif

LOCAL_CFLAGS += \
	local_cflags

LOCAL_CPPFLAGS := \
	local_cppflags

LOCAL_SRC_FILES := \
	local_src_files

LOCAL_SHARED_LIBRARIES := \
	local_shared_libraries

LOCAL_STATIC_LIBRARIES := \
	local_static_libraries

LOCAL_C_INCLUDES := \
	local_c_includes

LOCAL_EXPORT_C_INCLUDE_DIRS := \
	local_export_c_include_dirs

LOCAL_CFLAGS += \
	-Ddefines

LOCAL_MODULE_TAGS := \
	local_module_tags

LOCAL_MODULE := \
	local_module

ifeq ($(COND), true)
LOCAL_CFLAGS_foo += \
	local_cflags_foo

LOCAL_CPPFLAGS_foo += \
	local_cppflags_foo

LOCAL_SRC_FILES_foo += \
	local_src_files_foo

LOCAL_SHARED_LIBRARIES_foo += \
	local_shared_libraries_foo

LOCAL_STATIC_LIBRARIES_foo += \
	local_static_libraries_foo

LOCAL_C_INCLUDES_foo += \
	local_c_includes_foo

LOCAL_EXPORT_C_INCLUDE_DIRS_foo += \
	local_export_c_include_dirs_foo

LOCAL_CFLAGS_foo += \
	-Ddefines_foo

LOCAL_MODULE_TAGS_foo += \
	local_module_tags_foo

LOCAL_MODULE_foo += \
	local_module_foo

endif

LOCAL_CFLAGS_bar += \
	local_cflags_bar

LOCAL_CPPFLAGS_bar += \
	local_cppflags_bar

LOCAL_SRC_FILES_bar += \
	local_src_files_bar

LOCAL_SHARED_LIBRARIES_bar += \
	local_shared_libraries_bar

LOCAL_STATIC_LIBRARIES_bar += \
	local_static_libraries_bar

LOCAL_C_INCLUDES_bar += \
	local_c_includes_bar

LOCAL_EXPORT_C_INCLUDE_DIRS_bar += \
	local_export_c_include_dirs_bar

LOCAL_CFLAGS_bar += \
	-Ddefines_bar

LOCAL_MODULE_TAGS_bar += \
	local_module_tags_bar

LOCAL_MODULE_bar += \
	local_module_bar

include $(BUILD_SHARED_LIBRARY)

#############################################################
# Build the skia tools
#

# benchmark (timings)
include $(BASE_PATH)/bench/Android.mk

# diamond-master (one test to rule them all)
include $(BASE_PATH)/dm/Android.mk
