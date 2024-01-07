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

#include <QDomNode>
#include <QString>

#include <map>
#include <vector>

class Configuration
{
public:
    using StringList = std::vector<QString>;
    using StringMap = std::map<QString,QString>;

    struct Node
    {
        QString type;
        QString id;

        StringMap entries;
    };

    using NodeMap = std::map<QString,Node>;

    struct Sensor
    {
        QString type;
        QString id;
        QString node;
        QString input;

        StringMap config;
    };

    using SensorMap = std::map<QString,Sensor>;

    struct Testpoint
    {
        QString id;
        QString conductance;
        QString orp;
        QString ph;
        QString potentiostat;
        QString temperature;
    };

    using TestpointMap = std::map<QString,Testpoint>;

public:
    explicit Configuration(const QString& filename);

    auto tcpPort() const -> quint16;

    auto statusPort() const -> QString;

    auto nodes() const -> const NodeMap&;
    auto sensors() const -> const SensorMap&;
    auto testpoints() const -> const TestpointMap&;

private:
    auto parseEntries(const QDomNode& parent,
                      const StringList& validKeys = {}) const -> StringMap;

    auto parseSensor(const QDomElement& parent) const -> Sensor;
    auto parseTestpoint(const QDomNode& parent) const -> StringMap;

    void parseNodes(const QDomNode& parent);
    void parseSensors(const QDomNode& parent);
    void parseTestpoints(const QDomNode& parent);

    static auto getSensorTypeValid(const QString& type) -> bool;

private:
    const QString m_filename;

    quint16 m_tcpPort = 5432;

    QString m_statusPort = "ttyStatus0";

    NodeMap m_nodes;
    SensorMap m_sensors;
    TestpointMap m_testpoints;
};
