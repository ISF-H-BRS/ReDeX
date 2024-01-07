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

#include <redex.h>

#include <QWidget>

#include <memory>

namespace Ui {
    class TestpointWidget;
}

class TestpointWidget : public QWidget
{
    Q_OBJECT

public:
    TestpointWidget(const redex::TestpointInfo& info, QWidget* parent = nullptr);
    ~TestpointWidget() override;

    void setAdmittance(double voltage, double current, double admittance);
    void setPhValue(double value);
    void setOrpValue(double value);
    void setTemperature(double value);
    void setVoltammetryValues(size_t count);

signals:
    void voltammogramRequested(const QString& id);

private:
    std::unique_ptr<Ui::TestpointWidget> m_ui;
    const QString m_id;
};
