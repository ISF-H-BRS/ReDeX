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

#include "exception.h"
#include "orpsensor.h"

// ---------------------------------------------------------------------------------------------- //

OrpSensor::OrpSensor(const QString& id, SensorsNode* node, size_t input)
    : SensorsSensor(id, node, input)
{
    const SensorsNode::SensorType type = node->getSensorType(input);

    if (type != SensorsNode::SensorType::pH_ORP)
    {
        const QString msg = "Sensor for ID %1 at node %2, input %3 is not an ORP sensor.";
        throw Exception(msg.arg(id, node->id()).arg(input));
    }
}

// ---------------------------------------------------------------------------------------------- //

void OrpSensor::processValue(double value)
{
    emit valueAvailable(value);
}

// ---------------------------------------------------------------------------------------------- //
