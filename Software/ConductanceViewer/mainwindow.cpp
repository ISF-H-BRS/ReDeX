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
#include "deviceselectiondialog.h"
#include "mainwindow.h"

#include "ui_mainwindow.h"

#include <QAction>
#include <QMessageBox>
#include <QScrollBar>
#include <QSettings>

#include <cmath>
#include <limits>

// ---------------------------------------------------------------------------------------------- //

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_ui(std::make_unique<Ui::MainWindow>())
{
    m_ui->setupUi(this);

    m_ui->toolBar->setMinimumWidth(m_ui->toolBar->sizeHint().width());
    m_ui->statusBar->addPermanentWidget(&m_powerLabel);

    initializeArrays();
    restoreSettings();

    for (size_t i = 0; i < Device::InputCount; ++i)
    {
        const auto input = Device::toInput(i);

        auto voltagePlot = plot(input, Device::Channel::Voltage);
        auto currentPlot = plot(input, Device::Channel::Current);

        voltagePlot->setYAxisText("Voltage (V)");
        currentPlot->setYAxisText("Current ⋅ Gain (V)");

        auto& dataBuffer = m_dataBuffers.at(i);

        voltagePlot->setData(dataBuffer, Device::Channel::Voltage);
        currentPlot->setData(dataBuffer, Device::Channel::Current);

        auto setupWidget = m_setupWidgets.at(i);

        connect(setupWidget, SIGNAL(signalChanged(SetupWidget*)),
                this, SLOT(onSignalChanged(SetupWidget*)));

        connect(setupWidget, SIGNAL(gainChanged(SetupWidget*)),
                this, SLOT(onGainChanged(SetupWidget*)));

        connect(setupWidget, SIGNAL(offsetChanged(SetupWidget*)),
                this, SLOT(onOffsetChanged(SetupWidget*)));
    }

    for (auto plot : m_plots)
        connect(plot, SIGNAL(scrollRequested(int)), this, SLOT(scrollPlotsRelative(int)));

    connect(m_ui->tabWidget, SIGNAL(currentChanged(int)),
            m_ui->setupWidgetStack, SLOT(setCurrentIndex(int)));

    connect(m_ui->actionConnect, SIGNAL(triggered()), this, SLOT(openDevice()));
    connect(m_ui->actionDisconnect, SIGNAL(triggered()), this, SLOT(closeDevice()));
    connect(m_ui->actionResumeCapture, SIGNAL(triggered()), this, SLOT(startCapture()));
    connect(m_ui->actionPauseCapture, SIGNAL(triggered()), this, SLOT(stopCapture()));
    connect(m_ui->actionZoomIn, SIGNAL(triggered()), this, SLOT(zoomIn()));
    connect(m_ui->actionZoomOut, SIGNAL(triggered()), this, SLOT(zoomOut()));
    connect(m_ui->actionResetZoom, SIGNAL(triggered()), this, SLOT(resetZoom()));
    connect(m_ui->actionAutoScale, SIGNAL(toggled(bool)), this, SLOT(toggleAutoScale(bool)));
    connect(m_ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAboutDialog()));

    connect(m_ui->plotScrollBar, SIGNAL(valueChanged(int)), this, SLOT(scrollPlots(int)));
}

// ---------------------------------------------------------------------------------------------- //

MainWindow::~MainWindow()
{
    saveSettings();
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::openDevice()
{
    Q_ASSERT(m_device == nullptr);

    const std::vector<DeviceInfo> devices = DeviceInfo::getAvailableDevices();

    if (devices.empty())
    {
        QMessageBox::critical(this, "Error", "No devices found.");
        return;
    }

    ssize_t selectedIndex = 0;

    if (devices.size() > 1)
    {
        DeviceSelectionDialog dialog(devices, this);

        int result = dialog.exec();

        if (result != DeviceSelectionDialog::Accepted)
            return;

        selectedIndex = dialog.selectedIndex();
    }

    Q_ASSERT(selectedIndex >= 0);

    const DeviceInfo& info = devices.at(static_cast<size_t>(selectedIndex));

    try {
        const std::array<DataBuffer*, Device::InputCount> buffers = {
            &m_dataBuffers.at(0), &m_dataBuffers.at(1)
        };

        m_device = std::make_unique<DeviceWrapper>(info, buffers);
        auto ptr = m_device.get();

        connect(ptr, SIGNAL(dataUpdated()), this, SLOT(onDataUpdated()));
        connect(ptr, SIGNAL(error(QString)), this, SLOT(onError(QString)));

        connect(ptr,  SIGNAL(analysisComplete(Device::Input,double,double,double)),
                this, SLOT(onAnalysisComplete(Device::Input,double,double,double)));

        connect(ptr,  SIGNAL(powerUpdated(Device::PowerValues)),
                this, SLOT(onPowerUpdated(Device::PowerValues)));

        setUiConnected(true);
        resetZoom();

        for (size_t i = 0; i < Device::InputCount; ++i)
        {
            const Device::Input input = Device::toInput(i);
            const bool connected = m_device->inputConnected(input);

            m_ui->tabWidget->setTabEnabled(static_cast<int>(i), connected);
            m_analysisGroups.at(i)->setEnabled(connected);

            if (connected)
                sendConfiguration(input);
        }

        startCapture();
    }
    catch (const std::exception& e) {
        onError(e.what());
    }
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::closeDevice()
{
    if (!m_device)
        return;

    m_device = nullptr;

    setUiConnected(false);

    for (auto widget : m_analysisWidgets)
        widget->clear();

    for (auto& buffer : m_dataBuffers)
        buffer.clear();

    for (auto plot : m_plots)
        plot->replot();

    m_powerLabel.clear();
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::startCapture()
{
    Q_ASSERT(m_device != nullptr);

    try {
        m_device->startCapture();

        m_ui->actionResumeCapture->setEnabled(false);
        m_ui->actionPauseCapture->setEnabled(true);
    }
    catch (const std::exception& e) {
        onError(e.what());
    }
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::stopCapture()
{
    Q_ASSERT(m_device != nullptr);

    try {
        m_device->stopCapture();

        m_ui->actionResumeCapture->setEnabled(true);
        m_ui->actionPauseCapture->setEnabled(false);
    }
    catch (const std::exception& e) {
        onError(e.what());
    }
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::zoomIn()
{
    if (PlotWidget::toZoomLevel(m_zoomLevel) == PlotWidget::MaximumZoomLevel)
        return;

    ++m_zoomLevel;
    updateZoomLevels();
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::zoomOut()
{
    if (PlotWidget::toZoomLevel(m_zoomLevel) == PlotWidget::MinimumZoomLevel)
        return;

    --m_zoomLevel;
    updateZoomLevels();
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::resetZoom()
{
    m_zoomLevel = PlotWidget::indexOf(PlotWidget::DefaultZoomLevel);
    updateZoomLevels();
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::updateZoomLevels()
{
    const auto level = PlotWidget::toZoomLevel(m_zoomLevel);

    const bool isMinimum = level == PlotWidget::MinimumZoomLevel;
    const bool isMaximum = level == PlotWidget::MaximumZoomLevel;
    const bool isDefault = level == PlotWidget::DefaultZoomLevel;

    for (auto plot : m_plots)
        plot->setZoomLevel(level);

    const double zoomWindow = PlotWidget::toSeconds(level);
    const auto pageStep = static_cast<int>(zoomWindow * 1000.0);

    m_ui->plotScrollBar->setPageStep(pageStep);
    m_ui->plotScrollBar->setMaximum(1000 - pageStep);

    m_ui->actionZoomIn->setEnabled(!isMaximum);
    m_ui->actionZoomOut->setEnabled(!isMinimum);
    m_ui->actionResetZoom->setEnabled(!isDefault);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::toggleAutoScale(bool enable)
{
    for (auto plot : m_plots)
        plot->setAutoScale(enable);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::showAboutDialog()
{
    AboutDialog dialog;
    dialog.exec();
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::scrollPlots(int value)
{
    const double seconds = static_cast<double>(value) * 0.001;

    for (auto plot : m_plots)
        plot->scrollTo(seconds);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::scrollPlotsRelative(int amount)
{
    const double diff = std::round(amount * m_ui->plotScrollBar->pageStep() * 0.001);
    m_ui->plotScrollBar->setValue(m_ui->plotScrollBar->value() + static_cast<int>(diff));
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::onDataUpdated()
{
    for (auto plot : m_plots)
        plot->replot();
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::onError(const QString& message)
{
    closeDevice();
    QMessageBox::critical(this, "Error", message);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::onAnalysisComplete(Device::Input input,
                                    double voltage, double current, double admittance)
{
    const auto inputIndex = Device::indexOf(input);

    SetupWidget* setup = m_setupWidgets.at(inputIndex);

    if (setup->autoGainEnabled())
        updateGain(inputIndex, current * Device::toDouble(setup->gain()));

    m_analysisWidgets.at(inputIndex)->setData(voltage, current, admittance);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::onPowerUpdated(const Device::PowerValues& values)
{
    const QString baseString = "Device power: %1 V @ %2 mA | Device temperature: %3 °C";
    const QString powerString = baseString.arg(values.voltage, 0, 'f', 2)
                                          .arg(values.current * 1000.0, 0, 'f', 0)
                                          .arg(values.temperature, 0, 'f', 1);
    m_powerLabel.setText(powerString);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::onSignalChanged(SetupWidget* widget)
{
    m_device->setupSignal(widget->input(),
                          widget->waveform(),
                          widget->frequency(),
                          widget->amplitude());
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::onGainChanged(SetupWidget* widget)
{
    m_device->setGain(widget->input(), widget->gain());
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::onOffsetChanged(SetupWidget* widget)
{
    m_device->setLeadResistance(widget->input(), widget->offset());
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::initializeArrays()
{
    // Setup widgets
    m_ui->setupWidget1->setInput(Device::Input::One);
    m_ui->setupWidget2->setInput(Device::Input::Two);

    m_setupWidgets = {
        m_ui->setupWidget1, m_ui->setupWidget2
    };

    // Analysis groups
    m_analysisGroups = {
        m_ui->analysisGroup1, m_ui->analysisGroup2
    };

    m_analysisWidgets = {
        m_ui->analysisWidget1, m_ui->analysisWidget2
    };

    // Plots
    m_plots = {
        m_ui->sourcePlot1, m_ui->sinkPlot1, m_ui->sourcePlot2, m_ui->sinkPlot2
    };
}

// ---------------------------------------------------------------------------------------------- //

auto MainWindow::plot(Device::Input input, Device::Channel channel) -> PlotWidget*
{
    return m_plots.at(Device::indexOf(input) * Device::ChannelCount + Device::indexOf(channel));
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::setUiConnected(bool connected)
{
    m_ui->centralWidget->setEnabled(connected);
    m_ui->toolWidget->setEnabled(connected);

    m_ui->actionConnect->setEnabled(!connected);
    m_ui->actionDisconnect->setEnabled(connected);
    m_ui->actionResumeCapture->setEnabled(connected);
    m_ui->actionPauseCapture->setEnabled(false);
    m_ui->actionZoomIn->setEnabled(connected);
    m_ui->actionZoomOut->setEnabled(false);
    m_ui->actionResetZoom->setEnabled(false);
    m_ui->actionAutoScale->setEnabled(connected);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.setValue("mainGeometry", saveGeometry());

    for (auto setup : m_setupWidgets)
        setup->saveSettings();
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::restoreSettings()
{
    QSettings settings;
    restoreGeometry(settings.value("mainGeometry").toByteArray());

    for (auto setup : m_setupWidgets)
        setup->restoreSettings();
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::updateGain(size_t input, double currentMagnitude)
{
    static constexpr double UpperThreshold = 0.95 * Device::MaximumAmplitude;
    static constexpr double LowerThreshold = 0.09 * Device::MaximumAmplitude;

    SetupWidget* widget = m_setupWidgets.at(input);
    Q_ASSERT(widget->autoGainEnabled());

    const auto gain = Device::indexOf(widget->gain());

    if (currentMagnitude > UpperThreshold)
    {
        if (gain > 0)
            widget->setGain(Device::toGain(gain - 1));
    }
    else if (currentMagnitude < LowerThreshold)
    {
        if (gain < Device::GainCount - 1)
            widget->setGain(Device::toGain(gain + 1));
    }
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::sendConfiguration(Device::Input input)
{
    SetupWidget* setup = m_setupWidgets.at(Device::indexOf(input));

    m_device->setupSignal(input, setup->waveform(), setup->frequency(), setup->amplitude());
    m_device->setGain(input, setup->gain());
}

// ---------------------------------------------------------------------------------------------- //
