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

#include "phwidget.h"
#include "ui_phwidget.h"

#include <sensors/device.h>

#include <QLabel>
#include <QVBoxLayout>

// ---------------------------------------------------------------------------------------------- //

PhWidget::PhWidget(QWidget* parent)
    : SensorWidget(parent),
      m_ui(std::make_unique<Ui::PhWidget>())
{
    m_ui->setupUi(this);

    QFont font = m_ui->value->font();
    font.setPointSize(3 * font.pointSize());
    m_ui->value->setFont(font);

    updateLabel();

    connect(m_ui->phButton, SIGNAL(toggled(bool)), this, SLOT(onDisplayChanged(bool)));
    connect(m_ui->temperature, SIGNAL(valueChanged(double)), this, SLOT(onTemperatureChanged()));
}

// ---------------------------------------------------------------------------------------------- //

PhWidget::~PhWidget() = default;

// ---------------------------------------------------------------------------------------------- //

void PhWidget::setValue(double value)
{
    m_value = value;
    updateLabel();
}

// ---------------------------------------------------------------------------------------------- //

void PhWidget::onDisplayChanged(bool ph)
{
    updateLabel();
    m_ui->temperatureWidget->setVisible(ph);
}

// ---------------------------------------------------------------------------------------------- //

void PhWidget::onTemperatureChanged()
{
    updateLabel();
}

// ---------------------------------------------------------------------------------------------- //

void PhWidget::updateLabel()
{
    using Device = isf::Sensors::Device;

    if (m_ui->phButton->isChecked())
    {
        const double temperature = m_ui->temperature->value();
        m_ui->value->setText(QString("%1").arg(Device::toPhValue(m_value, temperature), 0, 'f', 2));
    }
    else
        m_ui->value->setText(QString("%1 mV").arg(1000.0 * m_value, 0, 'f', 1));
}

// ---------------------------------------------------------------------------------------------- //
