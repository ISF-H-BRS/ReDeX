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

#include "exception.h"
#include "macro.h"

#include <QMainWindow>
#include <QTimer>

#include <libssh/libssh.h>

#include <memory>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    REDEX_DELETE_COPY_MOVE(MainWindow)

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void connectToServer();
    void disconnectFromServer();

    void startService();
    void stopService();

    void editConfiguration();

    void downloadCurrentLog();
    void downloadLastLog();

    void rebootServer();
    void shutdownServer();

    void updateStatus();

private:
    void saveAddressHistory();
    void restoreAddressHistory();

    void tryConnect();

    void setUiConnected(bool connected);

    auto isConnected() const -> bool;

    enum class ServiceStatus
    {
        Active,
        Inactive,
        Failed
    };

    auto getServiceStatus() -> ServiceStatus;

    static auto getColor(ServiceStatus status) -> QColor;
    static auto getString(ServiceStatus status) -> QString;

    void downloadLog(const QString& filename);

    auto executeCommand(const QString& command) -> QString;

    void uploadFile(const QString& name, const QString& data);
    auto downloadFile(const QString& name) -> QString;

private:
    std::unique_ptr<Ui::MainWindow> m_ui;

    QTimer m_timer;

    ssh_session m_session = nullptr;
};
