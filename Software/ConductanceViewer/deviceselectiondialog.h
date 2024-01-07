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

#include "device.h"

#include <QDialog>
#include <QRadioButton>

namespace Ui {
    class DeviceSelectionDialog;
}

class DeviceSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    DeviceSelectionDialog(const std::vector<DeviceInfo>& devices, QWidget* parent = nullptr);
    ~DeviceSelectionDialog();

    DeviceSelectionDialog(const DeviceSelectionDialog&) = delete;
    DeviceSelectionDialog(DeviceSelectionDialog&&) = delete;

    auto operator=(const DeviceSelectionDialog&) = delete;
    auto operator=(DeviceSelectionDialog&&) = delete;

    auto selectedIndex() const -> ssize_t;

private slots:
    void saveIndex();

private:
    std::unique_ptr<Ui::DeviceSelectionDialog> m_ui;
    std::vector<QRadioButton*> m_buttons;

    ssize_t m_selectedIndex = -1;
};
