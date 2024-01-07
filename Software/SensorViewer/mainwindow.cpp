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
#include "nonewidget.h"
#include "phwidget.h"
#include "temperaturewidget.h"

#include "ui_mainwindow.h"

#include <QDateTime>
#include <QMessageBox>

#include <bitset>

// ---------------------------------------------------------------------------------------------- //

using namespace std::chrono_literals;

// ---------------------------------------------------------------------------------------------- //

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_ui(std::make_unique<Ui::MainWindow>()),
      m_powerLabel(new QLabel),
      m_updateTimer(this)
{
    m_ui->setupUi(this);

    auto separator = m_ui->toolBar->insertSeparator(m_ui->actionConnect);

    m_serialPortWidget = new SerialPortWidget;
    m_ui->toolBar->insertWidget(separator, m_serialPortWidget);

    m_ui->toolBar->setMinimumWidth(m_ui->toolBar->sizeHint().width());

    m_ui->actionConnect->setEnabled(!m_serialPortWidget->selectedSerialPort().isEmpty());

    m_sensorGroups = {
        m_ui->sensor1Group, m_ui->sensor2Group
    };

    setupSensorWidgets();

    m_ui->statusBar->addPermanentWidget(m_powerLabel);

    //adjustSize();
    //setMinimumSize(sizeHint());

    m_updateTimer.setInterval(500ms);
    m_updateTimer.setSingleShot(true);

    connect(m_serialPortWidget, SIGNAL(serialPortChanged(QString)),
            this, SLOT(onSerialPortChanged(QString)));

    connect(m_ui->actionConnect, SIGNAL(triggered()), this, SLOT(openDevice()));
    connect(m_ui->actionDisconnect, SIGNAL(triggered()), this, SLOT(closeDevice()));
    connect(m_ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAboutDialog()));

    connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(updateStatus()));
}

// ---------------------------------------------------------------------------------------------- //

MainWindow::~MainWindow() = default;

// ---------------------------------------------------------------------------------------------- //

void MainWindow::onSerialPortChanged(const QString& port)
{
    m_ui->actionConnect->setEnabled(!port.isEmpty());
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::openDevice()
{
    try {
        m_device = std::make_unique<Device>(m_serialPortWidget->selectedSerialPort().toStdString());

        m_serialPortWidget->setEnabled(false);
        m_ui->actionConnect->setEnabled(false);
        m_ui->actionDisconnect->setEnabled(true);
        m_ui->centralwidget->setEnabled(true);

        setupSensorWidgets();

        updateInfo();
        updateStatus();
    }
    catch (const std::exception& e) {
        handleError(e.what());
    }
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::closeDevice()
{
    m_updateTimer.stop();
    m_device = nullptr;

    m_serialPortWidget->setEnabled(true);
    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->centralwidget->setEnabled(false);

    setupSensorWidgets();
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::showAboutDialog()
{
    AboutDialog dialog(this);
    dialog.exec();
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::updateStatus()
{
    if (!m_device)
        return;

    try {
        for (size_t i = 0; i < Device::SensorCount; ++i)
            m_sensorWidgets.at(i)->setValue(m_device->getSensorValue(i));

        const Device::PowerValues powerValues = m_device->getPowerValues();

        const QString baseString = "Device power: %1 V @ %2 mA | Device temperature: %3 °C";
        const QString powerString = baseString.arg(powerValues.voltage, 0, 'f', 2)
                                              .arg(powerValues.current * 1000.0, 0, 'f', 0)
                                              .arg(powerValues.temperature, 0, 'f', 1);
        m_powerLabel->setText(powerString);

        m_updateTimer.start();
    }
    catch (const std::exception& e) {
        handleError(e.what());
    }
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::setupSensorWidgets()
{
    for (size_t i = 0; i < Device::SensorCount; ++i)
    {
        QGroupBox* group = m_sensorGroups.at(i);
        auto children = group->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);

        for (auto child : children)
            delete child;

        QLayout* layout = group->layout();
        Q_ASSERT(layout != nullptr);

        SensorWidget* widget = nullptr;

        if (m_device)
        {
            Device::SensorType type = m_device->getSensorType(i);

            if (type == Device::SensorType::pH_ORP)
                widget = new PhWidget(group);
            else if (type == Device::SensorType::Temperature)
                widget = new TemperatureWidget(group);
        }

        if (!widget)
            widget = new NoneWidget(group);

        m_sensorWidgets.at(i) = widget;
        layout->addWidget(widget);
    }
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::updateInfo()
{
    if (!m_device)
        return;

    m_ui->hardwareVersion->setText(m_device->getHardwareVersion().c_str());
    m_ui->firmwareVersion->setText(m_device->getFirmwareVersion().c_str());
    m_ui->serialNumber->setText(m_device->getSerialNumber().c_str());

    const auto date = QDateTime::fromSecsSinceEpoch(m_device->getBuildTimestamp()).date();
    m_ui->buildDate->setText(QLocale::system().toString(date, QLocale::ShortFormat));
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::handleError(const QString& msg)
{
    closeDevice();
    QMessageBox::critical(this, "Error", msg);
}

// ---------------------------------------------------------------------------------------------- //
