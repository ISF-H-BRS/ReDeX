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
#include "ui_aboutdialog.h"

// ---------------------------------------------------------------------------------------------- //

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent),
      m_ui(std::make_unique<Ui::AboutDialog>())
{
    m_ui->setupUi(this);
    resize(sizeHint());

    QFont font = m_ui->applicationName->font();
    font.setPointSize(font.pointSize() + 4);
    m_ui->applicationName->setFont(font);

    m_ui->applicationVersion->setText(QString("Version ") + APPLICATION_VERSION);
}

// ---------------------------------------------------------------------------------------------- //

AboutDialog::~AboutDialog() = default;

// ---------------------------------------------------------------------------------------------- //
