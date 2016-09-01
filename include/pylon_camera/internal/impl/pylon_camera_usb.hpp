/******************************************************************************
 * Software License Agreement (BSD License)
 *
 * Copyright (C) 2016, Magazino GmbH. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the names of Magazino GmbH nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#ifndef PYLON_CAMERA_INTERNAL_USB_H_
#define PYLON_CAMERA_INTERNAL_USB_H_

#include <string>
#include <vector>

#include <pylon_camera/internal/pylon_camera.h>

#include <pylon/usb/BaslerUsbInstantCamera.h>

namespace pylon_camera
{

struct USBCameraTrait
{
    typedef Pylon::CBaslerUsbInstantCamera CBaslerInstantCameraT;
    typedef Basler_UsbCameraParams::ExposureAutoEnums ExposureAutoEnums;
    typedef Basler_UsbCameraParams::GainAutoEnums GainAutoEnums;
    typedef Basler_UsbCameraParams::PixelFormatEnums PixelFormatEnums;
    typedef Basler_UsbCameraParams::PixelSizeEnums PixelSizeEnums;
    typedef GenApi::IFloat AutoTargetBrightnessType;
    typedef GenApi::IFloat GainType;
    typedef double AutoTargetBrightnessValueType;
    typedef Basler_UsbCameraParams::ShutterModeEnums ShutterModeEnums;
    typedef Basler_UsbCameraParams::UserOutputSelectorEnums UserOutputSelectorEnums;

    static inline AutoTargetBrightnessValueType convertBrightness(const int& value)
    {
        return value / 255.0;
    }
};

typedef PylonCameraImpl<USBCameraTrait> PylonUSBCamera;

template <>
bool PylonUSBCamera::applyCamSpecificStartupSettings(const PylonCameraParameter& parameters)
{
    try
    {
        // Remove all previous settings (sequencer etc.)
        // Default Setting = Free-Running
        cam_->UserSetSelector.SetValue(Basler_UsbCameraParams::UserSetSelector_Default);
        cam_->UserSetLoad.Execute();
        // UserSetSelector_Default overrides Software Trigger Mode !!
        cam_->TriggerSource.SetValue(Basler_UsbCameraParams::TriggerSource_Software);
        cam_->TriggerMode.SetValue(Basler_UsbCameraParams::TriggerMode_On);

         /* Thresholds for the AutoExposure Funcitons:
          *  - lower limit can be used to get rid of changing light conditions
          *    due to 50Hz lamps (-> 20ms cycle duration)
          *  - upper limit is to prevent motion blur
          */
        cam_->AutoExposureTimeLowerLimit.SetValue(cam_->ExposureTime.GetMin());
        cam_->AutoExposureTimeUpperLimit.SetValue(cam_->ExposureTime.GetMax());

        cam_->AutoGainLowerLimit.SetValue(cam_->Gain.GetMin());
        cam_->AutoGainUpperLimit.SetValue(cam_->Gain.GetMax());

        // If using USB rgb8 camera, set following additional parameters.
        if (parameters.imageEncoding() == "rgb8")
        {
            if (parameters.light_source_preset_ == Basler_UsbCameraParams::LightSourcePreset_Off)
            {
                cam_->LightSourcePreset.SetValue(Basler_UsbCameraParams::LightSourcePreset_Off);
            }
            else if (parameters.light_source_preset_ == Basler_UsbCameraParams::LightSourcePreset_Daylight5000K)
            {
                cam_->LightSourcePreset.SetValue(Basler_UsbCameraParams::LightSourcePreset_Daylight5000K);
            }
            else if (parameters.light_source_preset_ == Basler_UsbCameraParams::LightSourcePreset_Daylight6500K)
            {
                cam_->LightSourcePreset.SetValue(Basler_UsbCameraParams::LightSourcePreset_Daylight6500K);
            } else if (parameters.light_source_preset_ == Basler_UsbCameraParams::LightSourcePreset_Tungsten2800K)
            {
                cam_->LightSourcePreset.SetValue(Basler_UsbCameraParams::LightSourcePreset_Tungsten2800K);
            }
            else
            {
                ROS_WARN("Light source preset has not valid value.");
            }

            if (parameters.balance_white_auto_ == Basler_UsbCameraParams::BalanceWhiteAuto_Off)
            {
                cam_->BalanceWhiteAuto.SetValue(Basler_UsbCameraParams::BalanceWhiteAuto_Off);
            }
            else if (parameters.balance_white_auto_ == Basler_UsbCameraParams::BalanceWhiteAuto_Once)
            {
                cam_->BalanceWhiteAuto.SetValue(Basler_UsbCameraParams::BalanceWhiteAuto_Once);
            }
            else if (parameters.balance_white_auto_ == Basler_UsbCameraParams::BalanceWhiteAuto_Continuous)
            {
                cam_->BalanceWhiteAuto.SetValue(Basler_UsbCameraParams::BalanceWhiteAuto_Continuous);
            }
            else
            {
                ROS_WARN("Balance white auto has not valid value.");
            }

            if (parameters.balance_ratio_selector_ == Basler_UsbCameraParams::BalanceRatioSelector_Red)
            {
                cam_->BalanceRatioSelector.SetValue(Basler_UsbCameraParams::BalanceRatioSelector_Red);
            }
            else if (parameters.balance_ratio_selector_ == Basler_UsbCameraParams::BalanceRatioSelector_Green)
            {
                cam_->BalanceRatioSelector.SetValue(Basler_UsbCameraParams::BalanceRatioSelector_Green);
            }
            else if (parameters.balance_ratio_selector_ == Basler_UsbCameraParams::BalanceRatioSelector_Blue)
            {
                cam_->BalanceRatioSelector.SetValue(Basler_UsbCameraParams::BalanceRatioSelector_Blue);
            }
            else
            {
                ROS_WARN("Balance ratio selector has not valid value.");
            }

            cam_->BalanceRatio.SetValue(parameters.balance_ratio_);

            if (parameters.overlap_mode_on_)
            {
                cam_->OverlapMode.SetIntValue(Basler_UsbCameraParams::OverlapMode_On);
            }
            else
            {
                cam_->OverlapMode.SetIntValue(Basler_UsbCameraParams::OverlapMode_Off);
            }
        }

        // The gain auto function and the exposure auto function can be used at the same time. In this case,
        // however, you must also set the Auto Function Profile feature.
        //  cam_->AutoFunctionProfile.SetValue(Basler_UsbCameraParams::AutoFunctionProfile_MinimizeGain);

        if ( GenApi::IsAvailable(cam_->BinningHorizontal) &&
             GenApi::IsAvailable(cam_->BinningVertical) )
        {
            ROS_INFO_STREAM("Cam has binning range: x(hz) = ["
                    << cam_->BinningHorizontal.GetMin() << " - "
                    << cam_->BinningHorizontal.GetMax() << "], y(vt) = ["
                    << cam_->BinningVertical.GetMin() << " - "
                    << cam_->BinningVertical.GetMax() << "].");
        }
        else
        {
            ROS_INFO_STREAM("Cam does not support binning.");
        }
        ROS_INFO_STREAM("Cam has exposure time range: [" << cam_->ExposureTime.GetMin()
                << " - " << cam_->ExposureTime.GetMax()
                << "] measured in microseconds.");
        ROS_INFO_STREAM("Cam has gain range: [" << cam_->Gain.GetMin()
                << " - " << cam_->Gain.GetMax()
                << "] measured in dB.");
        ROS_INFO_STREAM("Cam has gammma range: ["
                << cam_->Gamma.GetMin() << " - "
                << cam_->Gamma.GetMax() << "].");
        ROS_INFO_STREAM("Cam has pylon auto brightness range: ["
                << cam_->AutoTargetBrightness.GetMin() * 255 << " - "
                << cam_->AutoTargetBrightness.GetMax() * 255
                << "] which is the average pixel intensity.");
    }
    catch ( const GenICam::GenericException &e )
    {
        ROS_ERROR_STREAM("Error applying cam specific startup setting for USB cameras: "
                << e.GetDescription());
        return false;
    }
    return true;
}

template <>
bool PylonUSBCamera::setImageEncoding(const PylonCameraParameter& parameters)
{
    try
    {
        if (parameters.imageEncoding() == "rgb8")
        {
            cam_->PixelFormat.SetValue(PixelFormatEnums::PixelFormat_RGB8);
        }
        else
        {
            cam_->PixelFormat.SetValue(PixelFormatEnums::PixelFormat_Mono8);
        }
        return true;
    }
    catch ( const GenICam::GenericException &e )
    {
        ROS_ERROR_STREAM("Error applying image encoding to USB cameras: "
                << e.GetDescription());
        return false;
    }
}

template <>
std::string PylonUSBCamera::imageEncoding() const
{
    switch ( image_encoding_ )
    {
        case PixelFormatEnums::PixelFormat_Mono8:
            return sensor_msgs::image_encodings::MONO8;

        case PixelFormatEnums::PixelFormat_RGB8:
            return sensor_msgs::image_encodings::RGB8;

        default:
            throw std::runtime_error("Currently, only mono8 and rgb8 cameras are supported");
    }
}

template <>
bool PylonUSBCamera::setupSequencer(const std::vector<float>& exposure_times,
                                    std::vector<float>& exposure_times_set)
{
    try
    {
        // Runtime Sequencer: cam_->IsGrabbing() ? cam_->StopGrabbing(); //10ms
        if ( GenApi::IsWritable(cam_->SequencerMode) )
        {
            cam_->SequencerMode.SetValue(Basler_UsbCameraParams::SequencerMode_Off);
        }
        else
        {
            ROS_ERROR("Sequencer Mode not writable");
        }

        cam_->SequencerConfigurationMode.SetValue(Basler_UsbCameraParams::SequencerConfigurationMode_On);

        // **** valid for all sets: reset on software signal 1 ****
        int64_t initial_set = cam_->SequencerSetSelector.GetMin();

        cam_->SequencerSetSelector.SetValue(initial_set);
        cam_->SequencerPathSelector.SetValue(0);
        cam_->SequencerSetNext.SetValue(initial_set);
        cam_->SequencerTriggerSource.SetValue(Basler_UsbCameraParams::SequencerTriggerSource_SoftwareSignal1);
        // advance on Frame Start
        cam_->SequencerPathSelector.SetValue(1);
        cam_->SequencerTriggerSource.SetValue(Basler_UsbCameraParams::SequencerTriggerSource_FrameStart);
        // ********************************************************

        for ( std::size_t i = 0; i < exposure_times.size(); ++i )
        {
            if ( i > 0 )
            {
                cam_->SequencerSetSelector.SetValue(i);
            }

            if ( i == exposure_times.size() - 1 )  // last frame
            {
                cam_->SequencerSetNext.SetValue(0);
            }
            else
            {
                cam_->SequencerSetNext.SetValue(i + 1);
            }
            float reached_exposure;
            setExposure(exposure_times.at(i), reached_exposure);
            exposure_times_set.push_back(reached_exposure / 1000000.);
            cam_->SequencerSetSave.Execute();
        }

        // config finished
        cam_->SequencerConfigurationMode.SetValue(Basler_UsbCameraParams::SequencerConfigurationMode_Off);
        cam_->SequencerMode.SetValue(Basler_UsbCameraParams::SequencerMode_On);
    }
    catch ( const GenICam::GenericException &e )
    {
        ROS_ERROR_STREAM("ERROR while initializing pylon sequencer: "
                << e.GetDescription());
        return false;
    }
    return true;
}

template <>
GenApi::IFloat& PylonUSBCamera::exposureTime()
{
    if ( GenApi::IsAvailable(cam_->ExposureTime) )
    {
        return cam_->ExposureTime;
    }
    else
    {
        throw std::runtime_error("Error while accessing ExposureTime in PylonUSBCamera");
    }
}

template <>
USBCameraTrait::GainType& PylonUSBCamera::gain()
{
    if ( GenApi::IsAvailable(cam_->Gain) )
    {
        return cam_->Gain;
    }
    else
    {
        throw std::runtime_error("Error while accessing Gain in PylonUSBCamera");
    }
}

template <>
GenApi::IFloat& PylonUSBCamera::autoExposureTimeLowerLimit()
{
    if ( GenApi::IsAvailable(cam_->AutoExposureTimeLowerLimit) )
    {
        return cam_->AutoExposureTimeLowerLimit;
    }
    else
    {
        throw std::runtime_error("Error while accessing AutoExposureTimeLowerLimit in PylonUSBCamera");
    }
}

template <>
GenApi::IFloat& PylonUSBCamera::autoExposureTimeUpperLimit()
{
    if ( GenApi::IsAvailable(cam_->AutoExposureTimeUpperLimit) )
    {
        return cam_->AutoExposureTimeUpperLimit;
    }
    else
    {
        throw std::runtime_error("Error while accessing AutoExposureTimeUpperLimit in PylonUSBCamera");
    }
}

template <>
USBCameraTrait::GainType& PylonUSBCamera::autoGainLowerLimit()
{
    if ( GenApi::IsAvailable(cam_->AutoGainLowerLimit) )
    {
        return cam_->AutoGainLowerLimit;
    }
    else
    {
        throw std::runtime_error("Error while accessing AutoGainLowerLimit in PylonUSBCamera");
    }
}

template <>
USBCameraTrait::GainType& PylonUSBCamera::autoGainUpperLimit()
{
    if ( GenApi::IsAvailable(cam_->AutoGainUpperLimit) )
    {
        return cam_->AutoGainUpperLimit;
    }
    else
    {
        throw std::runtime_error("Error while accessing AutoGainUpperLimit in PylonUSBCamera");
    }
}

template <>
GenApi::IFloat& PylonUSBCamera::resultingFrameRate()
{
    if ( GenApi::IsAvailable(cam_->ResultingFrameRate) )
    {
        return cam_->ResultingFrameRate;
    }
    else
    {
        throw std::runtime_error("Error while accessing ResultingFrameRate in PylonUSBCamera");
    }
}

template <>
USBCameraTrait::AutoTargetBrightnessType& PylonUSBCamera::autoTargetBrightness()
{
    if ( GenApi::IsAvailable(cam_->AutoTargetBrightness) )
    {
        return cam_->AutoTargetBrightness;
    }
    else
    {
        throw std::runtime_error("Error while accessing AutoTargetBrightness in PylonUSBCamera");
    }
}

template <>
std::string PylonUSBCamera::typeName() const
{
    return "USB";
}

}  // namespace pylon_camera

#endif  // PYLON_CAMERA_INTERNAL_USB_H_
