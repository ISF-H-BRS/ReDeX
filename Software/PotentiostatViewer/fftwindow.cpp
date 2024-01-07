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

#include "fftwindow.h"
#include "ui_fftwindow.h"

// ---------------------------------------------------------------------------------------------- //

FftWindow::FftWindow()
    : QWidget(nullptr),
      m_ui(std::make_unique<Ui::FftWindow>())
{
    m_ui->setupUi(this);

    m_ui->voltagePlot->setTitle("Voltage (V)");
    m_ui->currentPlot->setTitle("Current (A)");
}

// ---------------------------------------------------------------------------------------------- //

FftWindow::~FftWindow() = default;

// ---------------------------------------------------------------------------------------------- //

void FftWindow::showEvent(QShowEvent* event)
{
    emit visiblityChanged(true);
    QWidget::showEvent(event);
}

// ---------------------------------------------------------------------------------------------- //

void FftWindow::closeEvent(QCloseEvent* event)
{
    emit visiblityChanged(false);
    QWidget::closeEvent(event);
}

// ---------------------------------------------------------------------------------------------- //

void FftWindow::setFiltered(bool enable)
{
    m_ui->voltagePlot->setFiltered(enable);
    m_ui->currentPlot->setFiltered(enable);
}

// ---------------------------------------------------------------------------------------------- //

void FftWindow::addSamples(std::span<const double> voltages, std::span<const double> currents)
{
    Q_ASSERT(voltages.size() == currents.size());

    m_ui->voltagePlot->addSamples(voltages);
    m_ui->currentPlot->addSamples(currents);
}

// ---------------------------------------------------------------------------------------------- //

void FftWindow::addFilteredSamples(std::span<const double> voltages,
                                   std::span<const double> currents)
{
    Q_ASSERT(voltages.size() == currents.size());

    m_ui->voltagePlot->addFilteredSamples(voltages);
    m_ui->currentPlot->addFilteredSamples(currents);
}

// ---------------------------------------------------------------------------------------------- //

void FftWindow::clear()
{
    m_ui->voltagePlot->clear();
    m_ui->currentPlot->clear();
}

// ---------------------------------------------------------------------------------------------- //
