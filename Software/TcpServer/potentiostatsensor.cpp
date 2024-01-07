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

#include "assertions.h"
#include "exception.h"
#include "potentiostatsensor.h"

// ---------------------------------------------------------------------------------------------- //

PotentiostatSensor::PotentiostatSensor(const QString& id, PotentiostatNode* node, size_t input,
                                       const Configuration& config)
    : Sensor(id, node, input),
      m_node(node)
{
    ASSERT(input < PotentiostatNode::InputCount);
    m_node->registerSensor(this, input);

    setConfiguration(config);
}

// ---------------------------------------------------------------------------------------------- //

PotentiostatSensor::~PotentiostatSensor()
{
    m_node->unregisterSensor(this);
}

// ---------------------------------------------------------------------------------------------- //

void PotentiostatSensor::setConfiguration(const Configuration& config)
{
    checkConfiguration(config);

    Device::Setup setup = {};
    setup.measurementType = Device::MeasurementType::CyclicVoltammetry;
    setup.currentRange = config.currentRange;
    setup.autoRange = config.autoRange;
    setup.scanRate = config.scanRate;
    setup.vertex0 = config.vertex0;
    setup.vertex1 = config.vertex1;
    setup.vertex2 = config.vertex2;
    setup.cycleCount = config.cycleCount;

    Device::Calibration calibration = {};
    calibration.voltageOffset = config.voltageOffset;
    calibration.currentOffset = config.currentOffset;
    calibration.signalOffset = config.signalOffset;

    m_node->setup(setup, calibration);
}

// ---------------------------------------------------------------------------------------------- //

void PotentiostatSensor::processData(const Voltammogram& data)
{
    emit dataAvailable(data);
}

// ---------------------------------------------------------------------------------------------- //

void PotentiostatSensor::checkConfiguration(const Configuration& config)
{
    if (outOfRange(config.scanRate, Device::MinimumScanRate, Device::MaximumScanRate))
        throw Exception(QString::number(config.scanRate) + " is not a valid scan rate.");

    for (auto vertex : { config.vertex0, config.vertex1, config.vertex2 })
    {
        if (outOfRange(vertex, Device::MinimumPotential, Device::MaximumPotential))
            throw Exception(QString::number(vertex) + " is not a valid vertex potential.");
    }

    if (outOfRange(config.cycleCount, Device::MinimumCycleCount, Device::MaximumCycleCount))
        throw Exception(QString::number(config.cycleCount) + " is not a valid cycle count.");
}

// ---------------------------------------------------------------------------------------------- //
