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

#include "analysiswidget.h"
#include "devicewrapper.h"
#include "plotwidget.h"
#include "setupwidget.h"

#include <QGroupBox>
#include <QLabel>
#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void openDevice();
    void closeDevice();

    void startCapture();
    void stopCapture();

    void zoomIn();
    void zoomOut();
    void resetZoom();

    void toggleAutoScale(bool enable);

    void showAboutDialog();

    void scrollPlots(int value);
    void scrollPlotsRelative(int amount);

    void onDataUpdated();
    void onError(const QString& message);

    void onAnalysisComplete(Device::Input input,
                            double voltage, double current, double admittance);

    void onPowerUpdated(const Device::PowerValues& values);

    void onSignalChanged(SetupWidget* widget);
    void onGainChanged(SetupWidget* widget);
    void onOffsetChanged(SetupWidget* widget);

private:
    void initializeArrays();
    auto plot(Device::Input input, Device::Channel channel) -> PlotWidget*;

    void setUiConnected(bool connected);

    void saveSettings();
    void restoreSettings();

    void updateZoomLevels();

    void updateGain(size_t input, double currentMagnitude);

    void sendConfiguration(Device::Input input);

private:
    std::unique_ptr<Ui::MainWindow> m_ui;

    QLabel m_powerLabel;

    std::unique_ptr<DeviceWrapper> m_device;

    std::array<SetupWidget*, Device::InputCount> m_setupWidgets = {};

    std::array<QGroupBox*, Device::InputCount> m_analysisGroups = {};
    std::array<AnalysisWidget*, Device::InputCount> m_analysisWidgets = {};

    std::array<PlotWidget*, Device::InputCount * Device::ChannelCount> m_plots = {};

    std::array<DataBuffer, Device::InputCount> m_dataBuffers = {
        Device::Input::One, Device::Input::Two
    };

    size_t m_zoomLevel = 0;
};
