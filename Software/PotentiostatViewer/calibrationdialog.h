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

#include <QDialog>

namespace Ui {
    class CalibrationDialog;
}

class CalibrationDialog : public QDialog, public DeviceCalibrator::Listener
{
    Q_OBJECT

public:
    CalibrationDialog(const QString& port, Device::Calibration* results,
                      QWidget* parent = nullptr);
    ~CalibrationDialog();

    auto exec() -> int override;

private:
    void onCalibrationProgress(int percent) noexcept override;
    void onCalibrationComplete(const Device::Calibration& results) noexcept override;
    void onError(const std::string& msg) noexcept override;

private slots:
    void handleCalibrationProgress(int percent);
    void handleCalibrationComplete(const Device::Calibration& results);
    void handleError(const QString& msg);

private:
    std::unique_ptr<Ui::CalibrationDialog> m_ui;
    const QString& m_port;
    Device::Calibration* m_results;
};
