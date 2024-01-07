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

#include "configurationdialog.h"
#include "ui_configurationdialog.h"

// ---------------------------------------------------------------------------------------------- //

ConfigurationDialog::ConfigurationDialog(QString* data, QWidget* parent)
    : QDialog(parent),
      m_ui(std::make_unique<Ui::ConfigurationDialog>()),
      m_data(data)
{
    Q_ASSERT(data != nullptr);

    m_ui->setupUi(this);
    m_ui->textEdit->setPlainText(*data);

    const int width = m_ui->textEdit->fontMetrics().size(0, *data).width();
    m_ui->textEdit->setMinimumWidth(width + 20);

    connect(this, SIGNAL(accepted()), this, SLOT(saveData()));
}

// ---------------------------------------------------------------------------------------------- //

ConfigurationDialog::~ConfigurationDialog() = default;

// ---------------------------------------------------------------------------------------------- //

void ConfigurationDialog::saveData()
{
    *m_data = m_ui->textEdit->toPlainText();
}

// ---------------------------------------------------------------------------------------------- //
