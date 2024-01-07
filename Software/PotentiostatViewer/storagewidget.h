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

#include <QWidget>

namespace Ui {
    class StorageWidget;
}

class StorageWidget : public QWidget
{
    Q_OBJECT

public:
    static constexpr const char* DefaultName = "Measurement";
    static constexpr const char* DefaultFormat = "%N-%Y%M%D-%h%m%s";

public:
    explicit StorageWidget(QWidget* parent = nullptr);
    ~StorageWidget();

    void saveSettings() const;
    void loadSettings();

    auto getFilePath() const -> QString;
    auto autoSaveEnabled() const -> bool;

signals:
    void autoSaveToggled(bool enable);

private slots:
    void selectLocation();

private:
    std::unique_ptr<Ui::StorageWidget> m_ui;
};
