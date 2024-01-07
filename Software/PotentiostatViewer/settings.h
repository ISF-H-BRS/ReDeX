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

#include "storagewidget.h"

#include "CoSeSt2/CoSeSt2.h"

#include <QDir>
#include <QString>

COSEST_BEGIN(Settings, CoSeSt::StorageType::Global)
  COSEST_BEGIN_GROUP(Setup)
    COSEST_ENTRY(int, MeasurementType, 0)
  COSEST_END_GROUP(Setup)

  COSEST_BEGIN_GROUP(OpenCircuit)
    COSEST_ENTRY(int, Duration, 1)
  COSEST_END_GROUP(OpenCircuit)

  COSEST_BEGIN_GROUP(Electrolysis)
    COSEST_ENTRY(int,  CurrentRange,  0)
    COSEST_ENTRY(bool, AutoRange, false)
    COSEST_ENTRY(int,  Duration,      1)
    COSEST_ENTRY(int,  Potential,     0)
  COSEST_END_GROUP(Electrolysis)

  COSEST_BEGIN_GROUP(LinearSweep)
    COSEST_ENTRY(int,  CurrentRange,      0)
    COSEST_ENTRY(bool, AutoRange,     false)
    COSEST_ENTRY(int,  ScanRate,        100)
    COSEST_ENTRY(int,  StartPotential, -500)
    COSEST_ENTRY(int,  EndPotential,   +500)
  COSEST_END_GROUP(LinearSweep)

  COSEST_BEGIN_GROUP(CyclicVoltammetry)
    COSEST_ENTRY(int,  CurrentRange,      0)
    COSEST_ENTRY(bool, AutoRange,     false)
    COSEST_ENTRY(int,  ScanRate,        100)
    COSEST_ENTRY(int,  StartPotential,    0)
    COSEST_ENTRY(int,  Vertex1,        -500)
    COSEST_ENTRY(int,  Vertex2,        +500)
    COSEST_ENTRY(int,  Cycles,            1)
  COSEST_END_GROUP(CyclicVoltammetry)

  COSEST_BEGIN_GROUP(Storage)
    COSEST_ENTRY(QString, Location, QDir::currentPath())
    COSEST_ENTRY(QString, Name,     StorageWidget::DefaultName)
    COSEST_ENTRY(QString, Format,   StorageWidget::DefaultFormat)
    COSEST_ENTRY(bool,    AutoSave, false)
  COSEST_END_GROUP(Storage)
COSEST_END
