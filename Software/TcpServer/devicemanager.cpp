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
#include "devicemanager.h"
#include "exception.h"

// ---------------------------------------------------------------------------------------------- //

DeviceManager::DeviceManager(const Configuration& config)
{
    const Configuration::NodeMap& nodes = config.nodes();

    for (const auto& pair : nodes)
    {
        m_nodeMap[pair.first] = makeNode(pair.second);
        m_nodes.push_back(m_nodeMap.at(pair.first).get());
    }

    const Configuration::SensorMap& sensors = config.sensors();

    for (const auto& pair : sensors)
    {
        m_sensorMap[pair.first] = makeSensor(pair.second);
        m_sensors.push_back(m_sensorMap.at(pair.first).get());
    }

    const Configuration::TestpointMap& testpoints = config.testpoints();

    for (const auto& pair : testpoints)
    {
        m_testpointMap[pair.first] = makeTestpoint(pair.second);
        m_testpoints.push_back(m_testpointMap.at(pair.first).get());
    }
}

// ---------------------------------------------------------------------------------------------- //

DeviceManager::~DeviceManager() = default;

// ---------------------------------------------------------------------------------------------- //

auto DeviceManager::nodes() const -> const std::vector<Node*>&
{
    return m_nodes;
}

// ---------------------------------------------------------------------------------------------- //

auto DeviceManager::sensors() const -> const std::vector<Sensor*>&
{
    return m_sensors;
}

// ---------------------------------------------------------------------------------------------- //

auto DeviceManager::testpoints() const -> const std::vector<Testpoint*>&
{
    return m_testpoints;
}

// ---------------------------------------------------------------------------------------------- //

auto DeviceManager::findNode(const QString& id) const -> Node*
{
    auto it = m_nodeMap.find(id);

    if (it == m_nodeMap.end())
        return nullptr;

    return it->second.get();
}

// ---------------------------------------------------------------------------------------------- //

auto DeviceManager::findSensor(const QString& id) const -> Sensor*
{
    auto it = m_sensorMap.find(id);

    if (it == m_sensorMap.end())
        return nullptr;

    return it->second.get();
}

// ---------------------------------------------------------------------------------------------- //

auto DeviceManager::findTestpoint(const QString& id) const -> Testpoint*
{
    auto it = m_testpointMap.find(id);

    if (it == m_testpointMap.end())
        return nullptr;

    return it->second.get();
}

// ---------------------------------------------------------------------------------------------- //

template <typename T>
auto DeviceManager::getNode(const QString& id) const -> T*
{
    ASSERT(m_nodeMap.contains(id));
    Node* node = m_nodeMap.at(id).get();

    auto retval = dynamic_cast<T*>(node);
    ASSERT_NOT_NULL(retval);

    return retval;
}

// ---------------------------------------------------------------------------------------------- //

template <typename T>
auto DeviceManager::getSensor(const QString& id) const -> T*
{
    ASSERT(m_sensorMap.contains(id));
    Sensor* sensor = m_sensorMap.at(id).get();

    auto retval = dynamic_cast<T*>(sensor);
    ASSERT_NOT_NULL(retval);

    return retval;
}

// ---------------------------------------------------------------------------------------------- //

auto DeviceManager::makeConductanceSensor(const Configuration::Sensor& sensor) const -> SensorPtr
{
    ASSERT(sensor.type == "conductance");

    const QString id = getId(sensor.id);
    const auto node = getNode<ConductanceNode>(sensor.node);
    const size_t input = getUInt(sensor.input);

    ConductanceSensor::Configuration config = {};

    for (const auto& pair : sensor.config)
    {
        if (pair.first == "frequency")
            config.frequency = getUInt(pair.second);
        else if (pair.first == "amplitude")
            config.amplitude = getDouble(pair.second);
        else if (pair.first == "lead_resistance")
            config.leadResistance = getDouble(pair.second);
        else
            FAIL();
    }

    return std::make_unique<ConductanceSensor>(id, node, input, config);
}

// ---------------------------------------------------------------------------------------------- //

auto DeviceManager::makePotentiostatSensor(const Configuration::Sensor& sensor) const -> SensorPtr
{
    ASSERT(sensor.type == "potentiostat");

    static const auto getCurrentRange = [](const QString& value)
    {
        const auto index = getUInt(value);

        if (index >= PotentiostatSensor::Device::CurrentRangeCount)
            throw Exception("Value " + value +  " is not a valid current-range index.");

        return PotentiostatSensor::Device::toCurrentRange(index);
    };

    const QString id = getId(sensor.id);
    const auto node = getNode<PotentiostatNode>(sensor.node);
    const size_t input = getUInt(sensor.input);

    PotentiostatSensor::Configuration config = {};

    for (const auto& pair : sensor.config)
    {
        if (pair.first == "current_range")
            config.currentRange = getCurrentRange(pair.second);
        else if (pair.first == "auto_range")
            config.autoRange = getBool(pair.second);
        else if (pair.first == "scan_rate")
            config.scanRate = getInt(pair.second);
        else if (pair.first == "vertex0")
            config.vertex0 = getInt(pair.second);
        else if (pair.first == "vertex1")
            config.vertex1 = getInt(pair.second);
        else if (pair.first == "vertex2")
            config.vertex2 = getInt(pair.second);
        else if (pair.first == "cycle_count")
            config.cycleCount = getInt(pair.second);
        else if (pair.first == "voltage_offset")
            config.voltageOffset = getInt(pair.second);
        else if (pair.first == "current_offset")
            config.currentOffset = getInt(pair.second);
        else if (pair.first == "signal_offset")
            config.signalOffset = getInt(pair.second);
        else
            FAIL();
    }

    return std::make_unique<PotentiostatSensor>(id, node, input, config);
}

// ---------------------------------------------------------------------------------------------- //

auto DeviceManager::makeSensorsSensor(const Configuration::Sensor& sensor) const -> SensorPtr
{
    const QString id = getId(sensor.id);
    const auto node = getNode<SensorsNode>(sensor.node);
    const size_t input = getUInt(sensor.input);

    if (sensor.type == "orp")
        return std::make_unique<OrpSensor>(id, node, input);

    if (sensor.type == "ph")
        return std::make_unique<PhSensor>(id, node, input);

    ASSERT(sensor.type == "temperature");
    return std::make_unique<TemperatureSensor>(id, node, input);
}

// ---------------------------------------------------------------------------------------------- //

auto DeviceManager::makeNode(const Configuration::Node& node) const -> NodePtr
{
    const QString& id = getId(node.id);

    if (node.type == "conductance")
    {
        ASSERT(node.entries.contains("serial"));
        return std::make_unique<ConductanceNode>(id, node.entries.at("serial"));
    }

    ASSERT(node.entries.contains("port"));
    const QString& serialPort = node.entries.at("port");

    if (node.type == "potentiostat")
        return std::make_unique<PotentiostatNode>(id, serialPort);

    ASSERT(node.type == "sensors");
    return std::make_unique<SensorsNode>(id, serialPort);
}

// ---------------------------------------------------------------------------------------------- //

auto DeviceManager::makeSensor(const Configuration::Sensor& sensor) const -> SensorPtr
{
    if (sensor.type == "conductance")
        return makeConductanceSensor(sensor);

    if (sensor.type == "potentiostat")
        return makePotentiostatSensor(sensor);

    ASSERT_IN_LIST(sensor.type, "orp", "ph", "temperature");
    return makeSensorsSensor(sensor);
}

// ---------------------------------------------------------------------------------------------- //

auto DeviceManager::makeTestpoint(const Configuration::Testpoint& testpoint) const -> TestpointPtr
{
    Testpoint::Sensors sensors = {};

    if (!testpoint.conductance.isEmpty())
        sensors.conductance = getSensor<ConductanceSensor>(testpoint.conductance);

    if (!testpoint.orp.isEmpty())
        sensors.orp = getSensor<OrpSensor>(testpoint.orp);

    if (!testpoint.ph.isEmpty())
        sensors.ph = getSensor<PhSensor>(testpoint.ph);

    if (!testpoint.potentiostat.isEmpty())
        sensors.potentiostat = getSensor<PotentiostatSensor>(testpoint.potentiostat);

    if (!testpoint.temperature.isEmpty())
        sensors.temperature = getSensor<TemperatureSensor>(testpoint.temperature);

    return std::make_unique<Testpoint>(testpoint.id, sensors);
}

// ---------------------------------------------------------------------------------------------- //

auto DeviceManager::getId(const QString& id) -> QString
{
    ASSERT(!id.isEmpty());
    return id;
}

// ---------------------------------------------------------------------------------------------- //

auto DeviceManager::getInt(const QString& value) -> int
{
    bool ok = false;
    const int result = value.toInt(&ok);

    if (!ok)
        throw Exception("Value " + value +  " is not a valid signed integer.");

    return result;
}

// ---------------------------------------------------------------------------------------------- //

auto DeviceManager::getUInt(const QString& value) -> unsigned int
{
    bool ok = false;
    const unsigned int result = value.toUInt(&ok);

    if (!ok)
        throw Exception("Value " + value +  " is not a valid unsigned integer.");

    return result;
}

// ---------------------------------------------------------------------------------------------- //

auto DeviceManager::getDouble(const QString& value) -> double
{
    bool ok = false;
    const double result = value.toDouble(&ok);

    if (!ok)
        throw Exception("Value " + value +  " is not a valid floating-point number.");

    return result;
}

// ---------------------------------------------------------------------------------------------- //

auto DeviceManager::getBool(const QString& value) -> bool
{
    if (value == "true" || value == "1")
        return true;

    if (value == "false" || value == "0")
        return false;

    throw Exception("Value " + value +  " is not a valid boolean.");
}

// ---------------------------------------------------------------------------------------------- //
