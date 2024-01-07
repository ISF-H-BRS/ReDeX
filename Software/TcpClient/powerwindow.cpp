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

#include "powerwindow.h"
#include "ui_powerwindow.h"

// ---------------------------------------------------------------------------------------------- //

PowerWindow::PowerWindow(const std::vector<redex::NodeInfo>& infos, QWidget* parent)
    : QWidget(parent),
      m_ui(std::make_unique<Ui::PowerWindow>())
{
    m_ui->setupUi(this);

    QFormLayout* layout = m_ui->nodesLayout;
    Q_ASSERT(layout != nullptr);

    for (const auto& info : infos)
    {
        auto label = new QLabel("-");
        m_powerLabels[info.id.c_str()] = label;
        layout->addRow(QString("%1:").arg(info.id.c_str()), label);
    }
}

// ---------------------------------------------------------------------------------------------- //

PowerWindow::~PowerWindow() = default;

// ---------------------------------------------------------------------------------------------- //

void PowerWindow::setHubStatus(double temperature)
{
    m_ui->hubTemperature->setText(QString("%1 °C").arg(temperature, 0, 'f', 1));
}

// ---------------------------------------------------------------------------------------------- //

void PowerWindow::setNodeStatus(const QString& id,
                                double voltage, double current, double temperature)
{
    if (m_powerLabels.contains(id))
    {
        const auto text = QString("%1 V @ %2 A, %3 °C").arg(voltage, 0, 'f', 2)
                                                       .arg(current, 0, 'f', 3)
                                                       .arg(temperature, 0, 'f', 1);
        m_powerLabels.at(id)->setText(text);
    }
}

// ---------------------------------------------------------------------------------------------- //

void PowerWindow::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    emit visibilityChanged(true);
}

// ---------------------------------------------------------------------------------------------- //

void PowerWindow::closeEvent(QCloseEvent* event)
{
    emit visibilityChanged(false);
    QWidget::closeEvent(event);
}

// ---------------------------------------------------------------------------------------------- //
