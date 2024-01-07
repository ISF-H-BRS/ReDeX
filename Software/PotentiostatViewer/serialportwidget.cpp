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

#include "serialportwidget.h"
#include "ui_serialportwidget.h"

#include <QSerialPortInfo>

// ---------------------------------------------------------------------------------------------- //

SerialPortWidget::SerialPortWidget(QWidget* parent)
    : QWidget(parent),
      m_ui(std::make_unique<Ui::SerialPortWidget>())
{
    m_ui->setupUi(this);
    refresh();

    connect(m_ui->serialPort, SIGNAL(currentTextChanged(QString)),
            this, SIGNAL(serialPortChanged(QString)));

    connect(m_ui->refreshButton, SIGNAL(clicked()), this, SLOT(refresh()));
}

// ---------------------------------------------------------------------------------------------- //

SerialPortWidget::~SerialPortWidget() = default;

// ---------------------------------------------------------------------------------------------- //

auto SerialPortWidget::selectedSerialPort() const -> QString
{
    return m_ui->serialPort->currentText();
}

// ---------------------------------------------------------------------------------------------- //

void SerialPortWidget::refresh()
{
    m_ui->serialPort->clear();

    const QList<QSerialPortInfo> infos = QSerialPortInfo::availablePorts();

    for (const QSerialPortInfo& info : infos)
        m_ui->serialPort->addItem(info.portName());
}

// ---------------------------------------------------------------------------------------------- //
