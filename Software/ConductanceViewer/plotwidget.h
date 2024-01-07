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

class PlotWidget : public QWidget
{
    Q_OBJECT

public:
    enum class ZoomLevel
    {
        _1000ms = 0,
        _500ms,
        _200ms,
        _100ms,
        _50ms,
        _20ms,
        _10ms,
        _5ms
    };

    static constexpr size_t ZoomLevelCount = 8;

    static constexpr auto MinimumZoomLevel = ZoomLevel::_1000ms;
    static constexpr auto MaximumZoomLevel = ZoomLevel::_5ms;
    static constexpr auto DefaultZoomLevel = ZoomLevel::_1000ms;

    template <typename T = size_t>
    static constexpr auto indexOf(ZoomLevel level) { return static_cast<T>(level); }

    template <typename T>
    static constexpr auto toZoomLevel(T index) { return static_cast<ZoomLevel>(index); }

public:
    PlotWidget(QWidget* parent = nullptr);

    void setData(const DataBuffer& buffer, Device::Channel channel);

    void setYAxisText(const QString& text);

    void setZoomLevel(ZoomLevel level);
    auto zoomLevel() const -> ZoomLevel;

    void zoomIn();
    void zoomOut();

    void setAutoScale(bool enable);

    void scrollTo(double seconds);

    void replot();

    static auto toSeconds(ZoomLevel level) -> double;

signals:
    void scrollRequested(int amount);

private:
    void wheelEvent(QWheelEvent* event) override;

    void setAxisText(QwtAxisId axis, const QString& text);

    void updateAxisZoom();
    void updateAxisText();

private:
    QwtPlot* m_plot = nullptr;
    QwtPlotCurve* m_curve = nullptr;
    ZoomLevel m_zoomLevel = DefaultZoomLevel;
};
