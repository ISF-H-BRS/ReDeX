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

#include "logger.h"

#include <QDateTime>
#include <QFile>
#include <QTextStream>

// ---------------------------------------------------------------------------------------------- //

namespace {
    QFile g_logFile;
}

// ---------------------------------------------------------------------------------------------- //

static
void logMessage(FILE* out, const QString& level, const QString& message)
{
    QTextStream(out) << message << Qt::endl;

    if (g_logFile.isOpen())
    {
        QTextStream(&g_logFile) << QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss")
                                << " [" << level << "] " << message << Qt::endl;
    }
}

// ---------------------------------------------------------------------------------------------- //

void Logger::setLogFile(const QString& filename)
{
    if (g_logFile.isOpen())
        g_logFile.close();

    if (QFile::exists(filename))
    {
        if (QFile::exists(filename+ ".old"))
            QFile::remove(filename+ ".old");

        QFile::rename(filename, filename + ".old");
    }

    g_logFile.setFileName(filename);

    if (!g_logFile.open(QIODevice::WriteOnly | QIODevice::Text))
        throw Exception("Unable to open file " + filename + " for writing.");
}

// ---------------------------------------------------------------------------------------------- //

void Logger::info(const QString& message)
{
    logMessage(stdout, "I", message);
}

// ---------------------------------------------------------------------------------------------- //

void Logger::warning(const QString& message)
{
    logMessage(stdout, "W", message);
}

// ---------------------------------------------------------------------------------------------- //

void Logger::error(const QString& message)
{
    logMessage(stdout, "E", message);
}

// ---------------------------------------------------------------------------------------------- //
