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

#include "configuration.h"
#include "macro.h"
#include "testpoint.h"

#include <map>
#include <memory>

class DeviceManager
{
    REDEX_DELETE_COPY_MOVE(DeviceManager);

public:
    explicit DeviceManager(const Configuration& config);
    ~DeviceManager();

    auto nodes() const -> const std::vector<Node*>&;
    auto sensors() const -> const std::vector<Sensor*>&;
    auto testpoints() const -> const std::vector<Testpoint*>&;

    auto findNode(const QString& id) const -> Node*;
    auto findSensor(const QString& id) const -> Sensor*;
    auto findTestpoint(const QString& id) const -> Testpoint*;

private:
    template <typename T>
    auto getNode(const QString& id) const -> T*;

    template <typename T>
    auto getSensor(const QString& id) const -> T*;

    auto makeConductanceSensor(const Configuration::Sensor& sensor) const -> SensorPtr;
    auto makePotentiostatSensor(const Configuration::Sensor& sensor) const -> SensorPtr;
    auto makeSensorsSensor(const Configuration::Sensor& sensor) const -> SensorPtr;

    auto makeNode(const Configuration::Node& node) const -> NodePtr;
    auto makeSensor(const Configuration::Sensor& sensor) const -> SensorPtr;
    auto makeTestpoint(const Configuration::Testpoint& testpoint) const -> TestpointPtr;

    static auto getId(const QString& id) -> QString;
    static auto getInt(const QString& value) -> int;
    static auto getUInt(const QString& value) -> unsigned int;
    static auto getDouble(const QString& value) -> double;
    static auto getBool(const QString& value) -> bool;

private:
    std::map<QString,NodePtr> m_nodeMap;
    std::map<QString,SensorPtr> m_sensorMap;
    std::map<QString,TestpointPtr> m_testpointMap;

    std::vector<Node*> m_nodes;
    std::vector<Sensor*> m_sensors;
    std::vector<Testpoint*> m_testpoints;
};
