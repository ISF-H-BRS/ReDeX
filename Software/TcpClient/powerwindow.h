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

#include <QLabel>
#include <QWidget>

#include <memory>

namespace Ui {
    class PowerWindow;
}

class PowerWindow : public QWidget
{
    Q_OBJECT

public:
    PowerWindow(const std::vector<redex::NodeInfo>& infos, QWidget* parent = nullptr);
    ~PowerWindow() override;

    void setHubStatus(double temperature);
    void setNodeStatus(const QString& id, double voltage, double current, double temperature);

signals:
    void visibilityChanged(bool visible);

private:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    std::unique_ptr<Ui::PowerWindow> m_ui;
    std::map<QString, QLabel*> m_powerLabels;
};
