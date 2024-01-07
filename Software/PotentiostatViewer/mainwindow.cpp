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
#include "calibrationdialog.h"
#include "mainwindow.h"
#include "settings.h"

#include "ui_mainwindow.h"

#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QPen>
#include <QSettings>
#include <QTextStream>

#include <cmath>
#include <iostream>

// ---------------------------------------------------------------------------------------------- //

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_ui(std::make_unique<Ui::MainWindow>()),
      m_fftWindow(std::make_unique<FftWindow>()),
      m_powerLabel(new QLabel)
{
    m_ui->setupUi(this);

    auto separator = m_ui->toolBar->insertSeparator(m_ui->actionConnect);

    m_serialPortWidget = new SerialPortWidget;
    m_ui->toolBar->insertWidget(separator, m_serialPortWidget);

    m_ui->toolBar->setMinimumWidth(m_ui->toolBar->sizeHint().width());

    m_ui->actionConnect->setEnabled(!m_serialPortWidget->selectedSerialPort().isEmpty());

    m_ui->voltammogramPlot->setAxisTitles("Voltage (V)", "Current (A)");
    m_ui->voltagePlot->setAxisTitles("Time (s)", "Voltage (V)");
    m_ui->currentPlot->setAxisTitles("Time (s)", "Current (A)");

    toggleFilter(m_ui->actionFilter->isChecked());

    m_ui->statusBar->addPermanentWidget(m_powerLabel);

    m_statusTimer.setInterval(1s);
    m_statusTimer.setSingleShot(false);

    loadSettings();

    connect(m_serialPortWidget, SIGNAL(serialPortChanged(QString)),
            this, SLOT(onSerialPortChanged(QString)));

    connect(m_fftWindow.get(), SIGNAL(visiblityChanged(bool)),
            m_ui->actionShowFft, SLOT(setChecked(bool)));

    connect(m_ui->actionConnect, SIGNAL(triggered()), this, SLOT(openDevice()));
    connect(m_ui->actionDisconnect, SIGNAL(triggered()), this, SLOT(closeDevice()));
    connect(m_ui->actionCalibrate, SIGNAL(triggered()), this, SLOT(calibrate()));
    connect(m_ui->actionLoadCalibration, SIGNAL(triggered()), this, SLOT(loadCalibration()));
    connect(m_ui->actionFilter, SIGNAL(toggled(bool)), this, SLOT(toggleFilter(bool)));
    connect(m_ui->actionShowFft, SIGNAL(toggled(bool)), this, SLOT(toggleFftWindow(bool)));
    connect(m_ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAboutDialog()));

    connect(m_ui->runMeasurementButton, SIGNAL(clicked()), this, SLOT(startMeasurement()));
    connect(m_ui->abortMeasurementButton, SIGNAL(clicked()), this, SLOT(stopMeasurement()));

    connect(m_ui->storageWidget, SIGNAL(autoSaveToggled(bool)),
            m_ui->saveDataButton, SLOT(setDisabled(bool)));
    connect(m_ui->saveDataButton, SIGNAL(clicked()), this, SLOT(saveData()));

    connect(&m_statusTimer, SIGNAL(timeout()), this, SLOT(requestPowerValues()));

    connect(&m_deviceListener, SIGNAL(measurementStarted()), this, SLOT(onMeasurementStarted()));
    connect(&m_deviceListener, SIGNAL(measurementStopped()), this, SLOT(onMeasurementStopped()));
    connect(&m_deviceListener, SIGNAL(measurementComplete()), this, SLOT(onMeasurementComplete()));
    connect(&m_deviceListener, SIGNAL(currentRangeChanged(Device::CurrentRange)),
            this, SLOT(onCurrentRangeChanged(Device::CurrentRange)));
    connect(&m_deviceListener, SIGNAL(samplesReceived(DeviceListener::SampleBuffer)),
            this, SLOT(onSamplesReceived(DeviceListener::SampleBuffer)));
    connect(&m_deviceListener, SIGNAL(powerValuesReceived(Device::PowerValues)),
            this, SLOT(onPowerValuesReceived(Device::PowerValues)));
    connect(&m_deviceListener, SIGNAL(error(QString)), this, SLOT(onError(QString)));
}

// ---------------------------------------------------------------------------------------------- //

MainWindow::~MainWindow()
{
    saveSettings();
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::closeEvent(QCloseEvent* event)
{
    m_fftWindow = nullptr;
    QMainWindow::closeEvent(event);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::onSerialPortChanged(const QString& port)
{
    m_ui->actionConnect->setEnabled(!port.isEmpty());
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::openDevice()
{
    try {
        const auto port = m_serialPortWidget->selectedSerialPort().toStdString();

        m_device = std::make_unique<Device>(port);
        m_device->addListener(&m_deviceListener);

        m_device->setCalibration(m_calibration);
        m_device->requestPowerValues();

        m_serialPortWidget->setEnabled(false);
        m_ui->actionConnect->setEnabled(false);
        m_ui->actionDisconnect->setEnabled(true);
        m_ui->actionCalibrate->setEnabled(true);
        m_ui->runMeasurementButton->setEnabled(true);
        m_ui->abortMeasurementButton->setEnabled(false);
        m_ui->centralwidget->setEnabled(true);

        m_statusTimer.start();
    }
    catch (const std::exception& e) {
        handleError(e.what());
    }
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::closeDevice()
{
    m_statusTimer.stop();

    m_device = nullptr;

    m_serialPortWidget->setEnabled(true);
    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionCalibrate->setEnabled(false);
    m_ui->runMeasurementButton->setEnabled(false);
    m_ui->abortMeasurementButton->setEnabled(false);
    m_ui->centralwidget->setEnabled(false);

    m_powerLabel->clear();
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::startMeasurement()
{
    if (!m_device)
        return;

    try
    {
        m_voltageFilter.reset();
        m_currentFilter.reset();

        m_ui->voltammogramPlot->clear();
        m_ui->voltagePlot->clear();
        m_ui->currentPlot->clear();

        m_fftWindow->clear();

        Device::Setup setup = m_ui->setupWidget->getSetup();

        if (m_ui->setupWidget->getAutoRangeEnabled())
        {
            setup.currentRange = Device::CurrentRange::_200nA;
            m_ui->setupWidget->setCurrentRange(setup.currentRange);
        }

        m_device->startMeasurement(setup);
    }
    catch (const std::exception& e) {
        handleError(e.what());
    }
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::stopMeasurement()
{
    if (!m_device)
        return;

    try {
        m_device->stopMeasurement();
    }
    catch (const std::exception& e) {
        handleError(e.what());
    }
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::requestPowerValues()
{
    if (!m_device)
        return;

    try {
        m_device->requestPowerValues();
    }
    catch (const std::exception& e) {
        handleError(e.what());
    }
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::calibrate()
{
    if (!m_device)
        return;

    auto button =
            QMessageBox::information(this, "Calibration",
                                     "Please connect the electrodes to the calibration "
                                     "rig and press OK to begin the calibration process.",
                                     QMessageBox::Ok | QMessageBox::Cancel);

    if (button == QMessageBox::Cancel)
        return;

    closeDevice();

    CalibrationDialog dialog(m_serialPortWidget->selectedSerialPort(), &m_calibration, this);

    const int result = dialog.exec();

    if (result == CalibrationDialog::Accepted)
    {
        button = QMessageBox::information(this, "Calibration", "Calibration complete.",
                                          QMessageBox::Save | QMessageBox::Ok);

        if (button == QMessageBox::Save)
            saveCalibration();
    }

    openDevice();
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::saveCalibration()
{
    const QString filename = QFileDialog::getSaveFileName(this, "Save Calibration",
                                                          QString(), "Calibration Files (*.ini)");
    if (filename.isEmpty())
        return;

    QSettings settings(filename, QSettings::IniFormat);

    settings.beginGroup("Calibration");
    settings.setValue("VoltageOffset", m_calibration.voltageOffset);
    settings.setValue("CurrentOffset", m_calibration.currentOffset);
    settings.setValue("SignalOffset", m_calibration.signalOffset);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::loadCalibration()
{
    const QString filename = QFileDialog::getOpenFileName(this, "Open Calibration",
                                                          QString(), "Calibration Files (*.ini)");
    if (filename.isEmpty())
        return;

    QSettings settings(filename, QSettings::IniFormat);

    settings.beginGroup("Calibration");

    const bool valid = settings.contains("VoltageOffset") &&
                       settings.contains("CurrentOffset") &&
                       settings.contains("SignalOffset");

    if (!valid)
    {
        QMessageBox::warning(this, "Error", "Calibration file appears to be invalid "
                                            "or incomplete. No data has been loaded.");
    }

    m_calibration.voltageOffset = settings.value("VoltageOffset").toInt();
    m_calibration.currentOffset = settings.value("CurrentOffset").toInt();
    m_calibration.signalOffset = settings.value("SignalOffset").toInt();

    if (m_device)
        applyCalibration();
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::applyCalibration()
{
    Q_ASSERT(m_device != nullptr);

    try {
        m_device->setCalibration(m_calibration);
    }
    catch (const std::exception& e) {
        handleError(e.what());
    }
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::toggleFilter(bool enable)
{
    m_ui->voltammogramPlot->setFiltered(enable);
    m_ui->voltagePlot->setFiltered(enable);
    m_ui->currentPlot->setFiltered(enable);

    m_fftWindow->setFiltered(enable);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::toggleFftWindow(bool show)
{
    if (show)
    {
        m_fftWindow->show();
        m_fftWindow->raise();
    }
    else
        m_fftWindow->hide();
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::showAboutDialog()
{
    AboutDialog dialog(this);
    dialog.exec();
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::saveData()
{
    const std::vector<double> times = m_ui->voltagePlot->getXData();
    const std::vector<double> voltages = m_ui->voltagePlot->getYData();
    const std::vector<double> currents = m_ui->currentPlot->getYData();
    const std::vector<double> filteredVoltages = m_ui->voltagePlot->getFilteredYData();
    const std::vector<double> filteredCurrents = m_ui->currentPlot->getFilteredYData();

    const size_t size = times.size();

    if (size == 0)
    {
        QMessageBox::warning(this, "Warning", "There is no data available for saving.");
        return;
    }

    Q_ASSERT(voltages.size() == size && currents.size() == size &&
             filteredVoltages.size() == size && filteredCurrents.size() == size);

    const QString filename = m_ui->storageWidget->getFilePath();
    Q_ASSERT(!QFile::exists(filename));

    QFile file(filename);

    if (!file.open(QFile::WriteOnly))
    {
        QMessageBox::critical(this, "Error", "Unable to open file " + filename + " for writing.");
        return;
    }

    QTextStream out(&file);
    out << "Time (s);Voltage (V);Current (A);Filtered voltage (V);Filtered current (A)\n";

    for (size_t i = 0; i < size; ++i)
    {
        out << times[i] << ";"
            << voltages[i] << ";"
            << currents[i] << ";"
            << filteredVoltages[i] << ";"
            << filteredCurrents[i] << "\n";
    }

    m_ui->saveDataButton->setEnabled(false);
    m_ui->statusBar->showMessage("Data saved to " + filename + ".", 3000);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::onMeasurementStarted()
{
    m_ui->actionLoadCalibration->setEnabled(false);
    m_ui->setupWidget->setEnabled(false);
    m_ui->runMeasurementButton->setEnabled(false);
    m_ui->abortMeasurementButton->setEnabled(true);
    m_ui->storageWidget->setEnabled(false);
    m_ui->saveDataButton->setEnabled(false);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::onMeasurementStopped()
{
    m_filteredVoltages.clear();
    m_filteredCurrents.clear();

    m_voltageFilter.finalize(&m_filteredVoltages);
    m_currentFilter.finalize(&m_filteredCurrents);

    updateFilteredPlotData(m_filteredVoltages, m_filteredCurrents);

    m_ui->actionLoadCalibration->setEnabled(true);
    m_ui->setupWidget->setEnabled(true);
    m_ui->runMeasurementButton->setEnabled(true);
    m_ui->abortMeasurementButton->setEnabled(false);
    m_ui->storageWidget->setEnabled(true);
    m_ui->saveDataButton->setEnabled(true);

    if (m_ui->storageWidget->autoSaveEnabled())
        saveData();
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::onMeasurementComplete()
{
    onMeasurementStopped();
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::onCurrentRangeChanged(Device::CurrentRange range)
{
    m_ui->setupWidget->setCurrentRange(range);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::onSamplesReceived(const DeviceListener::SampleBuffer& samples)
{
    m_filteredVoltages.clear();
    m_filteredCurrents.clear();

    m_voltageFilter.update(samples.voltages, &m_filteredVoltages);
    m_currentFilter.update(samples.currents, &m_filteredCurrents);

    updatePlotData(samples.voltages, samples.currents);
    updateFilteredPlotData(m_filteredVoltages, m_filteredCurrents);

    m_fftWindow->addSamples(samples.voltages, samples.currents);
    m_fftWindow->addFilteredSamples(m_filteredVoltages, m_filteredCurrents);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::onPowerValuesReceived(const Device::PowerValues& values)
{
    const QString baseString = "Device power: %1 V @ %2 mA | Device temperature: %3 °C";
    const QString powerString = baseString.arg(values.voltage, 0, 'f', 2)
                                          .arg(values.current * 1000.0, 0, 'f', 0)
                                          .arg(values.temperature, 0, 'f', 1);
    m_powerLabel->setText(powerString);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::onError(const QString& msg)
{
    closeDevice();
    QMessageBox::critical(this, "Error", msg);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::updatePlotData(std::span<const double> voltages, std::span<const double> currents)
{
    Q_ASSERT(voltages.size() == currents.size());

    m_ui->voltammogramPlot->addSamples(voltages, currents);

    double time = m_ui->voltagePlot->size() * Device::SampleTime;
    const auto generate = [&time]{ double t = time; time += Device::SampleTime; return t; };

    std::vector<double> times(voltages.size());
    std::generate(times.begin(), times.end(), generate);

    m_ui->voltagePlot->addSamples(times, voltages);
    m_ui->currentPlot->addSamples(times, currents);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::updateFilteredPlotData(std::span<const double> voltages,
                                        std::span<const double> currents)
{
    Q_ASSERT(voltages.size() == currents.size());

    m_ui->voltammogramPlot->addFilteredSamples(voltages, currents);

    double time = m_ui->voltagePlot->filteredSize() * Device::SampleTime;
    const auto generate = [&time]{ double t = time; time += Device::SampleTime; return t; };

    std::vector<double> times(voltages.size());
    std::generate(times.begin(), times.end(), generate);

    m_ui->voltagePlot->addFilteredSamples(times, voltages);
    m_ui->currentPlot->addFilteredSamples(times, currents);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());

    m_ui->setupWidget->saveSettings();
    m_ui->storageWidget->saveSettings();

    Settings::getInstance()->saveToDisk();
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::loadSettings()
{
    QSettings settings;
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray());

    Settings::getInstance()->loadFromDisk();

    m_ui->setupWidget->loadSettings();
    m_ui->storageWidget->loadSettings();
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::handleError(const QString& msg)
{
    closeDevice();
    QMessageBox::critical(this, "Error", msg);
}

// ---------------------------------------------------------------------------------------------- //
