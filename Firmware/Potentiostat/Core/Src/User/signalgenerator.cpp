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

#include "assert.h"
#include "config.h"
#include "signalgenerator.h"

// ---------------------------------------------------------------------------------------------- //

void SignalPeriodElapsedCallback()
{
    ASSERT(SignalGenerator::s_instance != nullptr);
    SignalGenerator::s_instance->isr();
}

// ---------------------------------------------------------------------------------------------- //

SignalGenerator* SignalGenerator::s_instance = nullptr;

// ---------------------------------------------------------------------------------------------- //

SignalGenerator::SignalGenerator(Owner* owner)
    : m_owner(owner),
      m_dac(&m_dacInterface, Config::DacAddress)
{
    ASSERT(owner != nullptr);

    ASSERT(s_instance == nullptr);
    s_instance = this;

    updateSignal();
}

// ---------------------------------------------------------------------------------------------- //

SignalGenerator::~SignalGenerator()
{
    ASSERT(s_instance == this);

    HAL_TIM_Base_Stop_IT(Config::SignalTimerHandle);
    s_instance = nullptr;
}

// ---------------------------------------------------------------------------------------------- //

void SignalGenerator::setCalibrationOffset(int offset)
{
    ASSERT(offset >= MinimumCalibrationOffset && offset <= MaximumCalibrationOffset);
    m_calibrationOffset = offset;
}

// ---------------------------------------------------------------------------------------------- //

void SignalGenerator::prepare(const Measurement::Setup& setup)
{
    static constexpr int PotentialRatio = 2; // Two LSB per mV

    const auto toValue = [this](int millivolts)
    {
        const int value = MidRangeValue - (PotentialRatio*millivolts + m_calibrationOffset);
        return static_cast<uint16_t>(value); // Value negated
    };

    m_measurementType = setup.type;

    const uint32_t div = isScanningMode() ? (PotentialRatio * setup.scanRate) : 1;
    m_timerPeriod = TimerFrequency / div;

    m_vertex0Value = toValue(setup.vertex0);
    m_vertex1Value = toValue(setup.vertex1);
    m_vertex2Value = toValue(setup.vertex2);

    m_currentValue = m_vertex0Value;

    m_state = isScanningMode() ? State::Vertex1 : State::Vertex0;
    m_currentTargetValue = isScanningMode() ? m_vertex1Value : m_vertex0Value;

    m_secondsRemaining = setup.duration.count();
    m_cyclesRemaining = setup.cycleCount;

    updateSignal();
}

// ---------------------------------------------------------------------------------------------- //

void SignalGenerator::start()
{
    __HAL_TIM_SET_COUNTER(Config::SignalTimerHandle, 0);
    __HAL_TIM_SET_AUTORELOAD(Config::SignalTimerHandle, m_timerPeriod - 1);

    updateSignal();

    HAL_TIM_Base_Start_IT(Config::SignalTimerHandle);
}

// ---------------------------------------------------------------------------------------------- //

void SignalGenerator::stop()
{
    HAL_TIM_Base_Stop_IT(Config::SignalTimerHandle);
    m_state = State::Idle;
}

// ---------------------------------------------------------------------------------------------- //

void SignalGenerator::pause()
{
    __HAL_TIM_DISABLE(Config::SignalTimerHandle);
}

// ---------------------------------------------------------------------------------------------- //

void SignalGenerator::unpause()
{
    __HAL_TIM_ENABLE(Config::SignalTimerHandle);
}

// ---------------------------------------------------------------------------------------------- //

void SignalGenerator::update()
{
    if (m_periodElapsed)
    {
        m_periodElapsed = false;
        process();
    }
}

// ---------------------------------------------------------------------------------------------- //

inline
void SignalGenerator::isr()
{
    m_periodElapsed = true;
}

// ---------------------------------------------------------------------------------------------- //

inline
void SignalGenerator::updateSignal()
{
    m_dac.setValue(m_currentValue);
}

// ---------------------------------------------------------------------------------------------- //

void SignalGenerator::updateState()
{
    ASSERT(isScanningMode() && m_currentValue == m_currentTargetValue);

    if (m_state == State::Vertex1)
    {
        if (m_measurementType == Measurement::Type::CyclicVoltammetry)
        {
            m_state = State::Vertex2;
            m_currentTargetValue = m_vertex2Value;
        }
        else
            completeMeasurement();
    }
    else if (m_state == State::Vertex2)
    {
        ASSERT(m_measurementType == Measurement::Type::CyclicVoltammetry);

        m_state = State::Vertex0;
        m_currentTargetValue = m_vertex0Value;
    }
    else if (m_state == State::Vertex0)
    {
        ASSERT(m_measurementType == Measurement::Type::CyclicVoltammetry);

        if (--m_cyclesRemaining <= 0)
            completeMeasurement();
        else
        {
            m_state = State::Vertex1;
            m_currentTargetValue = m_vertex1Value;
        }
    }
}

// ---------------------------------------------------------------------------------------------- //

void SignalGenerator::process()
{
    if (isScanningMode())
    {
        if (m_currentValue < m_currentTargetValue)
            ++m_currentValue;
        else if (m_currentValue > m_currentTargetValue)
            --m_currentValue;
        else
            updateState();

        updateSignal();
    }
    else if (--m_secondsRemaining <= 0)
        completeMeasurement();
}

// ---------------------------------------------------------------------------------------------- //

inline
void SignalGenerator::completeMeasurement()
{
    stop();
    m_owner->onSignalGenerationComplete();
}

// ---------------------------------------------------------------------------------------------- //

inline
auto SignalGenerator::isScanningMode() const -> bool
{
    return m_measurementType == Measurement::Type::LinearSweep ||
           m_measurementType == Measurement::Type::CyclicVoltammetry;
}

// ---------------------------------------------------------------------------------------------- //

void SignalGenerator::DacInterface::write(uint8_t address, std::span<const uint8_t> data)
{
    auto i2c = Config::DacI2cHandle;
    auto ptr = const_cast<uint8_t*>(data.data());

    HAL_I2C_Master_Transmit(i2c, address, ptr, data.size(), 100);
}

// ---------------------------------------------------------------------------------------------- //
