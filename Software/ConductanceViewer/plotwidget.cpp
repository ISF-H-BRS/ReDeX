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

#include "device.h"
#include "plotwidget.h"

#include <qwt_plot_grid.h>
#include <qwt_scale_draw.h>
#include <qwt_symbol.h>
#include <qwt_text.h>

#include <QApplication>
#include <QVBoxLayout>
#include <QWheelEvent>

#include <cmath> // for round()

// ---------------------------------------------------------------------------------------------- //

namespace {
    constexpr std::array<double, PlotWidget::ZoomLevelCount> ZoomLevelSeconds = {
        1.0, 0.5, 0.2, 0.1, 0.05, 0.02, 0.01, 0.005
    };
}

// ---------------------------------------------------------------------------------------------- //

PlotWidget::PlotWidget(QWidget* parent)
    : QWidget(parent)
{
    static constexpr QColor BackgroundColor(0xff, 0xff, 0xff);
    static constexpr QColor GridColor(0xa0, 0xa0, 0xa4);
    static constexpr QColor CurveColor(0x33, 0x22, 0x88);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    m_plot = new QwtPlot(this);
    m_plot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(m_plot);

    QFont font = QApplication::font();

    m_plot->setAxisFont(QwtPlot::xBottom, font);
    m_plot->setAxisFont(QwtPlot::yLeft, font);

    auto scaleDraw = m_plot->axisScaleDraw(QwtPlot::xBottom);
    scaleDraw->enableComponent(QwtAbstractScaleDraw::Labels, false);

    auto grid = new QwtPlotGrid;
    grid->setPen(QPen(GridColor, 0, Qt::DotLine));
    grid->attach(m_plot);

    m_plot->setCanvasBackground(QBrush(BackgroundColor));
    m_plot->canvas()->setCursor(Qt::ArrowCursor);

    m_curve = new QwtPlotCurve;
    m_curve->setPen(QPen(CurveColor, 2.0, Qt::SolidLine));
    m_curve->attach(m_plot);

    setAutoScale(false);
    scrollTo(0.0);
    updateAxisText();
}

// ---------------------------------------------------------------------------------------------- //

void PlotWidget::setData(const DataBuffer& buffer, Device::Channel channel)
{
    const double* offset  = buffer.offset().data();
    const double* values = buffer.data(Device::toChannel(channel)).data();

    m_curve->setRawSamples(offset, values, DataBuffer::DataSize);
}

// ---------------------------------------------------------------------------------------------- //

void PlotWidget::setYAxisText(const QString& text)
{
    setAxisText(QwtPlot::yLeft, text);
}

// ---------------------------------------------------------------------------------------------- //

void PlotWidget::setZoomLevel(ZoomLevel level)
{
    m_zoomLevel = level;

    updateAxisZoom();
    updateAxisText();
}

// ---------------------------------------------------------------------------------------------- //

auto PlotWidget::zoomLevel() const -> ZoomLevel
{
    return m_zoomLevel;
}

// ---------------------------------------------------------------------------------------------- //

void PlotWidget::zoomIn()
{
    if (m_zoomLevel == MaximumZoomLevel)
        return;

    setZoomLevel(toZoomLevel(indexOf(m_zoomLevel) + 1));
}

// ---------------------------------------------------------------------------------------------- //

void PlotWidget::zoomOut()
{
    if (m_zoomLevel == MinimumZoomLevel)
        return;

    setZoomLevel(toZoomLevel(indexOf(m_zoomLevel) - 1));
}

// ---------------------------------------------------------------------------------------------- //

void PlotWidget::setAutoScale(bool enable)
{
    static constexpr auto MaximumAmplitude = Device::MaximumAmplitude;

    if (enable)
        m_plot->setAxisAutoScale(QwtPlot::yLeft);
    else
        m_plot->setAxisScale(QwtPlot::yLeft, -MaximumAmplitude, +MaximumAmplitude);
}

// ---------------------------------------------------------------------------------------------- //

void PlotWidget::scrollTo(double seconds)
{
    Q_ASSERT(seconds >= 0.0 && seconds <= 1.0);

    double minValue = seconds;
    double maxValue = seconds + ZoomLevelSeconds[indexOf(m_zoomLevel)];

    if (maxValue > 1.0)
    {
        minValue = 1.0 - ZoomLevelSeconds[indexOf(m_zoomLevel)];
        maxValue = 1.0;
    }

    m_plot->setAxisScale(QwtPlot::xBottom, minValue, maxValue);
    m_plot->replot();
}

// ---------------------------------------------------------------------------------------------- //

void PlotWidget::replot()
{
    m_plot->replot();
}

// ---------------------------------------------------------------------------------------------- //

auto PlotWidget::toSeconds(ZoomLevel level) -> double
{
    return ZoomLevelSeconds[indexOf(level)];
}

// ---------------------------------------------------------------------------------------------- //

void PlotWidget::wheelEvent(QWheelEvent* event)
{
    QWidget::wheelEvent(event);
    emit scrollRequested(event->angleDelta().y());
}

// ---------------------------------------------------------------------------------------------- //

void PlotWidget::setAxisText(QwtAxisId axis, const QString& text)
{
    QFont font = QApplication::font();
    font.setBold(true);

    QwtText title;
    title.setFont(font);
    title.setText(text);

    m_plot->setAxisTitle(axis, title);
}

// ---------------------------------------------------------------------------------------------- //

void PlotWidget::updateAxisZoom()
{
    const QwtScaleDiv& scaleDiv = m_plot->axisScaleDiv(QwtPlot::xBottom);
    scrollTo(scaleDiv.lowerBound());
}

// ---------------------------------------------------------------------------------------------- //

void PlotWidget::updateAxisText()
{
    const QwtScaleDiv& scaleDiv = m_plot->axisScaleDiv(QwtPlot::xBottom);

    const QList<double> ticks = scaleDiv.ticks(QwtScaleDiv::MajorTick);
    Q_ASSERT(ticks.size() >= 2);

    const double stepSize = std::round(1000.0 * (ticks.at(1) - ticks.at(0)));
    setAxisText(QwtPlot::xBottom, QString("Time (%1 ms / div)").arg(static_cast<int>(stepSize)));
}

// ---------------------------------------------------------------------------------------------- //
