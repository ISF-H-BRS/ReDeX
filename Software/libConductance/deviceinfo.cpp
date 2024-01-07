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
#include <conductance/deviceinfo.h>

#include <libusb-1.0/libusb.h>

#include <array>
#include <span>

// ---------------------------------------------------------------------------------------------- //

using namespace isf::Conductance;

// ---------------------------------------------------------------------------------------------- //

static
auto getProductNameAndSerialNumber(const libusb_device_descriptor& descriptor,
                                   libusb_device* device) -> std::pair<std::string,std::string>
{
    libusb_device_handle* handle = nullptr;
    int result = libusb_open(device, &handle);

    if (result < 0)
        return {};

    std::array<unsigned char, 255> product = {};
    std::array<unsigned char, 255> serial = {};

    result = libusb_get_string_descriptor_ascii(handle, descriptor.iProduct,
                                                product.data(), product.size());
    if (result >= 0)
    {
        result = libusb_get_string_descriptor_ascii(handle, descriptor.iSerialNumber,
                                                    serial.data(), serial.size());
    }

    libusb_close(handle);

    if (result < 0)
        return {};

    return {
        reinterpret_cast<char*>(product.data()), reinterpret_cast<char*>(serial.data())
    };
}

// ---------------------------------------------------------------------------------------------- //

DeviceInfo::DeviceInfo(const std::string& product, const std::string& serial,
                       uint8_t bus, uint8_t port)
    : m_productName(product), m_serialNumber(serial), m_busNumber(bus), m_portNumber(port) {}

// ---------------------------------------------------------------------------------------------- //

auto DeviceInfo::productName() const -> const std::string&
{
    return m_productName;
}

// ---------------------------------------------------------------------------------------------- //

auto DeviceInfo::serialNumber() const -> const std::string&
{
    return m_serialNumber;
}

// ---------------------------------------------------------------------------------------------- //

auto DeviceInfo::busNumber() const -> uint8_t
{
    return m_busNumber;
}

// ---------------------------------------------------------------------------------------------- //

auto DeviceInfo::portNumber() const -> uint8_t
{
    return m_portNumber;
}

// ---------------------------------------------------------------------------------------------- //

auto DeviceInfo::getAvailableDevices() -> std::vector<DeviceInfo>
{
    libusb_context* context = nullptr;

    if (libusb_init(&context) < 0)
        return {};

    libusb_device** list = nullptr;
    ssize_t count = libusb_get_device_list(nullptr, &list);

    std::span<libusb_device*> devices(list, count);

    std::vector<DeviceInfo> result;

    for (auto device : devices)
    {
        libusb_device_descriptor descriptor = {};

        if (libusb_get_device_descriptor(device, &descriptor) < 0)
            continue;

        if (descriptor.idVendor == Device::VendorId && descriptor.idProduct == Device::ProductId)
        {
            const auto [product,serial] = getProductNameAndSerialNumber(descriptor, device);

            if (product == Device::ProductName)
            {
                const uint8_t bus = libusb_get_bus_number(device);
                const uint8_t port = libusb_get_port_number(device);

                DeviceInfo info(product, serial, bus, port);
                result.push_back(std::move(info));
            }
        }
    }

    if (count >= 0)
        libusb_free_device_list(list, 1);

    libusb_exit(context);

    return result;
}

// ---------------------------------------------------------------------------------------------- //

auto DeviceInfo::getFirstDevice() -> DeviceInfo
{
    const std::vector<DeviceInfo> devices = getAvailableDevices();

    if (devices.empty())
        throw NoDeviceError("No devices found.");

    return devices.at(0);
}

// ---------------------------------------------------------------------------------------------- //
