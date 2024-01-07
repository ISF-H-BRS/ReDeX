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

#include "temperaturewidget.h"

#include <QVBoxLayout>

// ---------------------------------------------------------------------------------------------- //

TemperatureWidget::TemperatureWidget(QWidget* parent)
    : SensorWidget(parent),
      m_label(new QLabel)
{
    auto layout = new QVBoxLayout(this);

    QMargins margins = layout->contentsMargins();
    margins.setLeft(16);
    margins.setRight(16);
    layout->setContentsMargins(margins);

    setLayout(layout);

    QFont font = m_label->font();
    font.setPointSize(3 * font.pointSize());
    m_label->setFont(font);

    m_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_label->setFrameShape(QFrame::StyledPanel);
    m_label->setFrameShadow(QFrame::Sunken);

    layout->addWidget(m_label);

    updateLabel();
}

// ---------------------------------------------------------------------------------------------- //

void TemperatureWidget::setValue(double value)
{
    m_value = value;
    updateLabel();
}

// ---------------------------------------------------------------------------------------------- //

void TemperatureWidget::updateLabel()
{
    m_label->setText(QString("%1 °C").arg(m_value, 0, 'f', 2));
}

// ---------------------------------------------------------------------------------------------- //
