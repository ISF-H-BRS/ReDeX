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

#include "configurationdialog.h"
#include "logfiledialog.h"
#include "mainwindow.h"

#include "ui_mainwindow.h"

#include <QApplication>
#include <QMessageBox>
#include <QSettings>

// ---------------------------------------------------------------------------------------------- //

class SshException : public Exception
{
public:
    SshException(ssh_session s, const QString& msg)
        : Exception(QString("%1: %2.").arg(msg, ssh_get_error(s))) {}
};

// ---------------------------------------------------------------------------------------------- //

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_ui(std::make_unique<Ui::MainWindow>())
{
    m_ui->setupUi(this);

    restoreAddressHistory();

    const int size = QFontMetrics(QApplication::font()).ascent();
    m_ui->statusLed->setFixedSize(size, size);

    m_timer.setInterval(1000);
    m_timer.setSingleShot(false);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(updateStatus()));

    connect(m_ui->connectButton, SIGNAL(clicked()), this, SLOT(connectToServer()));
    connect(m_ui->disconnectButton, SIGNAL(clicked()), this, SLOT(disconnectFromServer()));

    connect(m_ui->startButton, SIGNAL(clicked()), this, SLOT(startService()));
    connect(m_ui->stopButton, SIGNAL(clicked()), this, SLOT(stopService()));

    connect(m_ui->editConfigButton, SIGNAL(clicked()), this, SLOT(editConfiguration()));

    connect(m_ui->downloadCurrentLogButton, SIGNAL(clicked()), this, SLOT(downloadCurrentLog()));
    connect(m_ui->downloadLastLogButton, SIGNAL(clicked()), this, SLOT(downloadLastLog()));

    connect(m_ui->rebootButton, SIGNAL(clicked()), this, SLOT(rebootServer()));
    connect(m_ui->shutdownButton, SIGNAL(clicked()), this, SLOT(shutdownServer()));
}

// ---------------------------------------------------------------------------------------------- //

MainWindow::~MainWindow()
{
    disconnectFromServer();
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::updateStatus()
{
    try {
        const ServiceStatus status = getServiceStatus();

        QPalette palette = m_ui->statusLed->palette();
        palette.setColor(QPalette::Active, QPalette::Window, getColor(status));
        palette.setColor(QPalette::Inactive, QPalette::Window, getColor(status));
        m_ui->statusLed->setPalette(palette);

        m_ui->serviceStatus->setText(getString(status));

        m_ui->startButton->setEnabled(status != ServiceStatus::Active);
        m_ui->stopButton->setEnabled(status == ServiceStatus::Active);
    }
    catch (const std::exception& e)
    {
        disconnectFromServer();
        QMessageBox::critical(this, "Error", e.what());
    }
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::connectToServer()
{
    try {
        tryConnect();
        saveAddressHistory();
        updateStatus();
        setUiConnected(true);

        m_timer.start();
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::disconnectFromServer()
{
    m_timer.stop();
    setUiConnected(false);

    if (m_session)
    {
        ssh_disconnect(m_session);
        ssh_free(m_session);
        m_session = nullptr;
    }
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::startService()
{
    try {
        executeCommand("systemctl start redex-server");
        updateStatus();
    }
    catch (const std::exception& e)
    {
        disconnectFromServer();
        QMessageBox::critical(this, "Error", e.what());
    }
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::stopService()
{
    try {
        executeCommand("systemctl stop redex-server");
        updateStatus();
    }
    catch (const std::exception& e)
    {
        disconnectFromServer();
        QMessageBox::critical(this, "Error", e.what());
    }
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::editConfiguration()
{
    const QString filename = "config.xml";

    try {
        QString data = downloadFile(filename);
        ConfigurationDialog dialog(&data, this);

        int result = dialog.exec();

        if (result == QDialog::Accepted)
        {
            uploadFile(filename, data);

            result = QMessageBox::question(this, "Upload", "The configuration has been uploaded "
                                                           "to the server. Would you like to "
                                                           "restart the service now?");
            if (result == QMessageBox::Yes)
            {
                stopService();
                startService();
            }
        }
    }
    catch (const std::exception& e)
    {
        disconnectFromServer();
        QMessageBox::critical(this, "Error", e.what());
    }
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::downloadCurrentLog()
{
    downloadLog("log.txt");
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::downloadLastLog()
{
    downloadLog("log.txt.old");
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::rebootServer()
{
    int result = QMessageBox::question(this, "Reboot", "Are you sure you want to "
                                                       "reboot the server now?");
    if (result != QMessageBox::Yes)
        return;

    m_timer.stop();

    try {
        executeCommand("sudo reboot");
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", e.what());
    }

    disconnectFromServer();
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::shutdownServer()
{
    int result = QMessageBox::question(this, "Shutdown", "Are you sure you want to "
                                                         "shutdown the server now?");
    if (result != QMessageBox::Yes)
        return;

    m_timer.stop();

    try {
        executeCommand("sudo poweroff");
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", e.what());
    }

    disconnectFromServer();
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::saveAddressHistory()
{
    QStringList addresses;
    addresses << m_ui->serverAddress->currentText();

    for (int i = 0; i < m_ui->serverAddress->count(); ++i)
        addresses << m_ui->serverAddress->itemText(i);

    addresses.removeDuplicates();

    QSettings settings;
    settings.setValue("addressHistory", addresses);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::restoreAddressHistory()
{
    QSettings settings;

    QStringList addresses = settings.value("addressHistory", QStringList()).toStringList();
    addresses.removeDuplicates();

    if (!addresses.isEmpty())
    {
        m_ui->serverAddress->clear();
        m_ui->serverAddress->addItems(addresses);
    }
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::tryConnect()
{
    if (m_session)
        disconnectFromServer();

    m_session = ssh_new();
    Q_ASSERT(m_session != nullptr);

    ssh_set_blocking(m_session, true);

    const int verbosity = SSH_LOG_NOLOG;
    const int port = 22;

    ssh_options_set(m_session, SSH_OPTIONS_HOST, m_ui->serverAddress->currentText().toUtf8());
    ssh_options_set(m_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
    ssh_options_set(m_session, SSH_OPTIONS_PORT, &port);
    ssh_options_set(m_session, SSH_OPTIONS_USER, "redex");

    int result = ssh_connect(m_session);

    if (result != SSH_OK)
        throw SshException(m_session, "Unable to connect to server");

    result = ssh_userauth_password(m_session, nullptr, "redexpw");

    if (result != SSH_AUTH_SUCCESS)
        throw SshException(m_session, "Unable to authenticate");
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::setUiConnected(bool connected)
{
    m_ui->serverWidget->setEnabled(!connected);
    m_ui->connectButton->setEnabled(!connected);
    m_ui->disconnectButton->setEnabled(connected);

    m_ui->serviceGroup->setEnabled(connected);
    m_ui->configGroup->setEnabled(connected);
    m_ui->logGroup->setEnabled(connected);
    m_ui->serverGroup->setEnabled(connected);
}

// ---------------------------------------------------------------------------------------------- //

auto MainWindow::isConnected() const -> bool
{
    return m_session != nullptr && ssh_is_connected(m_session);
}

// ---------------------------------------------------------------------------------------------- //

auto MainWindow::getServiceStatus() -> ServiceStatus
{
    const QString response = executeCommand("systemctl is-active redex-server").trimmed();

    if (response == "active")
        return ServiceStatus::Active;

    if (response == "failed")
        return ServiceStatus::Failed;

    return ServiceStatus::Inactive;
}

// ---------------------------------------------------------------------------------------------- //

auto MainWindow::getColor(ServiceStatus status) -> QColor
{
    if (status == ServiceStatus::Active)
        return Qt::green;

    if (status == ServiceStatus::Failed)
        return Qt::red;

    return Qt::lightGray;
}

// ---------------------------------------------------------------------------------------------- //

auto MainWindow::getString(ServiceStatus status) -> QString
{
    if (status == ServiceStatus::Active)
        return "Service started";

    if (status == ServiceStatus::Failed)
        return "Service failed";

    return "Service stopped";
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::downloadLog(const QString& filename)
{
    try {
        const QString data = downloadFile(filename);

        LogFileDialog dialog(filename, data, this);
        dialog.exec();
    }
    catch (const std::exception& e)
    {
        disconnectFromServer();
        QMessageBox::critical(this, "Error", e.what());
    }
}

// ---------------------------------------------------------------------------------------------- //

auto MainWindow::executeCommand(const QString& command) -> QString
{
    ssh_channel channel = ssh_channel_new(m_session);
    Q_ASSERT(channel != nullptr);

    int result = ssh_channel_open_session(channel);

    if (result != SSH_OK)
        throw SshException(m_session, "Unable to open channel");

    result = ssh_channel_request_exec(channel, command.toUtf8());

    if (result != SSH_OK)
    {
        ssh_channel_close(channel);
        ssh_channel_free(channel);

        throw SshException(m_session, "Unable to execute command");
    }

    QByteArray response;
    std::array<char, 32> buffer = {};

    while (!ssh_channel_is_eof(channel))
    {
        int nbytes = ssh_channel_read_timeout(channel, buffer.data(), buffer.size(), false, 1000);

        if (nbytes <= 0)
            break;

        response.append(buffer.data(), nbytes);
    }

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);

    return QString::fromUtf8(response);
}

// ---------------------------------------------------------------------------------------------- //

void MainWindow::uploadFile(const QString& name, const QString& data)
{
    ssh_scp scp = nullptr;

    const auto cleanup = [&scp]
    {
        if (scp)
        {
            ssh_scp_close(scp);
            ssh_scp_free(scp);
        }
    };

    try {
        scp = ssh_scp_new(m_session, SSH_SCP_WRITE | SSH_SCP_RECURSIVE, "./ReDeX/");

        if (!scp)
            throw SshException(m_session, "Unable to allocate scp session");

        int result = ssh_scp_init(scp);

        if (result != SSH_OK)
            throw SshException(m_session, "Unable to initialize scp session");

        QByteArray bytes = data.toUtf8();
        const auto size = static_cast<size_t>(bytes.size());

        result = ssh_scp_push_file(scp, name.toUtf8(), size, 0644);

        if (result != SSH_OK)
            throw SshException(m_session, "Can't open remote file");

        result = ssh_scp_write(scp, bytes, size);

        if (result != SSH_OK)
            throw SshException(m_session, "Can't write to remote file");

        cleanup();
    }
    catch (...)
    {
        cleanup();
        throw;
    }
}

// ---------------------------------------------------------------------------------------------- //

auto MainWindow::downloadFile(const QString& name) -> QString
{
    ssh_scp scp = nullptr;

    const auto cleanup = [&scp]
    {
        if (scp)
        {
            ssh_scp_close(scp);
            ssh_scp_free(scp);
        }
    };

    try {
        scp = ssh_scp_new(m_session, SSH_SCP_READ, "./ReDeX/" + name.toUtf8());

        if (!scp)
            throw SshException(m_session, "Unable to allocate scp session");

        int result = ssh_scp_init(scp);

        if (result != SSH_OK)
            throw SshException(m_session, "Unable to initialize scp session");

        result = ssh_scp_pull_request(scp);

        if (result != SSH_SCP_REQUEST_NEWFILE)
            throw SshException(m_session, "Error receiving information about file");

        const size_t size = ssh_scp_request_get_size(scp);
        std::vector<char> buffer(size);

        ssh_scp_accept_request(scp);

        size_t read = 0;

        do {
            result = ssh_scp_read(scp, buffer.data() + read, buffer.size() - read);

            if (result < 0)
                throw SshException(m_session, "Error receiving file data");

            read += result;

        } while (read < buffer.size());

        result = ssh_scp_pull_request(scp);

        if (result != SSH_SCP_REQUEST_EOF)
            throw SshException(m_session, "Unexpected request");

        cleanup();
        return QString::fromUtf8(buffer.data(), static_cast<int>(buffer.size()));
    }
    catch (...)
    {
        cleanup();
        throw;
    }
}

// ---------------------------------------------------------------------------------------------- //
