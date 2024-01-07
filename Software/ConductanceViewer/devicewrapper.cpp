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

#include "devicewrapper.h"

// ---------------------------------------------------------------------------------------------- //

namespace {
    constexpr std::chrono::milliseconds DeferredTimeout =    0ms;
    constexpr std::chrono::milliseconds DefaultTimeout  =  500ms;
}

// ---------------------------------------------------------------------------------------------- //

class DeviceWrapper::Listener : public Device::Listener
{
public:
    Listener(DeviceWrapper* owner);

private:
    void onDataAvailable(const Device::Data& data) override;
    void onError(const std::string& msg) override;

private:
    DeviceWrapper* m_owner;
};

// ---------------------------------------------------------------------------------------------- //

DeviceWrapper::DeviceWrapper(const DeviceInfo& info,
                             const std::array<DataBuffer*, Device::InputCount>& data)
    : m_listener(std::make_unique<Listener>(this)),
      m_device(info),
      m_powerTimer(this)
{
    m_device.addListener(m_listener.get());

    const std::array<bool, Device::InputCount> inputsConnected = m_device.getInputsConnected();

    for (size_t i = 0; i < Device::InputCount; ++i)
    {
        if (inputsConnected[i])
        {
            m_sensors[i] = std::make_unique<SensorWrapper>(&m_device, Device::toInput(i), data[i]);

            auto ptr = m_sensors[i].get();
            connect(ptr,  SIGNAL(analysisComplete(Device::Input,double,double,double)),
                    this, SIGNAL(analysisComplete(Device::Input,double,double,double)));
        }
    }

    m_powerTimer.setSingleShot(true);
    connect(&m_powerTimer, SIGNAL(timeout()), this, SLOT(updatePowerValues()));

    restartPowerTimer(DeferredTimeout);
}

// ---------------------------------------------------------------------------------------------- //

DeviceWrapper::~DeviceWrapper() = default;

// ---------------------------------------------------------------------------------------------- //

void DeviceWrapper::startCapture()
{
    m_device.startCapture();
}

// ---------------------------------------------------------------------------------------------- //

void DeviceWrapper::stopCapture()
{
    m_device.stopCapture();
}

// ---------------------------------------------------------------------------------------------- //

auto DeviceWrapper::inputConnected(Device::Input input) const -> bool
{
    return m_sensors[Device::indexOf(input)] != nullptr;
}

// ---------------------------------------------------------------------------------------------- //

void DeviceWrapper::setGain(Device::Input input, Device::Gain gain)
{
    auto& sensor = m_sensors[Device::indexOf(input)];
    Q_ASSERT(sensor != nullptr);

    try {
        sensor->setGain(gain);
    }
    catch (const std::exception& e) {
        emit error(e.what());
    }
}

// ---------------------------------------------------------------------------------------------- //

void DeviceWrapper::setupSignal(Device::Input input,
                                Device::Waveform waveform, unsigned int frequency, double amplitude)
{
    auto& sensor = m_sensors[Device::indexOf(input)];
    Q_ASSERT(sensor != nullptr);

    try {
        sensor->setupSignal(waveform, frequency, amplitude);
    }
    catch (const std::exception& e) {
        emit error(e.what());
    }
}

// ---------------------------------------------------------------------------------------------- //

void DeviceWrapper::setLeadResistance(Device::Input input, unsigned int milliohm)
{
    auto& sensor = m_sensors[Device::indexOf(input)];
    Q_ASSERT(sensor != nullptr);

    sensor->setLeadResistance(milliohm);
}

// ---------------------------------------------------------------------------------------------- //

void DeviceWrapper::handleDataAvailable(const Device::Data& data)
{
    for (size_t i = 0; i < Device::InputCount; ++i)
    {
        auto& sensor = m_sensors[i];

        if (sensor)
            sensor->update(data);
    }

    emit dataUpdated();
}

// ---------------------------------------------------------------------------------------------- //

void DeviceWrapper::updatePowerValues()
{
    restartPowerTimer(DefaultTimeout);

    try {
        const Device::PowerValues values = m_device.getPowerValues();
        emit powerUpdated(values);
    }
    catch (const std::exception& e) {
        emit error(e.what());
    }
}

// ---------------------------------------------------------------------------------------------- //

void DeviceWrapper::restartPowerTimer(std::chrono::milliseconds ms)
{
    m_powerTimer.stop();
    m_powerTimer.start(ms);
}

// ---------------------------------------------------------------------------------------------- //

DeviceWrapper::Listener::Listener(DeviceWrapper* owner)
    : m_owner(owner) {}

// ---------------------------------------------------------------------------------------------- //

void DeviceWrapper::Listener::onDataAvailable(const Device::Data& data)
{
    QMetaObject::invokeMethod(m_owner, "handleDataAvailable", Qt::QueuedConnection,
                              Q_ARG(Device::Data, data));
}

// ---------------------------------------------------------------------------------------------- //

void DeviceWrapper::Listener::onError(const std::string& msg)
{
    QMetaObject::invokeMethod(m_owner, "error", Qt::QueuedConnection,
                              Q_ARG(QString, msg.c_str()));
}

// ---------------------------------------------------------------------------------------------- //
