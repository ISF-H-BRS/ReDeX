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
#include "devicemanager.h"
#include "devicerunner.h"
#include "macro.h"
#include "messagedispatcher.h"
#include "powermonitor.h"
#include "statusboard.h"

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

#include <memory>

class TcpServer : public QObject
{
    Q_OBJECT
    REDEX_DELETE_COPY_MOVE(TcpServer);

public:
    TcpServer(const Configuration& config, const DeviceManager& devices);
    ~TcpServer() override;

public slots:
    void start();
    void stop();

    void disconnect();

private slots:
    void onNewConnection();
    void onDisconnect();

    void onIncomingData();

    void onNodeInfoRequested();
    void onTestpointInfoRequested();

    void onStartPowerMonitorReceived();
    void onStopPowerMonitorReceived();

    void onStartMeasurementReceived();
    void onStopMeasurementReceived();

    void handleRecord(const QString& data);
    void handleError(const QString& msg);

    void updateAlarmStatus(size_t severityIndex);

private:
    void startMeasurement();
    void stopMeasurement();

    void sendStatus(const QString& status);

    void sendData(const QString& data);
    void sendError(const QString& message);

    static auto sendData(QTcpSocket* socket, QString data) -> qint64;
    static void sendError(QTcpSocket* socket, const QString& message);

    static auto makeNodeInfo(const DeviceManager& devices) -> QString;
    static auto makeTestpointInfo(const DeviceManager& devices) -> QString;

private:
    const quint16 m_portNumber;

    QTcpServer m_server;
    QTcpSocket* m_socket = nullptr;

    MessageDispatcher m_dispatcher;
    DeviceRunner m_deviceRunner;
    PowerMonitor m_powerMonitor;
    StatusBoard m_statusBoard;

    bool m_measurementRunning = false;
    bool m_criticalState = false;

    const QString m_nodeInfo;
    const QString m_testpointInfo;
};
