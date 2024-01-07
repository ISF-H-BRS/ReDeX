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

#include "plotwindow.h"
#include "ui_plotwindow.h"

#include <redex.h>

PlotWindow::PlotWindow(QWidget* parent)
    : QWidget(parent),
      m_ui(std::make_unique<Ui::PlotWindow>())
{
    m_ui->setupUi(this);

    m_ui->voltammogramPlot->setAxisTitles("Voltage (V)", "Current (A)");
    m_ui->voltagePlot->setAxisTitles("Time (s)", "Voltage (V)");
    m_ui->currentPlot->setAxisTitles("Time (s)", "Current (A)");

    hide();
}

// ---------------------------------------------------------------------------------------------- //

PlotWindow::~PlotWindow() = default;

// ---------------------------------------------------------------------------------------------- //

void PlotWindow::setSamples(std::span<const double> voltage, std::span<const double> current)
{
    m_voltage.resize(voltage.size());
    m_current.resize(current.size());

    redex::Client::filterVoltammetryData(voltage, m_voltage);
    redex::Client::filterVoltammetryData(current, m_current);

    m_ui->voltammogramPlot->setSamples(m_voltage, m_current);

    updateTimes(m_voltage.size());
    Q_ASSERT(m_times.size() == m_voltage.size());

    m_ui->voltagePlot->setSamples(m_times, m_voltage);
    m_ui->currentPlot->setSamples(m_times, m_current);
}

// ---------------------------------------------------------------------------------------------- //

void PlotWindow::updateTimes(size_t size)
{
    static constexpr double Period = 1.0 / 1000.0; // 1 kHz

    const size_t oldSize = m_times.size();
    m_times.resize(size);

    if (size > oldSize)
    {
        for (size_t i = oldSize; i < size; ++i)
            m_times[i] = i * Period;
    }

}

// ---------------------------------------------------------------------------------------------- //
