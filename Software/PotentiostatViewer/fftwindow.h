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

#include <QWidget>

#include <memory>
#include <span>

namespace Ui {
    class FftWindow;
}

class FftWindow : public QWidget
{
    Q_OBJECT

public:    
    FftWindow();
    ~FftWindow();

    void setFiltered(bool enable);

    void addSamples(std::span<const double> voltages, std::span<const double> currents);
    void addFilteredSamples(std::span<const double> voltages, std::span<const double> currents);

    void clear();

signals:
    void visiblityChanged(bool visible);

private:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    std::unique_ptr<Ui::FftWindow> m_ui;
};
