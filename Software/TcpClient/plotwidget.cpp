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

    m_curve = new QwtPlotCurve;
    m_curve->setPen(QPen(CurveColor, CurveWidth, Qt::SolidLine));
    //m_curve->setRenderHint(QwtPlotCurve::RenderAntialiased);
    //m_curve->setCurveAttribute(QwtPlotCurve::Fitted);
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

void PlotWidget::setSamples(std::span<const double> xData, std::span<const double> yData)
{
    Q_ASSERT(xData.size() == yData.size());

    m_curve->setSamples(xData.data(), yData.data(), xData.size());
    replot();
}

// ---------------------------------------------------------------------------------------------- //

void PlotWidget::clear()
{
    m_curve->setData(nullptr);
    replot();
}

// ---------------------------------------------------------------------------------------------- //
