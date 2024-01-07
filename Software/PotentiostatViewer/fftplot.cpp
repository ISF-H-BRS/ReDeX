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

#include "fftplot.h"

#include <qwt_plot_grid.h>
#include <qwt_scale_engine.h>
#include <qwt_text.h>

#include <QApplication>
#include <QPen>

// ---------------------------------------------------------------------------------------------- //

class FftPlot::Data : public QwtCPointerData<double>
{
public:
    Data(const double* x, const double* y, size_t size)
        : QwtCPointerData(x, y, size) {}

    void updateBoundingRect() {
        cachedBoundingRect = qwtBoundingRect(*this);
    }
};

// ---------------------------------------------------------------------------------------------- //

FftPlot::FftPlot(QWidget* parent)
    : QwtPlot(parent),
      m_context(WindowSize),
      m_samples(WindowSize),
      m_filteredSamples(WindowSize),
      m_windowedSamples(WindowSize),
      m_results(SpectrumSize),
      m_frequencies(SpectrumSize),
      m_magnitudes(SpectrumSize)
{
    std::iota(m_frequencies.begin(), m_frequencies.end(), 0);

    QFont font = QApplication::font();
    font.setPointSizeF(0.8 * font.pointSizeF());

    setAxisFont(QwtPlot::xBottom, font);
    setAxisFont(QwtPlot::yLeft, font);

    setTitle(QwtPlot::xBottom, "Frequency (Hz)");
    setTitle(QwtPlot::yLeft, "Magnitude");

    setAxisScale(QwtPlot::xBottom, 0, MaximumFrequency);

    setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine);
    setAxisScale(QwtPlot::yLeft, 1.0e-6, 1.0e-3);
    setAxisAutoScale(QwtPlot::yLeft, true);
    setAxisMaxMinor(QwtPlot::yLeft, 10);

    setCanvasBackground(QBrush(QColor(245, 245, 245)));

    auto grid = new QwtPlotGrid;
    grid->setPen(QPen(Qt::gray, 0, Qt::DotLine));
    grid->attach(this);

    m_data = new Data(m_frequencies.data(), m_magnitudes.data(), SpectrumSize);

    m_curve = new QwtPlotCurve;
    m_curve->setPen(QPen(QColor(0x33, 0x22, 0x88), 2, Qt::SolidLine));
    m_curve->setData(m_data);
    m_curve->attach(this);
}

// ---------------------------------------------------------------------------------------------- //

void FftPlot::showEvent(QShowEvent* event)
{
    QwtPlot::showEvent(event);
    reprocess();
}

// ---------------------------------------------------------------------------------------------- //

void FftPlot::setTitle(QwtAxisId axis, const QString& text)
{
    QFont font = QApplication::font();
    font.setBold(true);

    QwtText title;
    title.setFont(font);
    title.setText(text);

    setAxisTitle(axis, title);
}

// ---------------------------------------------------------------------------------------------- //

void FftPlot::setTitle(const QString& title)
{
    setTitle(QwtPlot::yLeft, title);
}

// ---------------------------------------------------------------------------------------------- //

void FftPlot::setFiltered(bool enable)
{
    m_filtered = enable;
    reprocess();
}

// ---------------------------------------------------------------------------------------------- //

void FftPlot::addSamples(std::span<const double> samples)
{
    std::copy(samples.begin(), samples.end(), std::back_inserter(m_samples));

    if (m_samples.size() > WindowSize)
        m_samples.erase(m_samples.begin(), m_samples.end() - WindowSize);

    if (!m_filtered)
        reprocess();
}

// ---------------------------------------------------------------------------------------------- //

void FftPlot::addFilteredSamples(std::span<const double> samples)
{
    std::copy(samples.begin(), samples.end(), std::back_inserter(m_filteredSamples));

    if (m_filteredSamples.size() > WindowSize)
        m_filteredSamples.erase(m_filteredSamples.begin(), m_filteredSamples.end() - WindowSize);

    if (m_filtered)
        reprocess();
}

// ---------------------------------------------------------------------------------------------- //

void FftPlot::clear()
{
    std::fill(m_magnitudes.begin(), m_magnitudes.end(), 0.0);
    m_data->updateBoundingRect();

    replot();
}

// ---------------------------------------------------------------------------------------------- //

void FftPlot::applyWindow(std::span<const double> in, std::span<double> out)
{
    static constexpr double Pi = 3.14159265358979323846;
    static constexpr double TwoPi = 2.0 * Pi;

    Q_ASSERT(in.size() == out.size() && in.size() > 1);

    const auto size = static_cast<int>(in.size());
    const int m = size - 1;

    for (int i = 0; i < size; ++i)
        out[i] = in[i] * (0.54 - 0.46*std::cos(TwoPi*i / m)); // Hamming
}

// ---------------------------------------------------------------------------------------------- //

void FftPlot::reprocess()
{
    if (!isVisible())
        return;

    if (m_filtered)
    {
        Q_ASSERT(m_filteredSamples.size() == WindowSize);
        applyWindow(m_filteredSamples, m_windowedSamples);
    }
    else
    {
        Q_ASSERT(m_samples.size() == WindowSize);
        applyWindow(m_samples, m_windowedSamples);
    }

    m_context.transform(m_windowedSamples, m_results);

    for (size_t i = 0; i < SpectrumSize; ++i)
    {
        m_magnitudes[i] = std::abs(m_results[i]) / WindowSize;

        if (i != 0)
            m_magnitudes[i] *= 2.0;
    }

    m_data->updateBoundingRect();

    replot();
}

// ---------------------------------------------------------------------------------------------- //
