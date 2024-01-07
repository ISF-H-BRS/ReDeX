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

#include "testpointwidget.h"
#include "units.h"

#include "ui_testpointwidget.h"

// ---------------------------------------------------------------------------------------------- //

TestpointWidget::TestpointWidget(const redex::TestpointInfo& info, QWidget* parent)
    : QWidget(parent),
      m_ui(std::make_unique<Ui::TestpointWidget>()),
      m_id(info.testpointId.c_str())
{
    static const auto makeToolTip = [](const redex::SensorInfo& info)
    {
        const QString toolTip = "Sensor ID: %1\nNode ID: %2\nInput: %3";
        return toolTip.arg(info.sensorId.c_str(), info.nodeId.c_str()).arg(info.input);
    };

    m_ui->setupUi(this);

    m_ui->admittanceLabel->setToolTip(makeToolTip(info.conductanceInfo));
    m_ui->phLabel->setToolTip(makeToolTip(info.phInfo));
    m_ui->orpLabel->setToolTip(makeToolTip(info.orpInfo));
    m_ui->temperatureLabel->setToolTip(makeToolTip(info.temperatureInfo));
    m_ui->voltammetryLabel->setToolTip(makeToolTip(info.potentiostatInfo));

    connect(m_ui->voltammetryValues, SIGNAL(linkActivated(QString)),
            this, SIGNAL(voltammogramRequested(QString)));
}

// ---------------------------------------------------------------------------------------------- //

TestpointWidget::~TestpointWidget() = default;

// ---------------------------------------------------------------------------------------------- //

void TestpointWidget::setAdmittance(double voltage, double current, double admittance)
{
    const QString voltageText = Units::makeAdjustedText(voltage, 4, "V");
    const QString currentText = Units::makeAdjustedText(current, 4, "A");
    const QString admittanceText = Units::makeAdjustedText(admittance, 4, "S");

    const QString text = "%1 (%2, %3)";
    m_ui->admittance->setText(text.arg(admittanceText, voltageText, currentText));
}

// ---------------------------------------------------------------------------------------------- //

void TestpointWidget::setPhValue(double value)
{
    m_ui->phValue->setText(QString("%1").arg(value));
}

// ---------------------------------------------------------------------------------------------- //

void TestpointWidget::setOrpValue(double value)
{
    m_ui->orpValue->setText(Units::makeAdjustedText(value, 4, "V"));
}

// ---------------------------------------------------------------------------------------------- //

void TestpointWidget::setTemperature(double value)
{
    m_ui->temperature->setText(QString("%1 °C").arg(value, 0, 'f', 1));
}

// ---------------------------------------------------------------------------------------------- //

void TestpointWidget::setVoltammetryValues(size_t count)
{
    const QString text = "%1 values received (<a href=\"%2\">view</a>)";
    m_ui->voltammetryValues->setText(text.arg(count).arg(m_id));
}

// ---------------------------------------------------------------------------------------------- //
