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
#include "logger.h"
#include "protocol.h"
#include "tcpserver.h"

#include <QCoreApplication>
#include <QDateTime>

// ---------------------------------------------------------------------------------------------- //

TcpServer::TcpServer(const Configuration& config, const DeviceManager& devices)
    : m_portNumber(config.tcpPort()),
      m_deviceRunner(devices),
      m_powerMonitor(devices),
      m_statusBoard(config.statusPort()),
      m_nodeInfo(makeNodeInfo(devices)),
      m_testpointInfo(makeTestpointInfo(devices))
{
    connect(&m_server, SIGNAL(newConnection()), this, SLOT(onNewConnection()));

    connect(&m_dispatcher, SIGNAL(nodeInfoRequested()),
            this, SLOT(onNodeInfoRequested()));
    connect(&m_dispatcher, SIGNAL(testpointInfoRequested()),
            this, SLOT(onTestpointInfoRequested()));

    connect(&m_dispatcher, SIGNAL(startPowerMonitorReceived()),
            this, SLOT(onStartPowerMonitorReceived()));
    connect(&m_dispatcher, SIGNAL(stopPowerMonitorReceived()),
            this, SLOT(onStopPowerMonitorReceived()));

    connect(&m_dispatcher, SIGNAL(startMeasurementReceived()),
            this, SLOT(onStartMeasurementReceived()));
    connect(&m_dispatcher, SIGNAL(stopMeasurementReceived()),
            this, SLOT(onStopMeasurementReceived()));

    connect(&m_deviceRunner, SIGNAL(recordAvailable(QString)),
            this, SLOT(handleRecord(QString)));
    connect(&m_deviceRunner, SIGNAL(error(QString)),
            this, SLOT(handleError(QString)));

    connect(&m_powerMonitor, SIGNAL(alarmStatusChanged(size_t)),
            this, SLOT(updateAlarmStatus(size_t)));
    connect(&m_powerMonitor, SIGNAL(recordAvailable(QString)),
            this, SLOT(handleRecord(QString)));
    connect(&m_powerMonitor, SIGNAL(error(QString)),
            this, SLOT(handleError(QString)));
}

// ---------------------------------------------------------------------------------------------- //

TcpServer::~TcpServer()
{
    stop();
}

// ---------------------------------------------------------------------------------------------- //

void TcpServer::start()
{
    if (m_server.isListening())
        return;

    if (!m_server.listen(QHostAddress::Any, m_portNumber))
    {
        Logger::error("Unable to start the server: " + m_server.errorString() + ".");
        return QCoreApplication::quit();
    }

    m_deviceRunner.start();
    m_powerMonitor.start();

    Logger::info(QString("TCP server started. Listening on port %1.").arg(m_server.serverPort()));
}

// ---------------------------------------------------------------------------------------------- //

void TcpServer::stop()
{
    m_powerMonitor.stop();
    m_deviceRunner.stop();

    if (m_socket && m_socket->isOpen())
        m_socket->close();

    if (m_server.isListening())
    {
        m_server.close();
        Logger::info("TCP server stopped.");
    }
}

// ---------------------------------------------------------------------------------------------- //

void TcpServer::disconnect()
{
    if (m_socket && m_socket->isOpen())
        m_socket->close();
}

// ---------------------------------------------------------------------------------------------- //

void TcpServer::onNewConnection()
{
    if (m_socket)
    {
        const QString message = "Another client is already connected.";

        QTcpSocket* socket = m_server.nextPendingConnection();
        sendError(socket, message);
        socket->close();

        Logger::warning(message);
    }
    else
    {
        m_socket = m_server.nextPendingConnection();
        connect(m_socket, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
        connect(m_socket, SIGNAL(readyRead()), this, SLOT(onIncomingData()));

        sendData("<WELCOME>");

        Logger::info("Connection from " + m_socket->peerAddress().toString() + " established.");
    }
}

// ---------------------------------------------------------------------------------------------- //

void TcpServer::onDisconnect()
{
    RETURN_IF_NULL(m_socket);

    Logger::info("Connection to " + m_socket->peerAddress().toString() + " closed.");

    m_socket->deleteLater();
    m_socket = nullptr;

    if (m_measurementRunning)
        stopMeasurement();
}

// ---------------------------------------------------------------------------------------------- //

void TcpServer::onIncomingData()
{
    if (!m_socket || !m_socket->isOpen()) // May have been closed by peer
        return;

    try {
        const QByteArray data = m_socket->readAll();
        m_dispatcher.process(data);
    }
    catch (const std::exception& e) {
        Logger::warning(QString(e.what()) + " Ignoring.");
    }
}

// ---------------------------------------------------------------------------------------------- //

void TcpServer::onNodeInfoRequested()
{
    const QString data = Protocol::joinTokens("<NODE_INFO>", m_nodeInfo);
    sendData(data);

    Logger::info("Node info sent.");
}

// ---------------------------------------------------------------------------------------------- //

void TcpServer::onTestpointInfoRequested()
{
    const QString data = Protocol::joinTokens("<TESTPOINT_INFO>", m_testpointInfo);
    sendData(data);

    Logger::info("Testpoint info sent.");
}

// ---------------------------------------------------------------------------------------------- //

void TcpServer::onStartPowerMonitorReceived()
{
    m_powerMonitor.setNotificationEnabled(true);
}

// ---------------------------------------------------------------------------------------------- //

void TcpServer::onStopPowerMonitorReceived()
{
    m_powerMonitor.setNotificationEnabled(false);
}

// ---------------------------------------------------------------------------------------------- //

void TcpServer::onStartMeasurementReceived()
{
    startMeasurement();
}

// ---------------------------------------------------------------------------------------------- //

void TcpServer::onStopMeasurementReceived()
{
    stopMeasurement();
}

// ---------------------------------------------------------------------------------------------- //

void TcpServer::handleRecord(const QString& data)
{
    sendData(data);
}

// ---------------------------------------------------------------------------------------------- //

void TcpServer::handleError(const QString& msg)
{
    stopMeasurement();

    const QString message = "A fatal error occured. " + msg;
    sendError(message);

    Logger::error(message);
}

// ---------------------------------------------------------------------------------------------- //

void TcpServer::updateAlarmStatus(size_t severityIndex)
{
    if (severityIndex > Alarm::indexOf(Alarm::Severity::None))
    {
        if (severityIndex > Alarm::indexOf(Alarm::Severity::Warning))
        {
            stopMeasurement();

            const QString message = "Alarm status changed to 'critical'.";
            sendError(message);

            Logger::error(message);

            m_statusBoard.setStatus(StatusBoard::Status::Alarm);
            m_criticalState = true;
        }
        else
            Logger::warning("Alarm status changed to 'warning'.");
    }
    else
        Logger::info("Alarm status changed to 'none'.");
}

// ---------------------------------------------------------------------------------------------- //

void TcpServer::startMeasurement()
{
    if (m_criticalState)
    {
        const QString message = "Device is in critial state.";
        sendError(message);

        Logger::warning(message + " Ignoring request to start measurement.");
        return;
    }

    m_deviceRunner.startMeasurement();
    m_statusBoard.setStatus(StatusBoard::Status::Active);
    m_measurementRunning = true;

    sendStatus("running");

    Logger::info("Measurement started.");
}

// ---------------------------------------------------------------------------------------------- //

void TcpServer::stopMeasurement()
{
    m_deviceRunner.stopMeasurement();
    m_statusBoard.setStatus(StatusBoard::Status::Idle);
    m_measurementRunning = false;

    sendStatus("idle");

    Logger::info("Measurement stopped.");
}

// ---------------------------------------------------------------------------------------------- //

void TcpServer::sendStatus(const QString& status)
{
    const QString data = Protocol::joinTokens("<STATUS>", status);
    sendData(data);
}

// ---------------------------------------------------------------------------------------------- //

void TcpServer::sendData(const QString& data)
{
    if (!m_socket)
        return;

    const qint64 result = sendData(m_socket, data);

    if (result <= 0)
    {
        Logger::error("Unable to send data to client.");
        onDisconnect();
    }
}

// ---------------------------------------------------------------------------------------------- //

void TcpServer::sendError(const QString& message)
{
    if (!m_socket)
        return;

    sendError(m_socket, message);
}

// ---------------------------------------------------------------------------------------------- //

auto TcpServer::sendData(QTcpSocket* socket, QString data) -> qint64
{
    RETURN_IF_NULL(socket);

    if (!socket->isOpen())
        return 0;

    data += Protocol::LineBreak;
    return socket->write(data.toUtf8());
}

// ---------------------------------------------------------------------------------------------- //

void TcpServer::sendError(QTcpSocket* socket, const QString& message)
{
    const QString data = Protocol::joinTokens("<ERROR>", message);
    sendData(socket, data);
}

// ---------------------------------------------------------------------------------------------- //

auto TcpServer::makeNodeInfo(const DeviceManager& devices) -> QString
{
    const std::vector<Node*>& nodes = devices.nodes();

    auto info = QString::number(nodes.size());

    for (auto node : nodes)
    {
        info += Protocol::TokenSeparator;
        info += node->id();
        info += Protocol::ValueSeparator;
        info += node->type();
    }

    return info;
}

// ---------------------------------------------------------------------------------------------- //

auto TcpServer::makeTestpointInfo(const DeviceManager& devices) -> QString
{
    static const auto makeSensorInfo = [](Sensor* sensor) -> QString
    {
        const QString id     = sensor ? sensor->id()                     : "";
        const QString nodeId = sensor ? sensor->node()->id()             : "";
        const QString input  = sensor ? QString::number(sensor->input()) : "";

        return Protocol::ValueSeparator + id +
               Protocol::ValueSeparator + nodeId +
               Protocol::ValueSeparator + input;
    };

    const std::vector<Testpoint*>& testpoints = devices.testpoints();

    auto info = QString::number(testpoints.size());

    for (auto testpoint : testpoints)
    {
        info += Protocol::TokenSeparator;
        info += testpoint->id();
        info += makeSensorInfo(testpoint->conductanceSensor());
        info += makeSensorInfo(testpoint->orpSensor());
        info += makeSensorInfo(testpoint->phSensor());
        info += makeSensorInfo(testpoint->potentiostatSensor());
        info += makeSensorInfo(testpoint->temperatureSensor());
    }

    return info;
}

// ---------------------------------------------------------------------------------------------- //
