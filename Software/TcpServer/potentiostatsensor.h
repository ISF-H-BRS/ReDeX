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

#include "macro.h"
#include "potentiostatnode.h"
#include "sensor.h"

class PotentiostatSensor : public Sensor
{
    Q_OBJECT
    REDEX_DELETE_COPY_MOVE(PotentiostatSensor);

public:
    using Device = isf::Potentiostat::Device;

    struct Configuration
    {
        Device::CurrentRange currentRange = Device::CurrentRange::_200nA;
        bool autoRange = true;
        int scanRate = Device::MaximumScanRate;
        int vertex0 = 0;
        int vertex1 = -500;
        int vertex2 = +500;
        int cycleCount = Device::MinimumCycleCount;
        int voltageOffset = 0;
        int currentOffset = 0;
        int signalOffset = 0;
    };

public:
    explicit PotentiostatSensor(const QString& id, PotentiostatNode* node, size_t input,
                                const Configuration& config);
    ~PotentiostatSensor() override;

    void setConfiguration(const Configuration& config);

    auto node() const -> PotentiostatNode* { return m_node; }

signals:
    void dataAvailable(const Voltammogram& data);

private:
    friend class PotentiostatNode;
    void processData(const Voltammogram& data);

    static void checkConfiguration(const Configuration& config);

private:
    PotentiostatNode* m_node;
};
