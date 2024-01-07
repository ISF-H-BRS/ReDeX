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

#include "aboutdialog.h"
#include "mainwindow.h"

#include "ui_mainwindow.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>

// ---------------------------------------------------------------------------------------------- //

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_ui(std::make_unique<Ui::MainWindow>())
{
    m_ui->setupUi(this);
    m_ui->toolBar->setMinimumWidth(m_ui->toolBar->sizeHint().width());

    connect(m_ui->actionConnect, SIGNAL(triggered()), this, SLOT(connectDevice()));
    connect(m_ui->actionDisconnect, SIGNAL(triggered()), this, SLOT(closeDevice()));
    connect(m_ui->actionStart, SIGNAL(triggered()), this, SLOT(startMeasurement()));
    connect(m_ui->actionStop, SIGNAL(triggered()), this, SLOT(stopMeasurement()));
    connect(m_ui->actionPowerMonitor, SIGNAL(toggled(bool)), this, SLOT(togglePowerWindow(bool)));
    connect(m_ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
}

// ---------------------------------------------------------------------------------------------- //

MainWindow::~MainWindow() = default;

// ---------------------------------------------------------------------------------------------- //

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (m_powerWindow)
        m_powerWindow->close();

    if (m_device)
        closeDevice();

    QMainWindow::closeEvent(event);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::connectDevice()
{
    Q_ASSERT(m_device == nullptr);

    const QString hostName = getHostName();

    if (hostName.isEmpty())
        return;

    try {
        m_device = std::make_unique<Device>(hostName);

        auto pdev = m_device.get();

        connect(pdev, SIGNAL(testpointInfoReceived(std::vector<redex::TestpointInfo>)),
                this, SLOT(onTestpointInfoReceived(std::vector<redex::TestpointInfo>)));

        connect(pdev, SIGNAL(nodeInfoReceived(std::vector<redex::NodeInfo>)),
                this, SLOT(onNodeInfoReceived(std::vector<redex::NodeInfo>)));

        connect(pdev, SIGNAL(statusChanged(redex::Status)),
                this, SLOT(onStatusChanged(redex::Status)));

        connect(pdev, SIGNAL(conductanceReceived(QString,double,double,double)),
                this, SLOT(onConductanceReceived(QString,double,double,double)));

        connect(pdev, SIGNAL(orpValueReceived(QString,double)),
                this, SLOT(onOrpValueReceived(QString,double)));

        connect(pdev, SIGNAL(phValueReceived(QString,double)),
                this, SLOT(onPhValueReceived(QString,double)));

        connect(pdev, SIGNAL(temperatureReceived(QString,double)),
                this, SLOT(onTemperatureReceived(QString,double)));

        connect(pdev, SIGNAL(voltammogramReceived(QString,Device::Voltammogram)),
                this, SLOT(onVoltammogramReceived(QString,Device::Voltammogram)));

        connect(pdev, SIGNAL(hubStatusReceived(double)),
                this, SLOT(onHubStatusReceived(double)));

        connect(pdev, SIGNAL(nodeStatusReceived(QString,double,double,double)),
                this, SLOT(onNodeStatusReceived(QString,double,double,double)));

        connect(pdev, SIGNAL(error(QString)), this, SLOT(onError(QString)));

        m_ui->actionConnect->setEnabled(false);
        m_ui->actionDisconnect->setEnabled(true);
        m_ui->actionStart->setEnabled(true);

        m_device->requestInfo();
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::closeDevice()
{
    Q_ASSERT(m_device != nullptr);

    m_powerWindow = nullptr;
    m_device = nullptr;

    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionStart->setEnabled(false);
    m_ui->actionStop->setEnabled(false);

    m_ui->actionPowerMonitor->setChecked(false);
    m_ui->actionPowerMonitor->setEnabled(false);

    m_ui->tabWidget->clear();

    m_testpointWidgets.clear();
    m_plotWindows.clear();
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::startMeasurement()
{
    Q_ASSERT(m_device != nullptr);

    try {
        m_device->startMeasurement();
    }
    catch (const std::exception& e) {
        onError(e.what());
    }
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::stopMeasurement()
{
    Q_ASSERT(m_device != nullptr);

    try {
        m_device->stopMeasurement();
    }
    catch (const std::exception& e) {
        onError(e.what());
    }
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::togglePowerWindow(bool visible)
{
    if (m_powerWindow)
        m_powerWindow->setVisible(visible);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::showAboutDialog()
{
    AboutDialog dialog;
    dialog.exec();
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::showVoltammogram(const QString& testpointId)
{
    auto window = getPlotWindow(testpointId);

    if (window)
        window->show();
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::onNodeInfoReceived(const std::vector<redex::NodeInfo>& infos)
{
    Q_ASSERT(m_device != nullptr);

    m_powerWindow = std::make_unique<PowerWindow>(infos);

    connect(m_powerWindow.get(), SIGNAL(visibilityChanged(bool)),
            m_ui->actionPowerMonitor, SLOT(setChecked(bool)));

    m_ui->actionPowerMonitor->setEnabled(true);

    m_device->startPowerMonitor();
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::onTestpointInfoReceived(const std::vector<redex::TestpointInfo>& infos)
{
    Q_ASSERT(m_device != nullptr);

    for (const auto& info : infos)
    {
        const QString id(info.testpointId.c_str());
        auto widget = new TestpointWidget(info);

        connect(widget, SIGNAL(voltammogramRequested(QString)),
                this, SLOT(showVoltammogram(QString)));

        m_ui->tabWidget->addTab(widget, id);
        m_testpointWidgets[id] = widget;

        auto window = std::make_unique<PlotWindow>();
        window->setWindowTitle(QString("Voltammogram (%1)").arg(id));

        m_plotWindows[id] = std::move(window);
    }
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::onStatusChanged(redex::Status status)
{
    const bool running = status == redex::Status::MeasurementStarted;

    m_ui->actionStart->setEnabled(!running);
    m_ui->actionStop->setEnabled(running);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::onConductanceReceived(const QString& testpointId,
                                       double voltage, double current, double admittance)
{
    auto widget = getTestpointWidget(testpointId);

    if (widget)
        widget->setAdmittance(voltage, current, admittance);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::onOrpValueReceived(const QString& testpointId, double value)
{
    auto widget = getTestpointWidget(testpointId);

    if (widget)
        widget->setOrpValue(value);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::onPhValueReceived(const QString& testpointId, double value)
{
    auto widget = getTestpointWidget(testpointId);

    if (widget)
        widget->setPhValue(value);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::onTemperatureReceived(const QString& testpointId, double value)
{
    auto widget = getTestpointWidget(testpointId);

    if (widget)
        widget->setTemperature(value);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::onVoltammogramReceived(const QString& testpointId,
                                        const Device::Voltammogram& voltammogram)
{
    auto widget = getTestpointWidget(testpointId);

    if (widget)
        widget->setVoltammetryValues(voltammogram.voltage.size());

    auto window = getPlotWindow(testpointId);

    if (window)
        window->setSamples(voltammogram.voltage, voltammogram.current);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::onHubStatusReceived(double temperature)
{
    if (m_powerWindow)
        m_powerWindow->setHubStatus(temperature);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::onNodeStatusReceived(const QString& nodeId,
                                      double voltage, double current, double temperature)
{
    if (m_powerWindow)
        m_powerWindow->setNodeStatus(nodeId, voltage, current, temperature);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::onError(const QString& msg)
{
    closeDevice();
    QMessageBox::critical(this, "Error", msg);
}

// ---------------------------------------------------------------------------------------------- //

auto MainWindow::getTestpointWidget(const QString& id) -> TestpointWidget*
{
    auto it = m_testpointWidgets.find(id);

    if (it == m_testpointWidgets.end())
    {
        onError("Unknown testpoint ID received.");
        return nullptr;
    }

    return it->second;
}

// ---------------------------------------------------------------------------------------------- //

auto MainWindow::getPlotWindow(const QString& id) -> PlotWindow*
{
    auto it = m_plotWindows.find(id);

    if (it == m_plotWindows.end())
    {
        onError("Unknown testpoint ID received.");
        return nullptr;
    }

    return it->second.get();
}

// ---------------------------------------------------------------------------------------------- //

auto MainWindow::getHostName() const -> QString
{
    QSettings settings;
    QString hostName = settings.value("lastHostName", "redex-hub.local").toString();

    bool ok = false;
    hostName = QInputDialog::getText(nullptr, "Connect to Host",
                                     "Host address:", QLineEdit::Normal, hostName, &ok);
    if (ok && !hostName.isEmpty())
    {
        settings.setValue("lastHostName", hostName);
        return hostName;
    }

    return {};
}

// ---------------------------------------------------------------------------------------------- //
