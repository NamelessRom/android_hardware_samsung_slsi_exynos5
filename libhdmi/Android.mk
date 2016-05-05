# Copyright (C) 2008 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false
LOCAL_SHARED_LIBRARIES := liblog libutils libcutils libexynosutils libexynosv4l2 libhwcutils libdisplay libhwcutilsmodule libmpp libsync

ifeq ($(BOARD_USES_GSC_VIDEO), true)
	LOCAL_CFLAGS += -DGSC_VIDEO
endif

ifeq ($(BOARD_USES_VIRTUAL_DISPLAY), true)
	LOCAL_CFLAGS += -DUSES_VIRTUAL_DISPLAY
endif

LOCAL_CFLAGS += -DLOG_TAG=\"hdmi\"

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../include \
	$(LOCAL_PATH)/../libhwcutils \
	$(LOCAL_PATH)/../libdisplay \
	$(LOCAL_PATH)/../libhwc \
	$(LOCAL_PATH)/../libexynosutils \
	$(LOCAL_PATH)/../libhwcmodule \
	$(LOCAL_PATH)/../libhwcutilsmodule \
	$(LOCAL_PATH)/../libmpp

LOCAL_SRC_FILES := \
	ExynosExternalDisplay.cpp dv_timings.c

LOCAL_MODULE_TAGS := eng
LOCAL_MODULE := libhdmi
include $(BUILD_SHARED_LIBRARY)

