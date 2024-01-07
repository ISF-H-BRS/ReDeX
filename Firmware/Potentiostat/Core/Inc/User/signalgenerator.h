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

#include "main.h"
#include "max521x.h"
#include "measurement.h"

class SignalGenerator
{
public:
    class Owner
    {
    public:
        virtual void onSignalGenerationComplete() = 0;
    };

    static constexpr int MinimumCalibrationOffset = -100;
    static constexpr int MaximumCalibrationOffset = +100;

public:
    SignalGenerator(Owner* owner);
    ~SignalGenerator();

    void setCalibrationOffset(int offset);

    void prepare(const Measurement::Setup& setup);
    void start();
    void stop();
    void pause();
    void unpause();
    void update();

private:
    friend void SignalPeriodElapsedCallback();
    void isr();

    void updateSignal();
    void updateState();

    void process();
    void completeMeasurement();

    auto isScanningMode() const -> bool;

private:
    static constexpr uint32_t TimerFrequency = 1'000'000;
    static constexpr int MidRangeValue = (1<<14) / 2;

    Owner* m_owner;

    enum class State
    {
        Idle,
        Vertex1,
        Vertex2,
        Vertex0
    } m_state = State::Idle;

    Measurement::Type m_measurementType = Measurement::Type::OpenCircuit;

    uint32_t m_timerPeriod = TimerFrequency;

    uint16_t m_vertex0Value = MidRangeValue;
    uint16_t m_vertex1Value = MidRangeValue;
    uint16_t m_vertex2Value = MidRangeValue;

    uint16_t m_currentValue = MidRangeValue;
    uint16_t m_currentTargetValue = MidRangeValue;

    int m_secondsRemaining = 0;
    int m_cyclesRemaining = 0;

    int m_calibrationOffset = 0;

    volatile bool m_periodElapsed = false;

    class DacInterface : public Max5215::Interface
    {
    private:
        void write(uint8_t address, std::span<const uint8_t> data) override;
    };

    DacInterface m_dacInterface;
    Max5215 m_dac;

    static SignalGenerator* s_instance;
};
