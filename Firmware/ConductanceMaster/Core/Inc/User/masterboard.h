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

#include "hostinterface.h"
#include "powermonitor.h"
#include "slaveinterface.h"

class MasterBoard : public HostInterface::Owner, public SlaveInterface::Owner
{
public:
    MasterBoard();
    ~MasterBoard();

    void exec();

private:
    void onInitComplete() override;

    void onControlOutReceived(uint8_t request, std::span<const uint8_t> data) override;
    void onControlInReceived(uint8_t request, std::span<uint8_t> data) override;

    void onBulkInComplete() override;
    void onSlaveBufferFull(std::span<const uint8_t> data) override;

    void sendNextBulkData();

private:
    HostInterface m_hostInterface;

    volatile bool m_hostInterfaceReady = false;
    volatile bool m_bulkInComplete = false;

    SlaveInterface m_slaveInterface;
    const uint8_t* m_nextData = nullptr;

    PowerMonitor m_powerMonitor;
};
