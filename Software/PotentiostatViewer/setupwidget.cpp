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

#include "settings.h"
#include "setupwidget.h"

#include "ui_setupwidget.h"

// ---------------------------------------------------------------------------------------------- //

SetupWidget::SetupWidget(QWidget *parent)
    : QWidget(parent),
      m_ui(std::make_unique<Ui::SetupWidget>())
{
    m_ui->setupUi(this);

    const auto widgets = {
        m_ui->mainWidget,
        m_ui->openCircuitWidget, m_ui->electrolysisWidget,
        m_ui->linearSweepWidget, m_ui->cyclicVoltammetryWidget
    };

    QFontMetrics fm(QApplication::font());
    int width = 0;

    const Qt::Alignment alignment(style()->styleHint(QStyle::SH_FormLayoutLabelAlignment));

    for (auto widget : widgets)
    {
        auto layout = qobject_cast<QGridLayout*>(widget->layout());

        for (int i = 0; i < layout->rowCount(); ++i)
        {
            auto label = qobject_cast<QLabel*>(layout->itemAtPosition(i, 0)->widget());
            label->setAlignment(alignment);

            width = std::max(width, fm.horizontalAdvance(label->text()));
        }
    }

    for (auto widget : widgets)
    {
        auto layout = qobject_cast<QGridLayout*>(widget->layout());
        layout->setColumnMinimumWidth(0, width);
    }

    selectWidget(m_ui->measurementType->currentIndex());
    connect(m_ui->measurementType, SIGNAL(currentIndexChanged(int)), this, SLOT(selectWidget(int)));
}

// ---------------------------------------------------------------------------------------------- //

SetupWidget::~SetupWidget() = default;

// ---------------------------------------------------------------------------------------------- //

void SetupWidget::saveSettings() const
{
    auto settings = Settings::getInstance();

    auto setup = settings->getSetupGroup();
    setup->setMeasurementType(m_ui->measurementType->currentIndex());

    auto openCircuit = settings->getOpenCircuitGroup();
    openCircuit->setDuration(m_ui->openCircuitDuration->value());

    auto electrolysis = settings->getElectrolysisGroup();
    electrolysis->setCurrentRange(m_ui->electrolysisCurrentRange->currentIndex());
    electrolysis->setAutoRange(m_ui->electrolysisAutoRange->isChecked());
    electrolysis->setDuration(m_ui->electrolysisDuration->value());
    electrolysis->setPotential(m_ui->electrolysisPotential->value());

    auto linearSweep = settings->getLinearSweepGroup();
    linearSweep->setCurrentRange(m_ui->linearSweepCurrentRange->currentIndex());
    linearSweep->setAutoRange(m_ui->linearSweepAutoRange->isChecked());
    linearSweep->setScanRate(m_ui->linearSweepScanRate->value());
    linearSweep->setStartPotential(m_ui->linearSweepStartPotential->value());
    linearSweep->setEndPotential(m_ui->linearSweepEndPotential->value());

    auto cyclicVoltammetry = settings->getCyclicVoltammetryGroup();
    cyclicVoltammetry->setCurrentRange(m_ui->cyclicVoltammetryCurrentRange->currentIndex());
    cyclicVoltammetry->setAutoRange(m_ui->cyclicVoltammetryAutoRange->isChecked());
    cyclicVoltammetry->setScanRate(m_ui->cyclicVoltammetryScanRate->value());
    cyclicVoltammetry->setStartPotential(m_ui->cyclicVoltammetryStartPotential->value());
    cyclicVoltammetry->setVertex1(m_ui->cyclicVoltammetryVertex1->value());
    cyclicVoltammetry->setVertex2(m_ui->cyclicVoltammetryVertex2->value());
    cyclicVoltammetry->setCycles(m_ui->cyclicVoltammetryCycles->value());
}

// ---------------------------------------------------------------------------------------------- //

void SetupWidget::loadSettings()
{
    const auto& settings = Settings::getConstInstance();

    const auto& setup = settings.getSetupGroup();
    m_ui->measurementType->setCurrentIndex(setup.getMeasurementType());

    const auto& openCircuit = settings.getOpenCircuitGroup();
    m_ui->openCircuitDuration->setValue(openCircuit.getDuration());

    const auto& electrolysis = settings.getElectrolysisGroup();
    m_ui->electrolysisCurrentRange->setCurrentIndex(electrolysis.getCurrentRange());
    m_ui->electrolysisAutoRange->setChecked(electrolysis.getAutoRange());
    m_ui->electrolysisDuration->setValue(electrolysis.getDuration());
    m_ui->electrolysisPotential->setValue(electrolysis.getPotential());

    const auto& linearSweep = settings.getLinearSweepGroup();
    m_ui->linearSweepCurrentRange->setCurrentIndex(linearSweep.getCurrentRange());
    m_ui->linearSweepAutoRange->setChecked(linearSweep.getAutoRange());
    m_ui->linearSweepScanRate->setValue(linearSweep.getScanRate());
    m_ui->linearSweepStartPotential->setValue(linearSweep.getStartPotential());
    m_ui->linearSweepEndPotential->setValue(linearSweep.getEndPotential());

    const auto& cyclicVoltammetry = settings.getCyclicVoltammetryGroup();
    m_ui->cyclicVoltammetryCurrentRange->setCurrentIndex(cyclicVoltammetry.getCurrentRange());
    m_ui->cyclicVoltammetryAutoRange->setChecked(cyclicVoltammetry.getAutoRange());
    m_ui->cyclicVoltammetryScanRate->setValue(cyclicVoltammetry.getScanRate());
    m_ui->cyclicVoltammetryStartPotential->setValue(cyclicVoltammetry.getStartPotential());
    m_ui->cyclicVoltammetryVertex1->setValue(cyclicVoltammetry.getVertex1());
    m_ui->cyclicVoltammetryVertex2->setValue(cyclicVoltammetry.getVertex2());
    m_ui->cyclicVoltammetryCycles->setValue(cyclicVoltammetry.getCycles());
}

// ---------------------------------------------------------------------------------------------- //

void SetupWidget::setCurrentRange(Device::CurrentRange range)
{
    const auto index = Device::indexOf<int>(range);

    if (m_selectedWidget == m_ui->electrolysisWidget)
        m_ui->electrolysisCurrentRange->setCurrentIndex(index);
    else if (m_selectedWidget == m_ui->linearSweepWidget)
        m_ui->linearSweepCurrentRange->setCurrentIndex(index);
    else if (m_selectedWidget == m_ui->cyclicVoltammetryWidget)
        m_ui->cyclicVoltammetryCurrentRange->setCurrentIndex(index);
}

// ---------------------------------------------------------------------------------------------- //

auto SetupWidget::getSetup() const -> Device::Setup
{
    Q_ASSERT(m_selectedWidget != nullptr);

    if (m_selectedWidget == m_ui->openCircuitWidget)
    {
        return {
            Device::MeasurementType::OpenCircuit,
            Device::CurrentRange::_200nA,
            false,
            std::chrono::seconds(m_ui->openCircuitDuration->value()),
            Device::MinimumScanRate,
            0,
            0,
            0,
            Device::MinimumCycleCount
        };
    }
    else if (m_selectedWidget == m_ui->electrolysisWidget)
    {
        return {
            Device::MeasurementType::Electrolysis,
            Device::toCurrentRange(m_ui->electrolysisCurrentRange->currentIndex()),
            m_ui->electrolysisAutoRange->isChecked(),
            std::chrono::seconds(m_ui->electrolysisDuration->value()),
            Device::MinimumScanRate,
            m_ui->electrolysisPotential->value(),
            0,
            0,
            Device::MinimumCycleCount
        };
    }
    else if (m_selectedWidget == m_ui->linearSweepWidget)
    {
        return {
            Device::MeasurementType::LinearSweep,
            Device::toCurrentRange(m_ui->linearSweepCurrentRange->currentIndex()),
            m_ui->linearSweepAutoRange->isChecked(),
            Device::MinimumDuration,
            m_ui->linearSweepScanRate->value(),
            m_ui->linearSweepStartPotential->value(),
            m_ui->linearSweepEndPotential->value(),
            0,
            Device::MinimumCycleCount
        };
    }
    else if (m_selectedWidget == m_ui->cyclicVoltammetryWidget)
    {
        return {
            Device::MeasurementType::CyclicVoltammetry,
            Device::toCurrentRange(m_ui->cyclicVoltammetryCurrentRange->currentIndex()),
            m_ui->cyclicVoltammetryAutoRange->isChecked(),
            Device::MinimumDuration,
            m_ui->cyclicVoltammetryScanRate->value(),
            m_ui->cyclicVoltammetryStartPotential->value(),
            m_ui->cyclicVoltammetryVertex1->value(),
            m_ui->cyclicVoltammetryVertex2->value(),
            m_ui->cyclicVoltammetryCycles->value()
        };
    }

    Q_ASSERT(false);
    return {};
}

// ---------------------------------------------------------------------------------------------- //

auto SetupWidget::getAutoRangeEnabled() const -> bool
{
    if (m_selectedWidget == m_ui->electrolysisWidget)
        return m_ui->electrolysisAutoRange->isChecked();

    if (m_selectedWidget == m_ui->linearSweepWidget)
        return m_ui->linearSweepAutoRange->isChecked();

    if (m_selectedWidget == m_ui->cyclicVoltammetryWidget)
        return m_ui->cyclicVoltammetryAutoRange->isChecked();

    return false;
}

// ---------------------------------------------------------------------------------------------- //

void SetupWidget::selectWidget(int index)
{
    const std::array<QWidget*, 4> widgets = {
        m_ui->openCircuitWidget, m_ui->electrolysisWidget,
        m_ui->linearSweepWidget, m_ui->cyclicVoltammetryWidget
    };

    const auto i = static_cast<size_t>(index);
    Q_ASSERT(i < widgets.size());

    for (auto widget : widgets)
        widget->setHidden(true);

    m_selectedWidget = widgets[i];
    m_selectedWidget->setVisible(true);
}

// ---------------------------------------------------------------------------------------------- //
