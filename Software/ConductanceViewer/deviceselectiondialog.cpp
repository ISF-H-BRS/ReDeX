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

#include "deviceselectiondialog.h"
#include "ui_deviceselectiondialog.h"

#include <QPushButton>

// ---------------------------------------------------------------------------------------------- //

DeviceSelectionDialog::DeviceSelectionDialog(const std::vector<DeviceInfo>& devices,
                                             QWidget* parent)
    : QDialog(parent),
      m_ui(std::make_unique<Ui::DeviceSelectionDialog>())
{
    m_ui->setupUi(this);

    for (const auto& info : devices)
    {
        const auto bus = static_cast<int>(info.busNumber());
        const auto port = static_cast<int>(info.portNumber());
        const std::string& serial = info.serialNumber();

        const auto label =
                QString("Bus %1, port %2, serial number %3").arg(bus).arg(port).arg(serial.c_str());

        auto button = new QRadioButton(label, m_ui->deviceGroup);
        m_ui->deviceGroup->layout()->addWidget(button);
        m_buttons.push_back(button);
    }

    if (m_buttons.empty())
        m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    else
        m_buttons.at(0)->setChecked(true);

    connect(this, SIGNAL(accepted()), this, SLOT(saveIndex()));
}

// ---------------------------------------------------------------------------------------------- //

DeviceSelectionDialog::~DeviceSelectionDialog() = default;

// ---------------------------------------------------------------------------------------------- //

auto DeviceSelectionDialog::selectedIndex() const -> ssize_t
{
    return m_selectedIndex;
}

// ---------------------------------------------------------------------------------------------- //

void DeviceSelectionDialog::saveIndex()
{
    m_selectedIndex = -1;

    for (size_t i = 0; i < m_buttons.size(); ++i)
    {
        if (m_buttons.at(i)->isChecked())
        {
            m_selectedIndex = static_cast<ssize_t>(i);
            break;
        }
    }
}

// ---------------------------------------------------------------------------------------------- //
