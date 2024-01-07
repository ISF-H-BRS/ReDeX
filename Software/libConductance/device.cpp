// ============================================================================================== //
//                                                                                                //
//  This file is part of the ISF ReDeX project.                                                   //
//                                                                                                //
//  Author:                                                                                       //
//  Marcel Hasler <mahasler@gmail.com>                                                            //
//                                                                                                //
//  Copyright (c) 2021 - 2023                                                                     //
//  Bonn-Rhein-Sieg University of Applied Sciences                                                //
//                                                                                                //
//  This program is free software: you can redistribute it and/or modify it under the terms       //
//  of the GNU General Public License as published by the Free Software Foundation, either        //
//  version 3 of the License, or (at your option) any later version.                              //
//                                                                                                //
//  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;     //
//  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.     //
//  See the GNU General Public License for more details.                                          //
//                                                                                                //
//  You should have received a copy of the GNU General Public License along with this program.    //
//  If not, see <https://www.gnu.org/licenses/>.                                                  //
//                                                                                                //
// ============================================================================================== //

#include <conductance/device.h>

#include <libusb-1.0/libusb.h>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <span>
#include <thread>

// ---------------------------------------------------------------------------------------------- //

using namespace isf::Conductance;
using namespace std::chrono_literals;

// ---------------------------------------------------------------------------------------------- //

namespace {
    constexpr int ConfigurationNumber = 1;
    constexpr int InterfaceNumber     = 0;
    constexpr int AlternateSetting    = 0;

    constexpr uint8_t RequestTypeWrite = 0x21; // Type: class, recipient: interface
    constexpr uint8_t RequestTypeRead  = 0xa1;

    constexpr uint8_t BulkInEndpoint = 0x81;

    constexpr unsigned int TimeoutMilliseconds = 500;
}

// ---------------------------------------------------------------------------------------------- //

namespace ControlRequest {
    constexpr uint8_t GetInputMask    = 0x01;
    constexpr uint8_t SetInputCommand = 0x02;
    constexpr uint8_t GetPowerValues  = 0x03;
}

// ---------------------------------------------------------------------------------------------- //

namespace Command {
    constexpr uint32_t SetupSignal = 0x01;
    constexpr uint32_t SetGain     = 0x02;
}

// ---------------------------------------------------------------------------------------------- //

static
auto openDevice(const std::string& serialNumber, libusb_context* context) -> libusb_device_handle*
{
    libusb_device_handle* handle = nullptr;

    libusb_device** list = nullptr;
    ssize_t count = libusb_get_device_list(context, &list);

    std::span<libusb_device*> devices(list, count);

    for (auto device : devices)
    {
        libusb_device_descriptor descriptor = {};

        if (libusb_get_device_descriptor(device, &descriptor) < 0)
            continue;

        if (descriptor.idVendor == Device::VendorId && descriptor.idProduct == Device::ProductId)
        {
            int result = libusb_open(device, &handle);

            if (result == 0)
            {
                std::array<unsigned char, 255> buffer = {};

                result = libusb_get_string_descriptor_ascii(handle, descriptor.iSerialNumber,
                                                            buffer.data(), buffer.size());
                if (result > 0)
                {
                    const std::string serial(reinterpret_cast<char*>(buffer.data()));

                    if (serial == serialNumber)
                        break;
                }
            }

            libusb_close(handle);
            handle = nullptr;
        }
    }

    if (count >= 0)
        libusb_free_device_list(list, true);

    return handle;
}

// ---------------------------------------------------------------------------------------------- //

static
void freeContext(libusb_context* context)
{
    if (context)
        libusb_exit(context);
}

// ---------------------------------------------------------------------------------------------- //

static
void freeHandle(libusb_device_handle* handle)
{
    if (handle)
    {
        libusb_release_interface(handle, InterfaceNumber);
        libusb_close(handle);
    }
}

// ---------------------------------------------------------------------------------------------- //

using ContextGuard = std::unique_ptr<libusb_context, decltype(&freeContext)>;
using HandleGuard = std::unique_ptr<libusb_device_handle, decltype(&freeHandle)>;

// ---------------------------------------------------------------------------------------------- //

class Device::Private
{
public:
    static constexpr size_t TransferBufferSize = 2048; // 64 * 64 bytes >= 2 * 2 * 500 samples
    using TransferBuffer = std::array<uint16_t, TransferBufferSize>;

    std::vector<Listener*> listeners;

    libusb_context* context = nullptr;
    libusb_device_handle* handle = nullptr;

    std::thread workerThread;

    TransferBuffer transferBuffer = {};
    Data data = {};

    bool capturing = false;
};

// ---------------------------------------------------------------------------------------------- //

class DeviceException : public std::runtime_error
{
public:
    DeviceException(std::string msg, int error = 0)
        : std::runtime_error(makeMessage(msg, error)) {}

private:
    static auto makeMessage(std::string msg, int error) -> std::string
    {
        if (error < 0)
        {
            msg += "\n\nResult: " + std::to_string(error) +
                   " (" + std::string(libusb_error_name(error)) +
                   ")\nDetails: " + std::string(libusb_strerror(static_cast<libusb_error>(error)));
        }

        return msg;
    }
};

// ---------------------------------------------------------------------------------------------- //

Device::Device(const DeviceInfo& info)
    : d(std::make_unique<Private>())
{
    // Initialize libusb
    if (libusb_init(&d->context) < 0)
        throw DeviceException("Unable to initialize libusb.");

    ContextGuard context(d->context, freeContext);

    libusb_set_option(d->context, LIBUSB_OPTION_LOG_LEVEL, 3);

    // Try to open the device
    d->handle = openDevice(info.serialNumber(), d->context);

    if (!d->handle)
        throw DeviceException("Unable to connect to device.");

    HandleGuard handle(d->handle, freeHandle);

    // Set a valid configuration
    int result = libusb_set_configuration(d->handle, ConfigurationNumber);

    if (result < 0)
        throw DeviceException("Unable to set a configuration.", result);

    // Claim the interface
    result = libusb_claim_interface(d->handle, InterfaceNumber);

    if (result < 0)
        throw DeviceException("Unable to claim the interface.", result);

    // Set alternate setting
    result = libusb_set_interface_alt_setting(d->handle, InterfaceNumber, AlternateSetting);

    if (result < 0)
        throw DeviceException("Unable to set an alternate setting.", result);

    handle.release();
    context.release();
}

// ---------------------------------------------------------------------------------------------- //

Device::Device() : Device(DeviceInfo::getFirstDevice()) {}

// ---------------------------------------------------------------------------------------------- //

Device::~Device()
{
    stopCapture();

    freeHandle(d->handle);
    freeContext(d->context);
}

// ---------------------------------------------------------------------------------------------- //

void Device::addListener(Listener* listener)
{
    assert(listener != nullptr);

    auto it = std::find(d->listeners.begin(), d->listeners.end(), listener);

    if (it == d->listeners.end())
        d->listeners.push_back(listener);
}

// ---------------------------------------------------------------------------------------------- //

void Device::removeListener(Listener* listener)
{
    auto it = std::find(d->listeners.begin(), d->listeners.end(), listener);

    if (it != d->listeners.end())
        d->listeners.erase(it);
}

// ---------------------------------------------------------------------------------------------- //

auto Device::getInputsConnected() const -> std::array<bool, InputCount>
{
    uint8_t mask = 0x00;

    int result = libusb_control_transfer(d->handle, RequestTypeRead,
                                         ControlRequest::GetInputMask, 0, 0,
                                         &mask, sizeof(mask), TimeoutMilliseconds);
    if (result != sizeof(mask))
        throw DeviceException("Unable to read input mask.", result);

    return { (mask & (1<<0)) != 0, (mask & (1<<1)) != 0 };
}

// ---------------------------------------------------------------------------------------------- //

void Device::setupSignal(Input input, Waveform waveform, unsigned int frequency, double amplitude)
{
    assert(frequency >= MinimumFrequency && frequency <= MaximumFrequency);
    assert(amplitude >= MinimumAmplitude && amplitude <= MaximumAmplitude);

    struct
    {
        uint8_t input;

        uint32_t amplitude : 8;
        uint32_t           : 8;
        uint32_t frequency : 10;
        uint32_t waveform  : 3;
        uint32_t command   : 3;

    } __attribute__((__packed__)) parameters = {};

    static_assert(sizeof(parameters) == 5);

    parameters.input = indexOf(input);
    parameters.command = Command::SetupSignal;
    parameters.waveform = indexOf(waveform);
    parameters.frequency = frequency;
    parameters.amplitude = static_cast<uint8_t>(100.0 * amplitude);

    auto ptr = reinterpret_cast<uint8_t*>(&parameters);

    int result = libusb_control_transfer(d->handle, RequestTypeWrite,
                                         ControlRequest::SetInputCommand, 0, 0,
                                         ptr, sizeof(parameters), TimeoutMilliseconds);
    if (result != sizeof(parameters))
        throw DeviceException("Unable to setup output signal.", result);

    std::this_thread::sleep_for(10ms);
}

// ---------------------------------------------------------------------------------------------- //

void Device::setGain(Input input, Gain gain)
{
    struct
    {
        uint8_t input;

        uint32_t gain    :  2;
        uint32_t         : 27;
        uint32_t command :  3;

    } __attribute__((__packed__)) parameters = {};

    static_assert(sizeof(parameters) == 5);

    parameters.input = indexOf(input);
    parameters.command = Command::SetGain;
    parameters.gain = indexOf(gain);

    auto ptr = reinterpret_cast<uint8_t*>(&parameters);

    int result = libusb_control_transfer(d->handle, RequestTypeWrite,
                                         ControlRequest::SetInputCommand, 0, 0,
                                         ptr, sizeof(parameters), TimeoutMilliseconds);
    if (result != sizeof(parameters))
        throw DeviceException("Unable to set gain.", result);

    std::this_thread::sleep_for(10ms);
}

// ---------------------------------------------------------------------------------------------- //

void Device::startCapture()
{
    if (d->capturing)
        return;

    assert(!d->workerThread.joinable());

    d->capturing = true;
    d->workerThread = std::thread(&Device::work, this);
}

// ---------------------------------------------------------------------------------------------- //

void Device::stopCapture()
{
    if (!d->capturing)
        return;

    assert(d->workerThread.joinable());

    d->capturing = false;
    d->workerThread.join();
}

// ---------------------------------------------------------------------------------------------- //

auto Device::getPowerValues() const -> PowerValues
{
    std::array<uint16_t, 3> values = {};
    auto ptr = reinterpret_cast<uint8_t*>(values.data());

    const int size = values.size() * sizeof(uint16_t);

    int result = libusb_control_transfer(d->handle, RequestTypeRead,
                                         ControlRequest::GetPowerValues, 0, 0,
                                         ptr, size, TimeoutMilliseconds);
    if (result != size)
        throw DeviceException("Unable to read power values.", result);

    return { values.at(0) * 0.001, values.at(1) * 0.001, values.at(2) * 0.01 };
}

// ---------------------------------------------------------------------------------------------- //

void Device::work()
{
    static constexpr int16_t MinimumSampleValue = -32760;
    static constexpr int16_t MaximumSampleValue = +32760;

    static const auto copyData = [](const Private::TransferBuffer& transfer, Data& data)
    {
        size_t bufferOffset = 0;

        for (PerInputData& inputData : data)
        {
            for (PerChannelData& channelData : inputData)
            {
                for (double& sample : channelData)
                {
                    const auto value =
                            MinimumSampleValue + static_cast<int32_t>(transfer[bufferOffset++]);
                    sample = value * MaximumAmplitude / MaximumSampleValue;
                }
            }
        }
    };

    while (d->capturing)
    {
        const auto size = d->transferBuffer.size() * sizeof(Private::TransferBuffer::value_type);
        auto data = reinterpret_cast<unsigned char*>(d->transferBuffer.data());

        int actualLength = 0;

        int result = libusb_bulk_transfer(d->handle, BulkInEndpoint,
                                          data, size, &actualLength, TimeoutMilliseconds);

        if (result == LIBUSB_ERROR_OVERFLOW)
        {
            std::cerr << "Warning: USB overflow occurred, continuing anyway." << std::endl;
            result = 0;
        }

        if (result < 0)
        {
            for (auto listener : d->listeners)
                listener->onError("USB bulk transfer failed.");
        }
        else
        {
            copyData(d->transferBuffer, d->data);

            for (auto listener : d->listeners)
                listener->onDataAvailable(d->data);
        }
    }
}

// ---------------------------------------------------------------------------------------------- //

auto Device::toString(Waveform waveform) -> const char*
{
    static constexpr std::array<const char*, WaveformCount> strings = {
        "None", "Sine", "Square", "Triangle", "Sawtooth"
    };

    return strings.at(indexOf(waveform));
}

// ---------------------------------------------------------------------------------------------- //

auto Device::toString(Gain gain) -> const char*
{
    static constexpr std::array<const char*, GainCount> strings = {
        "100 Ω", "1 kΩ", "10 kΩ", "100 kΩ"
    };

    return strings.at(indexOf(gain));
}

// ---------------------------------------------------------------------------------------------- //

auto Device::toDouble(Gain gain) -> double
{
    static constexpr double OnResistance = 0.5;

    static constexpr std::array<double, GainCount> values = {
        1.0e2, 1.0e3, 1.0e4, 1.0e5
    };

    return values.at(indexOf(gain)) + OnResistance;
}

// ---------------------------------------------------------------------------------------------- //
