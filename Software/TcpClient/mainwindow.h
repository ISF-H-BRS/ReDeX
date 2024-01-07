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
#include "plotwindow.h"
#include "powerwindow.h"
#include "testpointwidget.h"

#include <QMainWindow>

#include <memory>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void connectDevice();
    void closeDevice();

    void startMeasurement();
    void stopMeasurement();

    void togglePowerWindow(bool visible);

    void showAboutDialog();

    void showVoltammogram(const QString& testpointId);

    void onNodeInfoReceived(const std::vector<redex::NodeInfo>& info);
    void onTestpointInfoReceived(const std::vector<redex::TestpointInfo>& info);

    void onStatusChanged(redex::Status status);

    void onConductanceReceived(const QString& testpointId,
                               double voltage, double current, double admittance);

    void onOrpValueReceived(const QString& testpointId, double value);

    void onPhValueReceived(const QString& testpointId, double value);

    void onTemperatureReceived(const QString& testpointId, double value);

    void onVoltammogramReceived(const QString& testpointId,
                                const Device::Voltammogram& voltammogram);

    void onHubStatusReceived(double temperature);

    void onNodeStatusReceived(const QString& nodeId,
                              double voltage, double current, double temperature);

    void onError(const QString& msg);

private:
    void closeEvent(QCloseEvent* event) override;

    auto getTestpointWidget(const QString& id) -> TestpointWidget*;
    auto getPlotWindow(const QString& id) -> PlotWindow*;

    auto getHostName() const -> QString;

private:
    std::unique_ptr<Ui::MainWindow> m_ui;
    std::unique_ptr<Device> m_device;

    std::unique_ptr<PowerWindow> m_powerWindow;

    std::map<QString, TestpointWidget*> m_testpointWidgets;
    std::map<QString, std::unique_ptr<PlotWindow>> m_plotWindows;
};
