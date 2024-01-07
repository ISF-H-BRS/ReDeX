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

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_point_data.h>

#include <sft/sft.h>

#include <span>
#include <vector>

class FftPlot : public QwtPlot
{
    Q_OBJECT

public:
    static constexpr size_t WindowSize = Device::SampleRate;
    static constexpr size_t SpectrumSize = WindowSize/2 + 1;

    static constexpr double MaximumFrequency = WindowSize * 0.5;

public:
    FftPlot(QWidget* parent);

    void setTitle(const QString& title);
    void setFiltered(bool enable);

    void addSamples(std::span<const double> samples);
    void addFilteredSamples(std::span<const double> samples);

    void clear();

private:
    void showEvent(QShowEvent* event) override;

    void setTitle(QwtAxisId axis, const QString& text);

    void applyWindow(std::span<const double> in, std::span<double> out);
    void reprocess();

private:
    sft::Context<sft::Real> m_context;

    class Data;
    Data* m_data = nullptr;

    QwtPlotCurve* m_curve = nullptr;

    std::vector<double> m_samples;
    std::vector<double> m_filteredSamples;

    std::vector<double> m_windowedSamples;
    std::vector<sft::Complex> m_results;

    std::vector<double> m_frequencies;
    std::vector<double> m_magnitudes;

    bool m_filtered = false;
};
