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

namespace Ui {
    class SetupWidget;
}

class SetupWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SetupWidget(QWidget* parent = nullptr);
    ~SetupWidget();

    void saveSettings() const;
    void loadSettings();

    void setCurrentRange(Device::CurrentRange range);

    auto getSetup() const -> Device::Setup;
    auto getAutoRangeEnabled() const -> bool;

private slots:
    void selectWidget(int index);

private:
    std::unique_ptr<Ui::SetupWidget> m_ui;

    static constexpr size_t WidgetCount = 4;
    std::array<QWidget*, WidgetCount> m_widgets = {};

    QWidget* m_selectedWidget = nullptr;
};
