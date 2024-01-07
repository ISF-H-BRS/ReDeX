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

#include "analysiswidget.h"
#include "units.h"

#include "ui_analysiswidget.h"

#include <cmath>

// ---------------------------------------------------------------------------------------------- //

AnalysisWidget::AnalysisWidget(QWidget* parent)
    : QWidget(parent),
      m_ui(std::make_unique<Ui::AnalysisWidget>())
{
    m_ui->setupUi(this);
}

// ---------------------------------------------------------------------------------------------- //

AnalysisWidget::~AnalysisWidget() = default;

// ---------------------------------------------------------------------------------------------- //

void AnalysisWidget::setData(double voltage, double current, double admittance)
{
    static const auto setLabel = [](QLabel* label, double value, const QString& unit) {
        label->setText(Units::makeAdjustedText(value, 4, unit));
    };

    setLabel(m_ui->voltage, voltage, "V");
    setLabel(m_ui->current, current, "A");
    setLabel(m_ui->admittance, admittance, "S");
}

// ---------------------------------------------------------------------------------------------- //

void AnalysisWidget::clear()
{
    m_ui->voltage->setText("-");
    m_ui->current->setText("-");
    m_ui->admittance->setText("-");
}

// ---------------------------------------------------------------------------------------------- //
