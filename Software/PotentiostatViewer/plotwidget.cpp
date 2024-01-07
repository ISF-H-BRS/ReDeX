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

#include "plotwidget.h"

#include <qwt_plot_grid.h>
#include <qwt_text.h>

#include <QApplication>
#include <QPen>

// ---------------------------------------------------------------------------------------------- //

class PlotWidget::Data : public QwtArraySeriesData<QPointF>
{
public:
    void append(std::span<const double> xData, std::span<const double> yData)
    {
        Q_ASSERT(xData.size() == yData.size());

        const size_t size = xData.size();

        if (size == 0)
            return;

        if (m_samples.isEmpty())
            cachedBoundingRect = QRectF(xData[0], yData[0], 0.0, 0.0);

        qreal left = cachedBoundingRect.left();
        qreal right = cachedBoundingRect.right();
        qreal bottom = cachedBoundingRect.bottom();
        qreal top = cachedBoundingRect.top();

        for (size_t i = 0; i < size; ++i)
        {
            m_samples.append(QPointF(xData[i], yData[i]));

            if (xData[i] < left)
                left = xData[i];
            else if (xData[i] > right)
                right = xData[i];

            if (yData[i] < top)
                top = yData[i];
            else if (yData[i] > bottom)
                bottom = yData[i];
        }

        cachedBoundingRect.setCoords(left, top, right, bottom);
    }

    void clear()
    {
        m_samples.clear();
        m_samples.squeeze();
        cachedBoundingRect = QRectF(0.0, 0.0, 0.0, 0.0);
    }

    auto boundingRect() const -> QRectF override {
        return cachedBoundingRect;
    }
};

// ---------------------------------------------------------------------------------------------- //

PlotWidget::PlotWidget(QWidget* parent)
    : QwtPlot(parent)
{
    static constexpr QColor CurveColor = { 0x33, 0x22, 0x88 };
    static constexpr double CurveWidth = 2.0;

    QFont font = QApplication::font();
    font.setPointSizeF(0.8 * font.pointSizeF());

    setAxisFont(QwtPlot::xBottom, font);
    setAxisFont(QwtPlot::yLeft, font);

    setAxisScale(QwtPlot::xBottom, -1.0, 1.0);
    setAxisScale(QwtPlot::yLeft, -1.0, 1.0);

    setAxisAutoScale(QwtPlot::xBottom, true);
    setAxisAutoScale(QwtPlot::yLeft, true);

    auto grid = new QwtPlotGrid;
    grid->setPen(QPen(Qt::gray, 0, Qt::DotLine));
    grid->attach(this);

    setCanvasBackground(QBrush(QColor(245, 245, 245)));
    canvas()->setCursor(Qt::ArrowCursor);

    m_data = new Data;
    m_filteredData = new Data;

    m_curve = new QwtPlotCurve;
    m_curve->setData(m_data);
    m_curve->setPen(QPen(CurveColor, CurveWidth, Qt::SolidLine));
    //m_curve->setRenderHint(QwtPlotCurve::RenderAntialiased);
    //m_curve->setCurveAttribute(QwtPlotCurve::Fitted);

    m_filteredCurve = new QwtPlotCurve;
    m_filteredCurve->setData(m_filteredData);
    m_filteredCurve->setPen(QPen(CurveColor, CurveWidth, Qt::SolidLine));
    //m_filteredCurve->setRenderHint(QwtPlotCurve::RenderAntialiased);
    //m_filteredCurve->setCurveAttribute(QwtPlotCurve::Fitted);

    m_curve->attach(this);
}

// ---------------------------------------------------------------------------------------------- //

void PlotWidget::setAxisTitles(const QString& xTitle, const QString& yTitle)
{
    QFont font = QApplication::font();
    font.setBold(true);

    QwtText title;
    title.setFont(font);

    title.setText(xTitle);
    setAxisTitle(QwtPlot::xBottom, title);

    title.setText(yTitle);
    setAxisTitle(QwtPlot::yLeft, title);
}

// ---------------------------------------------------------------------------------------------- //

void PlotWidget::setFiltered(bool enable)
{
    m_filtered = enable;
    m_curve->attach(m_filtered ? nullptr : this);
    m_filteredCurve->attach(m_filtered ? this : nullptr);

    replot();
}

// ---------------------------------------------------------------------------------------------- //

void PlotWidget::addSamples(std::span<const double> xData, std::span<const double> yData)
{
    m_data->append(xData, yData);

    if (!m_filtered)
        replot();
}

// ---------------------------------------------------------------------------------------------- //

void PlotWidget::addFilteredSamples(std::span<const double> xData, std::span<const double> yData)
{
    m_filteredData->append(xData, yData);

    if (m_filtered)
        replot();
}

// ---------------------------------------------------------------------------------------------- //

void PlotWidget::clear()
{
    m_data->clear();
    m_filteredData->clear();

    replot();
}

// ---------------------------------------------------------------------------------------------- //

auto PlotWidget::size() const -> size_t
{
    return m_data->size();
}

// ---------------------------------------------------------------------------------------------- //

auto PlotWidget::filteredSize() const -> size_t
{
    return m_filteredData->size();
}

// ---------------------------------------------------------------------------------------------- //

auto PlotWidget::getData(Data* data, QwtAxisId axis) const -> std::vector<double>
{
    Q_ASSERT(axis == QwtPlot::xBottom || axis == QwtPlot::yLeft);

    const size_t size = data->size();
    std::vector<double> result(size);

    for (size_t i = 0; i < size; ++i)
        result[i] = (axis == QwtPlot::xBottom) ? data->sample(i).x() : data->sample(i).y();

    return result;
}

// ---------------------------------------------------------------------------------------------- //

auto PlotWidget::getXData() const -> std::vector<double>
{
    return getData(m_data, QwtPlot::xBottom);
}

// ---------------------------------------------------------------------------------------------- //

auto PlotWidget::getYData() const -> std::vector<double>
{
    return getData(m_data, QwtPlot::yLeft);
}

// ---------------------------------------------------------------------------------------------- //

auto PlotWidget::getFilteredXData() const -> std::vector<double>
{
    return getData(m_filteredData, QwtPlot::xBottom);
}

// ---------------------------------------------------------------------------------------------- //

auto PlotWidget::getFilteredYData() const -> std::vector<double>
{
    return getData(m_filteredData, QwtPlot::yLeft);
}

// ---------------------------------------------------------------------------------------------- //
