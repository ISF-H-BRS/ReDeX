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

#include "logfiledialog.h"
#include "ui_logfiledialog.h"

#include <QFileDialog>
#include <QMessageBox>

// ---------------------------------------------------------------------------------------------- //

LogFileDialog::LogFileDialog(const QString& filename, const QString& data, QWidget* parent)
    : QDialog(parent),
      m_ui(std::make_unique<Ui::LogFileDialog>()),
      m_filename(filename)
{
    m_ui->setupUi(this);
    m_ui->logData->setPlainText(data);

    const int width = m_ui->logData->fontMetrics().size(0, data).width();
    m_ui->logData->setMinimumWidth(width + 20);

    connect(m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(saveFile()));
}

// ---------------------------------------------------------------------------------------------- //

LogFileDialog::~LogFileDialog() = default;

// ---------------------------------------------------------------------------------------------- //

void LogFileDialog::saveFile()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save Log File",
                                                    QDir::homePath() + "/" + m_filename,
                                                    "Log files (*.txt *.old)");
    if (filename.isEmpty())
        return;

    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, "Error",
                              QString("Unable to open file %1 for writing.").arg(filename));
        return;
    }

    file.write(m_ui->logData->toPlainText().toUtf8());
    file.close();

    accept();
}

// ---------------------------------------------------------------------------------------------- //
