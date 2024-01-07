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

#include "calibrationdialog.h"
#include "ui_calibrationdialog.h"

#include <QMessageBox>
#include <QPushButton>

// ---------------------------------------------------------------------------------------------- //

CalibrationDialog::CalibrationDialog(const QString& port, Device::Calibration* results,
                                     QWidget* parent)
    : QDialog(parent),
      m_ui(std::make_unique<Ui::CalibrationDialog>()),
      m_port(port),
      m_results(results)
{
    Q_ASSERT(results != nullptr);

    m_ui->setupUi(this);

    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    setWindowModality(Qt::ApplicationModal);
}

// ---------------------------------------------------------------------------------------------- //

CalibrationDialog::~CalibrationDialog() = default;

// ---------------------------------------------------------------------------------------------- //

auto CalibrationDialog::exec() -> int
{
    try {
        DeviceCalibrator calibrator(m_port.toLocal8Bit(), this);
        calibrator.start();

        return QDialog::exec();
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(this, "Error", QString("Unable to start calibration:\n") + e.what());
        return CalibrationDialog::Rejected;
    }
}

// ---------------------------------------------------------------------------------------------- //

void CalibrationDialog::onCalibrationProgress(int percent) noexcept
{
    QMetaObject::invokeMethod(this, "handleCalibrationProgress",
                              Qt::QueuedConnection, Q_ARG(int, percent));
}

// ---------------------------------------------------------------------------------------------- //

void CalibrationDialog::onCalibrationComplete(const Device::Calibration& results) noexcept
{
    QMetaObject::invokeMethod(this, "handleCalibrationComplete",
                              Qt::QueuedConnection, Q_ARG(Device::Calibration, results));
}

// ---------------------------------------------------------------------------------------------- //

void CalibrationDialog::onError(const std::string& msg) noexcept
{
    QMetaObject::invokeMethod(this, "handleError",
                              Qt::QueuedConnection, Q_ARG(QString, msg.c_str()));
}

// ---------------------------------------------------------------------------------------------- //

void CalibrationDialog::handleCalibrationProgress(int percent)
{
    m_ui->progressBar->setValue(percent);
}

// ---------------------------------------------------------------------------------------------- //

void CalibrationDialog::handleCalibrationComplete(const Device::Calibration& results)
{
    *m_results = results;
    accept();
}

// ---------------------------------------------------------------------------------------------- //

void CalibrationDialog::handleError(const QString& msg)
{
    QMessageBox::critical(this, "Error", "Calibration failed:\n" + msg);
    reject();
}

// ---------------------------------------------------------------------------------------------- //
