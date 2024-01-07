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
#include "storagewidget.h"

#include "ui_storagewidget.h"

#include <QDateTime>
#include <QDir>
#include <QFileDialog>

// ---------------------------------------------------------------------------------------------- //

StorageWidget::StorageWidget(QWidget* parent)
    : QWidget(parent),
      m_ui(std::make_unique<Ui::StorageWidget>())
{
    m_ui->setupUi(this);

    connect(m_ui->selectLocationButton, SIGNAL(clicked()), this, SLOT(selectLocation()));
    connect(m_ui->autoSave, SIGNAL(toggled(bool)), this, SIGNAL(autoSaveToggled(bool)));
}

// ---------------------------------------------------------------------------------------------- //

StorageWidget::~StorageWidget() = default;

// ---------------------------------------------------------------------------------------------- //

void StorageWidget::saveSettings() const
{
    auto settings = Settings::getInstance()->getStorageGroup();

    settings->setLocation(m_ui->location->text());
    settings->setName(m_ui->name->text());
    settings->setFormat(m_ui->format->text());
    settings->setAutoSave(m_ui->autoSave->isChecked());
}

// ---------------------------------------------------------------------------------------------- //

void StorageWidget::loadSettings()
{
    const auto& settings = Settings::getConstInstance().getStorageGroup();

    QString location = settings.getLocation();

    if (!QDir(location).exists())
        location = QDir::currentPath();

    QString name = settings.getName();

    if (name.isEmpty())
        name = DefaultName;

    QString format = settings.getFormat();

    if (format.isEmpty())
        format = DefaultFormat;

    m_ui->location->setText(location);
    m_ui->name->setText(name);
    m_ui->format->setText(format);
    m_ui->autoSave->setChecked(settings.getAutoSave());
}

// ---------------------------------------------------------------------------------------------- //

auto StorageWidget::getFilePath() const -> QString
{
    QString location = m_ui->location->text();

    if (!QDir(location).exists())
        location = QDir::currentPath();

    QString name = m_ui->name->text();

    if (name.isEmpty())
        name = DefaultName;

    QString format = m_ui->format->text();

    if (format.isEmpty())
        format = DefaultFormat;

    const auto date = QDate::currentDate();
    const auto time = QTime::currentTime();

    format.replace("%N", name);
    format.replace("%Y", QString::number(date.year()));
    format.replace("%M", QString::number(date.month()));
    format.replace("%D", QString::number(date.day()));
    format.replace("%h", QString::number(time.hour()));
    format.replace("%m", QString::number(time.minute()));
    format.replace("%s", QString::number(time.second()));

    if (format.isEmpty())
        format = "Unnamed";

    QString filename = format;
    int suffix = 0;

    while (QDir(location).exists(filename + ".csv"))
        filename = format + "_" + QString::number(++suffix);

    return location + "/" + filename + ".csv";
}

// ---------------------------------------------------------------------------------------------- //

auto StorageWidget::autoSaveEnabled() const -> bool
{
    return m_ui->autoSave->isChecked();
}

// ---------------------------------------------------------------------------------------------- //

void StorageWidget::selectLocation()
{
    const QString location =
            QFileDialog::getExistingDirectory(nullptr, "Select Location", m_ui->location->text());

    if (location.isEmpty())
        return;

    m_ui->location->setText(location);
}

// ---------------------------------------------------------------------------------------------- //
