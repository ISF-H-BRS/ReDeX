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

#include "tcpclient.h"
#include "voltammogramfilter.h"

#include <redex.h>

// ---------------------------------------------------------------------------------------------- //

using namespace redex;

// ---------------------------------------------------------------------------------------------- //

class Client::Private
{
public:
    Private(const std::string& host, Listener* listener) : client(host, listener) {}
    TcpClient client;
};

// ---------------------------------------------------------------------------------------------- //

Client::Client(const std::string& host, Listener* listener)
    : d(std::make_unique<Private>(host, listener))
{
}

// ---------------------------------------------------------------------------------------------- //

Client::~Client() = default;

// ---------------------------------------------------------------------------------------------- //

void Client::requestNodeInfo()
{
    d->client.requestNodeInfo();
}

// ---------------------------------------------------------------------------------------------- //

void Client::requestTestpointInfo()
{
    d->client.requestTestpointInfo();
}

// ---------------------------------------------------------------------------------------------- //

void Client::startPowerMonitor()
{
    d->client.startPowerMonitor();
}

// ---------------------------------------------------------------------------------------------- //

void Client::stopPowerMonitor()
{
    d->client.stopPowerMonitor();
}

// ---------------------------------------------------------------------------------------------- //

void Client::startMeasurement()
{
    d->client.startMeasurement();
}

// ---------------------------------------------------------------------------------------------- //

void Client::stopMeasurement()
{
    d->client.stopMeasurement();
}

// ---------------------------------------------------------------------------------------------- //

void Client::filterVoltammetryData(std::span<const double> input, std::span<double> output)
{
    static VoltammogramFilter filter;
    filter.apply(input, output);
}

// ---------------------------------------------------------------------------------------------- //

void Listener::onNodeInfoReceived(std::span<const NodeInfo>) {}

// ---------------------------------------------------------------------------------------------- //

void Listener::onTestpointInfoReceived(std::span<const TestpointInfo>) {}

// ---------------------------------------------------------------------------------------------- //

void Listener::onStatusChanged(Status) {}

// ---------------------------------------------------------------------------------------------- //

void Listener::onConductanceReceived(const std::string&, double, double, double) {}

// ---------------------------------------------------------------------------------------------- //

void Listener::onOrpValueReceived(const std::string&, double) {}

// ---------------------------------------------------------------------------------------------- //

void Listener::onPhValueReceived(const std::string&, double) {}

// ---------------------------------------------------------------------------------------------- //

void Listener::onTemperatureReceived(const std::string&, double) {}

// ---------------------------------------------------------------------------------------------- //

void Listener::onVoltammogramReceived(const std::string&,
                                      std::span<const double>, std::span<const double>) {}

// ---------------------------------------------------------------------------------------------- //

void Listener::onHubAlarm(AlarmType, AlarmSeverity) {}

// ---------------------------------------------------------------------------------------------- //

void Listener::onNodeAlarm(const std::string&, AlarmType, AlarmSeverity) {}

// ---------------------------------------------------------------------------------------------- //

void Listener::onHubStatusReceived(double) {}

// ---------------------------------------------------------------------------------------------- //

void Listener::onNodeStatusReceived(const std::string&, double, double, double) {}

// ---------------------------------------------------------------------------------------------- //

void Listener::onError(const std::string&) {}

// ---------------------------------------------------------------------------------------------- //
