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

#include "configuration.h"
#include "parsererror.h"

#include <QDir>
#include <QDomDocument>
#include <QFile>

// ---------------------------------------------------------------------------------------------- //

static
void readConfigFile(const QString& filename, QDomDocument* result)
{
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly))
        throw ParserError(QString("Unable to open file %1 for parsing.").arg(filename));

    QString msg;
    int line = -1;

    if (!result->setContent(&file, &msg, &line))
        throw ParserError(filename, line, msg);
}

// ---------------------------------------------------------------------------------------------- //

Configuration::Configuration(const QString& filename)
    : m_filename(filename)
{
    QDomDocument document;
    readConfigFile(m_filename, &document);

    const QDomElement root = document.documentElement();
    const QString tagName = root.tagName();

    if (tagName != "configuration")
        throw ParserError(m_filename, root.lineNumber(), "Unexpected tag \"" + tagName + "\".");

    for (QDomNode child = root.firstChild(); !child.isNull(); child = child.nextSibling())
    {
        if (!child.isElement())
            continue;

        const auto throwError = [&](const QString& msg) {
            throw ParserError(m_filename, child.lineNumber(), msg);
        };

        const QDomElement element = child.toElement();
        const QString tagName = element.tagName();

        if (tagName == "tcp_port")
        {
            const QString portStr = element.text();

            bool ok = false;
            const unsigned int port = portStr.toUInt(&ok);

            if (portStr.isEmpty() || !ok || port > std::numeric_limits<quint16>::max())
                throwError("Invalid TCP port specified.");

            m_tcpPort = static_cast<quint16>(port);
        }
        else if (tagName == "status_port")
        {
            const QString port = element.text();

            if (port.isEmpty() || !port.startsWith("tty"))
                throwError("Invalid status-board port specified.");

            m_statusPort = port;
        }
        else if (tagName == "nodes")
            parseNodes(element);
        else if (tagName == "sensors")
            parseSensors(element);
        else if (tagName == "testpoints")
            parseTestpoints(element);
        else
            throwError("Unexpected tag \"" + tagName + "\".");
    }
}

// ---------------------------------------------------------------------------------------------- //

auto Configuration::tcpPort() const -> quint16
{
    return m_tcpPort;
}

// ---------------------------------------------------------------------------------------------- //

auto Configuration::statusPort() const -> QString
{
    return m_statusPort;
}

// ---------------------------------------------------------------------------------------------- //

auto Configuration::nodes() const -> const NodeMap&
{
    return m_nodes;
}

// ---------------------------------------------------------------------------------------------- //

auto Configuration::sensors() const -> const SensorMap&
{
    return m_sensors;
}

// ---------------------------------------------------------------------------------------------- //

auto Configuration::testpoints() const -> const TestpointMap&
{
    return m_testpoints;
}

// ---------------------------------------------------------------------------------------------- //

auto Configuration::parseEntries(const QDomNode& parent,
                                 const StringList& validKeys) const -> StringMap
{
    const auto getKeyValid = [&](const QString& key)
    {
        return validKeys.empty() ||
               std::find(validKeys.begin(), validKeys.end(), key) != validKeys.end();
    };

    StringMap entries;

    for (QDomNode child = parent.firstChild(); !child.isNull(); child = child.nextSibling())
    {
        if (!child.isElement())
            continue;

        const auto throwError = [&](const QString& msg) {
            throw ParserError(m_filename, child.lineNumber(), msg);
        };

        const QDomElement element = child.toElement();
        const QString& key = element.tagName();

        if (!getKeyValid(key))
            throwError("Unsupported key \"" + key + "\".");

        entries[key] = element.text();
    }

    return entries;
}

// ---------------------------------------------------------------------------------------------- //

auto Configuration::parseSensor(const QDomElement& parent) const -> Sensor
{
    static const auto getInputValid = [](const QString& type, const QString& input)
    {
        if (type == "potentiostat")
            return input == "0";

        return input == "0" || input == "1";
    };

    static const auto getValidConfigKeys = [](const QString& type) -> StringList
    {
        if (type == "conductance")
            return { "frequency", "amplitude", "lead_resistance" };

        if (type == "potentiostat")
        {
            return {
                "current_range", "auto_range", "scan_rate",
                "vertex0", "vertex1", "vertex2", "cycle_count",
                "voltage_offset", "current_offset", "signal_offset"
            };
        }

        return {};
    };

    const auto throwError = [&](const QString& msg) {
        throw ParserError(m_filename, parent.lineNumber(), msg);
    };

    const QString type = parent.tagName();

    if (!getSensorTypeValid(type))
        throwError("Invalid sensor type.");

    const QString id = parent.attribute("id");

    if (id.isEmpty())
        throwError("Missing sensor ID.");

    if (m_sensors.contains(id))
        throwError("Duplicate sensor ID.");

    QString node;
    QString input;
    StringMap config;

    for (QDomNode child = parent.firstChild(); !child.isNull(); child = child.nextSibling())
    {
        if (!child.isElement())
            continue;

        const auto throwError = [&](const QString& msg) {
            throw ParserError(m_filename, child.lineNumber(), msg);
        };

        const QDomElement element = child.toElement();
        const QString tagName = element.tagName();

        if (tagName == "node")
        {
            node = element.text();

            if (!m_nodes.contains(node))
                throwError("Invalid node ID.");
        }
        else if (tagName == "input")
        {
            input = element.text();

            if (!getInputValid(type, input))
                throwError("Invalid input index.");
        }
        else if (tagName == "config")
            config = parseEntries(element, getValidConfigKeys(type));
        else
            throwError("Tag \"" + tagName + "\" not recognized.");
    }

    if (node.isEmpty())
        throwError("Missing node ID.");

    if (input.isEmpty())
        throwError("Missing input index.");

    return { type, id, node, input, config };
}

// ---------------------------------------------------------------------------------------------- //

auto Configuration::parseTestpoint(const QDomNode& parent) const -> StringMap
{
    const auto getIdValid = [this](const QString& id) {
        return m_sensors.contains(id);
    };

    StringMap sensors;

    for (QDomNode child = parent.firstChild(); !child.isNull(); child = child.nextSibling())
    {
        if (!child.isElement())
            continue;

        const auto throwError = [&](const QString& msg) {
            throw ParserError(m_filename, child.lineNumber(), msg);
        };

        const QDomElement element = child.toElement();
        const QString type = element.tagName();

        if (!getSensorTypeValid(type))
            throwError("Invalid sensor type.");

        if (sensors.contains(type))
            throwError("Duplicate sensor type.");

        const QString id = element.text();

        if (!getIdValid(id))
            throwError("Invalid sensor ID.");

        sensors[type] = id;
    }

    return sensors;
}

// ---------------------------------------------------------------------------------------------- //

void Configuration::parseNodes(const QDomNode& parent)
{
    static const auto isValidType = [](const QString& type) {
        return type == "conductance" || type == "potentiostat" || type == "sensors";
    };

    static const auto getValidKeys = [](const QString& type) -> StringList
    {
        if (type == "conductance")
            return { "serial" };

        return { "port" };
    };

    const auto isDuplicateId = [this](const QString& id) {
        return m_nodes.contains(id);
    };

    for (QDomNode child = parent.firstChild(); !child.isNull(); child = child.nextSibling())
    {
        if (!child.isElement())
            continue;

        const auto throwError = [&](const QString& msg) {
            throw ParserError(m_filename, child.lineNumber(), msg);
        };

        const QDomElement element = child.toElement();
        const QString type = element.tagName();

        if (!isValidType(type))
            throwError("Invalid node type.");

        const QString id = element.attribute("id");

        if (id.isEmpty())
            throwError("Missing node ID.");

        if (isDuplicateId(id))
            throwError("Duplicate node ID.");

        const StringMap entries = parseEntries(element, getValidKeys(type));

        if (type == "conductance" && !entries.contains("serial"))
            throwError("Missing serial number.");

        if (type != "conductance" && !entries.contains("port"))
            throwError("Missing serial port.");

        Node node = {};
        node.type = type;
        node.id = id;
        node.entries = std::move(entries);
        m_nodes[id] = std::move(node);
    }
}

// ---------------------------------------------------------------------------------------------- //

void Configuration::parseSensors(const QDomNode& parent)
{
    for (QDomNode child = parent.firstChild(); !child.isNull(); child = child.nextSibling())
    {
        if (!child.isElement())
            continue;

        const QDomElement element = child.toElement();

        Sensor sensor = parseSensor(element);
        m_sensors[sensor.id] = std::move(sensor);
    }
}

// ---------------------------------------------------------------------------------------------- //

void Configuration::parseTestpoints(const QDomNode& parent)
{
    const auto isDuplicateId = [this](const QString& id) {
        return m_testpoints.contains(id);
    };

    for (QDomNode child = parent.firstChild(); !child.isNull(); child = child.nextSibling())
    {
        if (!child.isElement())
            continue;

        const auto throwError = [&](const QString& msg) {
            throw ParserError(m_filename, child.lineNumber(), msg);
        };

        const QDomElement element = child.toElement();
        const QString tag = element.tagName();

        if (tag != "testpoint")
            throwError("Invalid tag.");

        const QString id = element.attribute("id");

        if (id.isEmpty())
            throwError("Missing testpoint ID.");

        if (isDuplicateId(id))
            throwError("Duplicate testpoint ID.");

        const StringMap sensors = parseTestpoint(element);

        const auto getSensorOrEmpty = [&sensors](const QString& type) -> QString {
            return sensors.contains(type) ? sensors.at(type) : "";
        };

        Testpoint testpoint = {};
        testpoint.id = id;
        testpoint.conductance = getSensorOrEmpty("conductance");
        testpoint.orp = getSensorOrEmpty("orp");
        testpoint.ph = getSensorOrEmpty("ph");
        testpoint.potentiostat = getSensorOrEmpty("potentiostat");
        testpoint.temperature = getSensorOrEmpty("temperature");
        m_testpoints[id] = std::move(testpoint);
    }
}

// ---------------------------------------------------------------------------------------------- //

auto Configuration::getSensorTypeValid(const QString& type) -> bool
{
    return type == "conductance"  ||
           type == "orp"          ||
           type == "ph"           ||
           type == "potentiostat" ||
           type == "temperature";
}

// ---------------------------------------------------------------------------------------------- //
