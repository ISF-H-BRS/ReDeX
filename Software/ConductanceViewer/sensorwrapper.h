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

#include "device.h"

#include <QObject>
#include <QTimer>

class SensorWrapper : public QObject
{
    Q_OBJECT

public:
    SensorWrapper(Device* device, Device::Input input, DataBuffer* buffer);

    void setupSignal(Device::Waveform waveform, unsigned int frequency, double amplitude);
    void setGain(Device::Gain gain);

    void setLeadResistance(unsigned int milliohm) noexcept;

    void update(const Device::Data& data);

signals:
    void analysisComplete(Device::Input input,
                          double voltage, double current, double admittance);

private slots:
    void analyzeData();

private:
    void restartAnalysisTimer(std::chrono::milliseconds ms);

private:
    Device* m_device;
    Device::Input m_input;
    DataBuffer* m_dataBuffer;

    Analyzer m_analyzer;

    Device::Gain m_gain = Device::Gain::_100;
    double m_leadResistance = 0.0;

    QTimer m_analysisTimer;
};
