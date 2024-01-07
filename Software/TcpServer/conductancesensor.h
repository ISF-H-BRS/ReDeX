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

#include "conductancenode.h"
#include "macro.h"
#include "sensor.h"

#include <conductance/analyzer.h>

class ConductanceSensor : public Sensor
{
    Q_OBJECT
    REDEX_DELETE_COPY_MOVE(ConductanceSensor);

public:
    using Analyzer = isf::Conductance::Analyzer;
    using DataBuffer = isf::Conductance::DataBuffer;
    using Device = isf::Conductance::Device;

    struct Configuration
    {
        unsigned int frequency = 168;
        double amplitude = 1.0;
        double leadResistance = 0.0;
    };

public:
    ConductanceSensor(const QString& id, ConductanceNode* node, size_t input,
                      const Configuration& config);
    ~ConductanceSensor() override;

    void setConfiguration(const Configuration& config);

    auto node() const -> ConductanceNode* { return m_node; }

signals:
    void valuesAvailable(double voltage, double current, double admittance);

private:
    friend class ConductanceNode;
    void processData(const DataBuffer& buffer);

private:
    void updateGain(double current);

    static void checkConfiguration(const Configuration& config);

private:
    ConductanceNode* m_node;

    Analyzer m_analyzer;
    int m_analysisSkips = 0;

    Device::Gain m_gain = Device::Gain::_100;
    double m_leadResistance = 0.0;
};
