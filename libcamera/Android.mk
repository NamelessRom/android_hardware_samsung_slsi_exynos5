# Copyright (C) 2012 The Android Open Source Project
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

ifeq ($(TARGET_BOARD_PLATFORM), exynos5)

LOCAL_PATH:= $(call my-dir)

#################
# libexynoscamera

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SHARED_LIBRARIES:= libutils libcutils libc++ libbinder liblog libcamera_client libhardware
LOCAL_SHARED_LIBRARIES += libexynosutils libhwjpeg libexynosv4l2 libexynosgscaler libion_exynos libcsc
LOCAL_SHARED_LIBRARIES += libexpat libc++ #libstlport
LOCAL_SHARED_LIBRARIES += libpower

LOCAL_CFLAGS += -DGAIA_FW_BETA
#LOCAL_CFLAGS += -DMAIN_CAMERA_SENSOR_NAME=$(BOARD_BACK_CAMERA_SENSOR)
#LOCAL_CFLAGS += -DFRONT_CAMERA_SENSOR_NAME=$(BOARD_FRONT_CAMERA_SENSOR)
ifeq ($(BOARD_CAMERA_DISPLAY_WQHD), true)
	LOCAL_CFLAGS += -DCAMERA_DISPLAY_WQHD
endif
LOCAL_CFLAGS += -DUSE_CAMERA_ESD_RESET
LOCAL_CFLAGS += -DBACK_ROTATION=$(BOARD_BACK_CAMERA_ROTATION)
LOCAL_CFLAGS += -DFRONT_ROTATION=$(BOARD_FRONT_CAMERA_ROTATION)

LOCAL_C_INCLUDES += \
	$(TOP)/system/core/include \
	$(TOP)/hardware/libhardware_legacy/include/hardware_legacy \
	$(TOP)/bionic \
	$(TOP)/external/expat/lib \
	$(TOP)/system/media/camera/include \
	$(LOCAL_PATH)/../libcamera \
	$(LOCAL_PATH)/54xx \
	$(LOCAL_PATH)/../include \
	$(LOCAL_PATH)/54xx/JpegEncoderForCamera \
	$(LOCAL_PATH)/common \
	$(LOCAL_PATH)/common/Pipes \
	$(LOCAL_PATH)/common/Activities \
	$(LOCAL_PATH)/common/Buffers \
	$(LOCAL_PATH)/../libcamera/Vendor
#	$(TOP)/external/stlport/stlport \
#	$(TOP)/hardware/samsung_slsi/exynos/include \
#	$(TOP)/hardware/samsung_slsi/$(TARGET_SOC)/include \
#	$(TOP)/hardware/samsung_slsi/$(TARGET_BOARD_PLATFORM)/include \
#	$(TOP)/hardware/samsung_slsi/$(TARGET_BOARD_PLATFORM)/libcamera \
#	$(TOP)/vendor/samsung/feature/CscFeature/libsecnativefeature \

LOCAL_SRC_FILES:= \
	ExynosCameraSensorInfo.cpp \
	common/ExynosCameraFrame.cpp \
	common/ExynosCameraMemory.cpp \
	common/ExynosCameraUtils.cpp \
	common/ExynosCameraNode.cpp \
	common/ExynosCameraFrameSelector.cpp \
	common/Pipes/ExynosCameraPipe.cpp \
	common/Pipes/ExynosCameraPipeFlite.cpp \
	common/Pipes/ExynosCameraPipe3AA_ISP.cpp \
	common/Pipes/ExynosCameraPipeSCC.cpp \
	common/Pipes/ExynosCameraPipeSCP.cpp \
	common/Pipes/ExynosCameraPipeGSC.cpp \
	common/Pipes/ExynosCameraPipeJpeg.cpp \
	common/Pipes/ExynosCameraPipe3AA.cpp \
	common/Pipes/ExynosCameraPipe3AC.cpp \
	common/Pipes/ExynosCameraPipeISP.cpp \
	common/Buffers/ExynosCameraBufferManager.cpp \
	common/Buffers/ExynosCameraBufferLocker.cpp \
	common/Activities/ExynosCameraActivityBase.cpp \
	common/Activities/ExynosCameraActivityAutofocus.cpp \
	common/Activities/ExynosCameraActivityFlash.cpp \
	common/Activities/ExynosCameraActivitySpecialCapture.cpp \
	common/Activities/ExynosCameraActivityUCTL.cpp \
	54xx/JpegEncoderForCamera/ExynosJpegEncoderForCamera.cpp \
	54xx/ExynosCamera.cpp \
	54xx/ExynosCameraParameters.cpp \
	54xx/ExynosCameraFrameFactory.cpp \
	54xx/ExynosCameraFrameReprocessingFactory.cpp \
	54xx/ExynosCameraFrameFactoryFront.cpp \
	54xx/ExynosCameraActivityControl.cpp\
	54xx/ExynosCameraUtilsModule.cpp \
	54xx/ExynosCameraScalableSensor.cpp

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libexynoscamera

include $(BUILD_SHARED_LIBRARY)

#################
# libcamera

include $(CLEAR_VARS)

# HAL module implemenation stored in
# hw/<COPYPIX_HARDWARE_MODULE_ID>.<ro.product.board>.so
LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../libcamera \
	$(LOCAL_PATH)/../libcamera/Vendor \
	$(LOCAL_PATH)/54xx \
	$(LOCAL_PATH)/../include \
	$(LOCAL_PATH)/54xx/JpegEncoderForCamera \
	$(LOCAL_PATH)/common \
	$(LOCAL_PATH)/common/Pipes \
	$(LOCAL_PATH)/common/Activities \
	$(LOCAL_PATH)/common/Buffers \
	$(TOP)/frameworks/native/include \
	$(TOP)/system/media/camera/include
#	$(TOP)/hardware/samsung_slsi/exynos/include \
#	$(TOP)/hardware/samsung_slsi/$(TARGET_SOC)/include \
#	$(TOP)/hardware/samsung_slsi/$(TARGET_BOARD_PLATFORM)/include \
#	$(TOP)/hardware/samsung_slsi/$(TARGET_BOARD_PLATFORM)/libcamera \

LOCAL_SRC_FILES:= \
	common/ExynosCameraInterface.cpp

LOCAL_CFLAGS += -DGAIA_FW_BETA
LOCAL_CFLAGS += -DBACK_ROTATION=$(BOARD_BACK_CAMERA_ROTATION)
LOCAL_CFLAGS += -DFRONT_ROTATION=$(BOARD_FRONT_CAMERA_ROTATION)

LOCAL_SHARED_LIBRARIES:= libutils libcutils libbinder liblog libcamera_client libhardware
LOCAL_SHARED_LIBRARIES += libexynosutils libhwjpeg libexynosv4l2 libcsc libion_exynos libexynoscamera

LOCAL_MODULE := camera.$(TARGET_BOOTLOADER_BOARD_NAME)

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

endif
