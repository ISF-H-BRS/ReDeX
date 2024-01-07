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
#include "configuration.h"
#include "lockfile.h"
#include "logger.h"
#include "tcpserver.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>

#include <csignal>

// ---------------------------------------------------------------------------------------------- //

namespace {
    TcpServer* g_tcpServer = nullptr;
}

// ---------------------------------------------------------------------------------------------- //

void catchQuitSignals()
{
    const auto unixSignals = { SIGQUIT, SIGINT, SIGTERM };

    const auto handler = [](int) {
        QCoreApplication::quit();
    };

    for (auto sig : unixSignals)
        std::signal(sig, handler);
}

// ---------------------------------------------------------------------------------------------- //

void catchDisconnectSignal()
{
    const auto handler = [](int)
    {
        ASSERT_NOT_NULL(g_tcpServer);
        QMetaObject::invokeMethod(g_tcpServer, "disconnect", Qt::QueuedConnection);
    };

    std::signal(SIGHUP, handler);
}

// ---------------------------------------------------------------------------------------------- //

auto main(int argc, char* argv[]) -> int
{
    QCoreApplication::setOrganizationName("Bonn-Rhein-Sieg University of Applied Sciences");
    QCoreApplication::setApplicationName("ReDeX TCP Server");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("TCP interface to sensor network.");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption configOption({"c", "config"}, "Set path to configuration file.",
                                    "filename", "config.xml");
    parser.addOption(configOption);

    QCommandLineOption logfileOption({"l", "logfile"}, "Set path to log file.",
                                    "filename", "log.txt");
    parser.addOption(logfileOption);

    QCoreApplication application(argc, argv);
    parser.process(application);

    try {
        QString lockPath = QDir::temp().absoluteFilePath("redex-tcp-server.lock");
        LockFile lock(lockPath);

        Logger::setLogFile(parser.value(logfileOption));

        Configuration config(parser.value(configOption));
        DeviceManager devices(config);

        TcpServer server(config, devices);
        g_tcpServer = &server;

        catchQuitSignals();
        catchDisconnectSignal();

        QMetaObject::invokeMethod(&server, "start", Qt::QueuedConnection);

        return QCoreApplication::exec();
    }
    catch (const std::exception& e)
    {
        Logger::error(e.what());
        return -1;
    }
}

// ---------------------------------------------------------------------------------------------- //
