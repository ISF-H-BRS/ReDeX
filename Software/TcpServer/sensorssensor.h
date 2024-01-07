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
#include "sensor.h"
#include "sensorsnode.h"

class SensorsSensor : public Sensor
{
    Q_OBJECT
    REDEX_DELETE_COPY_MOVE(SensorsSensor);

public:
    SensorsSensor(const QString& id, SensorsNode* node, size_t input);
    ~SensorsSensor() override;

    auto node() const -> SensorsNode*;

signals:
    void valueAvailable(double value);

protected:
    friend class SensorsNode;
    virtual void processValue(double value) = 0;

private:
    SensorsNode* m_node;
};
