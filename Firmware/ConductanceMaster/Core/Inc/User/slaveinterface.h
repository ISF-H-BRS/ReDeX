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

#pragma once

#include "config.h"

#include <array>
#include <span>

class SlaveInterface
{
public:
    static constexpr unsigned int SampleRate = 10'000;

    static constexpr size_t InputCount = 2;

    static constexpr size_t DataQueueLength = 2;
    static constexpr size_t DataChannelCount = 2;
    static constexpr size_t DataSamplesCount = 500;

    static constexpr size_t DataBufferSize =
            InputCount * DataChannelCount * DataSamplesCount * sizeof(uint16_t);

    class Owner
    {
        friend class SlaveInterface;
        virtual void onSlaveBufferFull(std::span<const uint8_t> data) {}
    };

public:
    SlaveInterface(Owner* owner);
    ~SlaveInterface();

    auto getInputConnected(size_t input) const -> bool;
    auto getInputMask() const -> uint8_t;

    void startCapture();
    void stopCapture();

    void setCommand(size_t input, uint32_t command);

private:
    friend void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim);
    void onTimerPeriodElapsed();

private:
    Owner* m_owner;

    TIM_HandleTypeDef* m_timerHandle = Config::SampleTimerHandle;

    std::array<SPI_HandleTypeDef*, InputCount> m_spiHandles = {
        Config::Slave1SpiHandle, Config::Slave2SpiHandle
    };

    std::array<GPIO_TypeDef*, InputCount> m_csPorts = {
        SLAVE1_CS_GPIO_Port, SLAVE2_CS_GPIO_Port
    };

    std::array<uint16_t, InputCount> m_csPins = {
        SLAVE1_CS_Pin, SLAVE2_CS_Pin
    };

    std::array<GPIO_TypeDef*, InputCount> m_conPorts = {
        SLAVE1_CON_GPIO_Port, SLAVE2_CON_GPIO_Port
    };

    std::array<uint16_t, InputCount> m_conPins = {
        SLAVE1_CON_Pin, SLAVE2_CON_Pin
    };

    std::array<GPIO_TypeDef*, InputCount> m_drdyPorts = {
        SLAVE1_DRDY_GPIO_Port, SLAVE2_DRDY_GPIO_Port
    };

    std::array<uint16_t, InputCount> m_drdyPins = {
        SLAVE1_DRDY_Pin, SLAVE2_DRDY_Pin
    };

    using DataArray = std::array<uint16_t, InputCount * DataChannelCount * DataSamplesCount>;
    using DataQueue = std::array<DataArray, DataQueueLength>;
    DataQueue m_data;

    size_t m_queuePosition = 0;
    size_t m_samplePosition = 0;

    std::array<uint32_t, InputCount> m_nextCommands = { 0, 0 };

    static SlaveInterface* s_instance;
};
