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

#include "setupwidget.h"
#include "ui_setupwidget.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSettings>
#include <QSpinBox>

#include <cmath>

// ---------------------------------------------------------------------------------------------- //

SetupWidget::SetupWidget(QWidget* parent)
    : QWidget(parent),
      m_ui(std::make_unique<Ui::SetupWidget>())
{
    m_ui->setupUi(this);

    for (size_t i = 0; i < Device::WaveformCount; ++i)
        m_ui->waveform->addItem(Device::toString(Device::toWaveform(i)));

    m_ui->waveform->setCurrentIndex(Device::indexOf(Device::Waveform::Sine));

    for (size_t i = 0; i < Device::GainCount; ++i)
        m_ui->gain->addItem(Device::toString(Device::toGain(i)));

    m_ui->gain->setCurrentIndex(Device::indexOf(Device::Gain::_100));

    updateUi();

    connect(m_ui->waveform, SIGNAL(currentIndexChanged(int)), this, SLOT(handleSignalChanged()));
    connect(m_ui->frequency, SIGNAL(valueChanged(int)), this, SLOT(handleSignalChanged()));
    connect(m_ui->amplitude, SIGNAL(valueChanged(double)), this, SLOT(handleSignalChanged()));
    connect(m_ui->gain, SIGNAL(currentIndexChanged(int)), this, SLOT(handleGainChanged()));
    connect(m_ui->offset, SIGNAL(valueChanged(int)), this, SLOT(handleOffsetChanged()));
}

// ---------------------------------------------------------------------------------------------- //

SetupWidget::~SetupWidget() = default;

// ---------------------------------------------------------------------------------------------- //

void SetupWidget::setInput(Device::Input input)
{
    m_input = input;
}

// ---------------------------------------------------------------------------------------------- //

auto SetupWidget::input() const -> Device::Input
{
    return m_input;
}

// ---------------------------------------------------------------------------------------------- //

void SetupWidget::setWaveform(Device::Waveform waveform)
{
    m_ui->waveform->setCurrentIndex(Device::indexOf(waveform));
}

// ---------------------------------------------------------------------------------------------- //

auto SetupWidget::waveform() const -> Device::Waveform
{
    return Device::toWaveform(m_ui->waveform->currentIndex());
}

// ---------------------------------------------------------------------------------------------- //

void SetupWidget::setFrequency(unsigned int frequency)
{
    m_ui->frequency->setValue(static_cast<int>(frequency));
}

// ---------------------------------------------------------------------------------------------- //

auto SetupWidget::frequency() const -> unsigned int
{
    return static_cast<unsigned int>(m_ui->frequency->value());
}

// ---------------------------------------------------------------------------------------------- //

void SetupWidget::setAmplitude(double amplitude)
{
    m_ui->amplitude->setValue(amplitude);
}

// ---------------------------------------------------------------------------------------------- //

auto SetupWidget::amplitude() const -> double
{
    // Correct rounding errors from QDoubleSpinBox implementation
    const double steps = std::round(m_ui->amplitude->value() / m_ui->amplitude->singleStep());
    const double amplitude = steps * m_ui->amplitude->singleStep();

    return amplitude;
}

// ---------------------------------------------------------------------------------------------- //

void SetupWidget::setGain(Device::Gain gain)
{
    m_ui->gain->setCurrentIndex(Device::indexOf<int>(gain));
}

// ---------------------------------------------------------------------------------------------- //

auto SetupWidget::gain() const -> Device::Gain
{
    return Device::toGain(m_ui->gain->currentIndex());
}

// ---------------------------------------------------------------------------------------------- //

void SetupWidget::setAutoGainEnabled(bool enable)
{
    m_ui->autogain->setChecked(enable);
}

// ---------------------------------------------------------------------------------------------- //

auto SetupWidget::autoGainEnabled() const -> bool
{
    return m_ui->autogain->isChecked();
}

// ---------------------------------------------------------------------------------------------- //

void SetupWidget::setOffset(unsigned int offset)
{
    m_ui->offset->setValue(static_cast<int>(offset));
}

// ---------------------------------------------------------------------------------------------- //

auto SetupWidget::offset() const -> unsigned int
{
    return static_cast<unsigned int>(m_ui->offset->value());
}

// ---------------------------------------------------------------------------------------------- //

void SetupWidget::saveSettings()
{
    const auto input = Device::indexOf(m_input) + 1;

    QSettings settings;
    settings.setValue(QString("waveform%1").arg(input), m_ui->waveform->currentIndex());
    settings.setValue(QString("frequency%1").arg(input), m_ui->frequency->value());
    settings.setValue(QString("amplitude%1").arg(input), m_ui->amplitude->value());
    settings.setValue(QString("gain%1").arg(input), m_ui->gain->currentIndex());
    settings.setValue(QString("autogain%1").arg(input), m_ui->autogain->isChecked());
    settings.setValue(QString("offset%1").arg(input), m_ui->offset->value());
}

// ---------------------------------------------------------------------------------------------- //

void SetupWidget::restoreSettings()
{
    const auto input = Device::indexOf(m_input) + 1;

    QSettings settings;
    m_ui->waveform->setCurrentIndex(settings.value(QString("waveform%1").arg(input),
                                                   m_ui->waveform->currentIndex()).toInt());

    m_ui->frequency->setValue(settings.value(QString("frequency%1").arg(input),
                                             m_ui->frequency->value()).toInt());

    m_ui->amplitude->setValue(settings.value(QString("amplitude%1").arg(input),
                                             m_ui->amplitude->value()).toDouble());

    m_ui->gain->setCurrentIndex(settings.value(QString("gain%1").arg(input),
                                               m_ui->gain->currentIndex()).toInt());

    m_ui->autogain->setChecked(settings.value(QString("autogain%1").arg(input),
                                              m_ui->autogain->isChecked()).toBool());

    m_ui->offset->setValue(settings.value(QString("offset%1").arg(input),
                                          m_ui->offset->value()).toInt());
    updateUi();
}

// ---------------------------------------------------------------------------------------------- //

void SetupWidget::handleSignalChanged()
{
    updateUi();
    emit signalChanged(this);
}

// ---------------------------------------------------------------------------------------------- //

void SetupWidget::handleGainChanged()
{
    emit gainChanged(this);
}

// ---------------------------------------------------------------------------------------------- //

void SetupWidget::handleOffsetChanged()
{
    emit offsetChanged(this);
}

// ---------------------------------------------------------------------------------------------- //

void SetupWidget::updateUi()
{
    const bool enable = waveform() != Device::Waveform::None;

    m_ui->frequency->setEnabled(enable);
    m_ui->amplitude->setEnabled(enable);
}

// ---------------------------------------------------------------------------------------------- //
