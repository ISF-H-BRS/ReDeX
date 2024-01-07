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

#include "signalfilter.h"

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

#include <vector>

class PlotWidget : public QwtPlot
{
public:
    PlotWidget(QWidget* parent = nullptr);

    void setAxisTitles(const QString& xTitle, const QString& yTitle);
    void setFiltered(bool enable);

    void addSamples(std::span<const double> xData, std::span<const double> yData);
    void addFilteredSamples(std::span<const double> xData, std::span<const double> yData);

    void clear();

    auto size() const -> size_t;
    auto filteredSize() const -> size_t;

    auto getXData() const -> std::vector<double>;
    auto getYData() const -> std::vector<double>;

    auto getFilteredXData() const -> std::vector<double>;
    auto getFilteredYData() const -> std::vector<double>;

private:
    class Data;
    auto getData(Data* data, QwtAxisId axis) const -> std::vector<double>;

private:
    Data* m_data = nullptr;
    QwtPlotCurve* m_curve = nullptr;

    Data* m_filteredData = nullptr;
    QwtPlotCurve* m_filteredCurve = nullptr;

    bool m_filtered = false;
};

