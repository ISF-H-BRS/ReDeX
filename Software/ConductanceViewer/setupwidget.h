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

namespace Ui {
    class SetupWidget;
}

class SetupWidget : public QWidget
{
    Q_OBJECT

public:
    SetupWidget(QWidget* parent = nullptr);
    ~SetupWidget();

    void setInput(Device::Input input);
    auto input() const -> Device::Input;

    void setWaveform(Device::Waveform waveform);
    auto waveform() const -> Device::Waveform;

    void setFrequency(unsigned int frequency);
    auto frequency() const -> unsigned int;

    void setAmplitude(double amplitude);
    auto amplitude() const -> double;

    void setGain(Device::Gain gain);
    auto gain() const -> Device::Gain;

    void setAutoGainEnabled(bool enable);
    auto autoGainEnabled() const -> bool;

    void setOffset(unsigned int offset);
    auto offset() const -> unsigned int;

    void saveSettings();
    void restoreSettings();

signals:
    void signalChanged(SetupWidget* widget);
    void gainChanged(SetupWidget* widget);
    void offsetChanged(SetupWidget* widget);

private slots:
    void handleSignalChanged();
    void handleGainChanged();
    void handleOffsetChanged();

private:
    void updateUi();

private:
    std::unique_ptr<Ui::SetupWidget> m_ui;
    Device::Input m_input = Device::Input::One;
};
