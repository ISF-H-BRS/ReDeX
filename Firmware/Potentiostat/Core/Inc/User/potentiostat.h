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

#include "blinker.h"
#include "gainmux.h"
#include "hostinterface.h"
#include "powermonitor.h"
#include "signalgenerator.h"
#include "signalreader.h"

class Potentiostat : public HostInterface::Owner,
                     public SignalGenerator::Owner,
                     public SignalReader::Owner
{
public:
    Potentiostat();
    ~Potentiostat();

    void exec();

private:
    void onHostDataReceived(const String& data) override;
    void onHostDataOverflow() override;

    void onSamplesAvailable(std::span<const Measurement::Sample> samples) override;
    void onSignalGenerationComplete() override;

    void protocolSetCalibration(const String& data, size_t tokenCount);

    void protocolStartMeasurement(const String& data, size_t tokenCount);
    void protocolStopMeasurement();

    void protocolSetGain(const String& data, size_t tokenCount);

    void protocolGetPowerValues();

    void checkTokenCount(size_t tokenCount, size_t expectedCount);

    void sendResponse(const String& tag, const String& data = {});
    void sendError(const String& error);

    void startMeasurement();
    void stopMeasurement();

    void setGain(Measurement::Gain gain);

    auto getSetupValid(const Measurement::Setup& setup) const -> bool;

private:
    HostInterface m_hostInterface;
    PowerMonitor m_powerMonitor;
    GainMux m_gainMux;
    SignalGenerator m_signalGenerator;
    SignalReader m_signalReader;
    Blinker m_blinker;

    Measurement::Gain m_currentGain = Measurement::DefaultGain;
    bool m_measurementRunning = false;

    std::array<String, Config::SamplesPerTransfer> m_sampleStrings = {};

    static Potentiostat* s_instance;
};
