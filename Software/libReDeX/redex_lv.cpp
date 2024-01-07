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

#include "tcpclient.h"
#include "voltammogramfilter.h"

#include <redex_lv.h>

#include <extcode.h> // LabVIEW API

#include <cassert>

// ---------------------------------------------------------------------------------------------- //

using namespace redex;

// ---------------------------------------------------------------------------------------------- //

namespace {
    std::string g_lastError = "";
}

// ---------------------------------------------------------------------------------------------- //

class ListenerImpl : public redex::Listener
{
public:
    void setNodeInfoRef(redex_lv_event_ref* ref);
    void setTestpointInfoRef(redex_lv_event_ref* ref);
    void setStatusRef(redex_lv_event_ref* ref);
    void setConductanceRef(redex_lv_event_ref* ref);
    void setOrpRef(redex_lv_event_ref* ref);
    void setPhRef(redex_lv_event_ref* ref);
    void setPotentiostatRef(redex_lv_event_ref* ref);
    void setTemperatureRef(redex_lv_event_ref* ref);
    void setHubStatusRef(redex_lv_event_ref* ref);
    void setNodeStatusRef(redex_lv_event_ref* ref);
    void setErrorRef(redex_lv_event_ref* ref);

private:
    void onNodeInfoReceived(std::span<const NodeInfo> info) override;

    void onTestpointInfoReceived(std::span<const TestpointInfo> info) override;

    void onStatusChanged(Status status) override;

    void onConductanceReceived(const std::string& id,
                               double voltage, double current, double admittance) override;

    void onOrpValueReceived(const std::string& id, double value) override;

    void onPhValueReceived(const std::string& id, double value) override;

    void onTemperatureReceived(const std::string& id, double value) override;

    void onVoltammogramReceived(const std::string& id,
                                std::span<const double> voltage,
                                std::span<const double> current) override;

    void onHubStatusReceived(double temperature) override;

    void onNodeStatusReceived(const std::string& nodeId,
                              double voltage, double current, double temperature) override;

    void onError(const std::string& msg) override;

    static auto makeString(const std::string& str) -> redex_lv_string_handle;

private:
    redex_lv_event_ref m_nodeInfoRef = 0;
    redex_lv_event_ref m_testpointInfoRef = 0;
    redex_lv_event_ref m_statusRef = 0;
    redex_lv_event_ref m_conductanceRef = 0;
    redex_lv_event_ref m_orpRef = 0;
    redex_lv_event_ref m_phRef = 0;
    redex_lv_event_ref m_potentiostatRef = 0;
    redex_lv_event_ref m_temperatureRef = 0;
    redex_lv_event_ref m_hubStatusRef = 0;
    redex_lv_event_ref m_nodeStatusRef = 0;
    redex_lv_event_ref m_errorRef = 0;
};

// ---------------------------------------------------------------------------------------------- //

struct _redex_lv_handle
{
    std::unique_ptr<ListenerImpl> listener;
    std::unique_ptr<TcpClient> client;
    std::string lastError = "No error.";
};

// ---------------------------------------------------------------------------------------------- //

template <typename Func>
auto tryRequest(Func func) -> redex_lv_result
{
    try {
        func();
        return REDEX_LV_SUCCESS;
    }
    catch (const std::exception& e)
    {
        g_lastError = e.what();
        return REDEX_LV_CONNECTION_LOST;
    }
}

// ---------------------------------------------------------------------------------------------- //

redex_lv_result redex_lv_connect(const char* host, redex_lv_handle* handle)
{
    _redex_lv_handle* ptr = nullptr;

    try {
        ptr = new _redex_lv_handle;
        ptr->listener = std::make_unique<ListenerImpl>();
        ptr->client = std::make_unique<TcpClient>(host, ptr->listener.get());

        *handle = ptr;
        return REDEX_LV_SUCCESS;
    }
    catch (const std::exception& e)
    {
        delete ptr;

        g_lastError = e.what();

        *handle = nullptr;
        return REDEX_LV_CONNECTION_FAILED;
    }
}

// ---------------------------------------------------------------------------------------------- //

redex_lv_result redex_lv_close(redex_lv_handle handle)
{
    delete handle;
    return REDEX_LV_SUCCESS;
}

// ---------------------------------------------------------------------------------------------- //

redex_lv_result redex_lv_register_node_info_event(redex_lv_handle handle,
                                                  redex_lv_event_ref *ref)
{
    handle->listener->setNodeInfoRef(ref);
    return REDEX_LV_SUCCESS;
}

// ---------------------------------------------------------------------------------------------- //

redex_lv_result redex_lv_register_testpoint_info_event(redex_lv_handle handle,
                                                       redex_lv_event_ref* ref)
{
    handle->listener->setTestpointInfoRef(ref);
    return REDEX_LV_SUCCESS;
}

// ---------------------------------------------------------------------------------------------- //

redex_lv_result redex_lv_register_status_event(redex_lv_handle handle, redex_lv_event_ref* ref)
{
    handle->listener->setStatusRef(ref);
    return REDEX_LV_SUCCESS;
}

// ---------------------------------------------------------------------------------------------- //

redex_lv_result redex_lv_register_conductance_event(redex_lv_handle handle, redex_lv_event_ref* ref)
{
    handle->listener->setConductanceRef(ref);
    return REDEX_LV_SUCCESS;
}

// ---------------------------------------------------------------------------------------------- //

redex_lv_result redex_lv_register_orp_event(redex_lv_handle handle, redex_lv_event_ref* ref)
{
    handle->listener->setOrpRef(ref);
    return REDEX_LV_SUCCESS;
}

// ---------------------------------------------------------------------------------------------- //

redex_lv_result redex_lv_register_ph_event(redex_lv_handle handle, redex_lv_event_ref* ref)
{
    handle->listener->setPhRef(ref);
    return REDEX_LV_SUCCESS;
}

// ---------------------------------------------------------------------------------------------- //

redex_lv_result redex_lv_register_potentiostat_event(redex_lv_handle handle,
                                                     redex_lv_event_ref* ref)
{
    handle->listener->setPotentiostatRef(ref);
    return REDEX_LV_SUCCESS;
}

// ---------------------------------------------------------------------------------------------- //

redex_lv_result redex_lv_register_temperature_event(redex_lv_handle handle, redex_lv_event_ref* ref)
{
    handle->listener->setTemperatureRef(ref);
    return REDEX_LV_SUCCESS;
}

// ---------------------------------------------------------------------------------------------- //

redex_lv_result redex_lv_register_hub_status_event(redex_lv_handle handle, redex_lv_event_ref *ref)
{
    handle->listener->setHubStatusRef(ref);
    return REDEX_LV_SUCCESS;
}

// ---------------------------------------------------------------------------------------------- //

redex_lv_result redex_lv_register_node_status_event(redex_lv_handle handle, redex_lv_event_ref *ref)
{
    handle->listener->setNodeStatusRef(ref);
    return REDEX_LV_SUCCESS;
}

// ---------------------------------------------------------------------------------------------- //

redex_lv_result redex_lv_register_error_event(redex_lv_handle handle, redex_lv_event_ref* ref)
{
    handle->listener->setErrorRef(ref);
    return REDEX_LV_SUCCESS;
}

// ---------------------------------------------------------------------------------------------- //

redex_lv_result redex_lv_request_node_info(redex_lv_handle handle)
{
    return tryRequest([handle]{ handle->client->requestNodeInfo(); });
}

// ---------------------------------------------------------------------------------------------- //

redex_lv_result redex_lv_request_testpoint_info(redex_lv_handle handle)
{
    return tryRequest([handle]{ handle->client->requestTestpointInfo(); });
}

// ---------------------------------------------------------------------------------------------- //

redex_lv_result redex_lv_start_power_monitor(redex_lv_handle handle)
{
    return tryRequest([handle]{ handle->client->startPowerMonitor(); });
}

// ---------------------------------------------------------------------------------------------- //

redex_lv_result redex_lv_stop_power_monitor(redex_lv_handle handle)
{
    return tryRequest([handle]{ handle->client->stopPowerMonitor(); });
}

// ---------------------------------------------------------------------------------------------- //

redex_lv_result redex_lv_start_measurement(redex_lv_handle handle)
{
    return tryRequest([handle]{ handle->client->startMeasurement(); });
}

// ---------------------------------------------------------------------------------------------- //

redex_lv_result redex_lv_stop_measurement(redex_lv_handle handle)
{
    return tryRequest([handle]{ handle->client->stopMeasurement(); });
}

// ---------------------------------------------------------------------------------------------- //

redex_lv_result redex_lv_filter_voltammetry_data(const redex_lv_double_array_handle input,
                                                 redex_lv_double_array_handle output)
{
    static VoltammogramFilter filter;

    NumericArrayResize(::fD, 1, reinterpret_cast<UHandle*>(&output), (*input)->count);
    (*output)->count = (*input)->count;

    try {
        std::span<const double> in((*input)->entries, (*input)->count);
        std::span<double> out((*output)->entries, (*output)->count);

        filter.apply(in, out);
        return REDEX_LV_SUCCESS;
    }
    catch (const std::exception& e)
    {
        g_lastError = e.what();
        return REDEX_LV_INVALID_DATA;
    }
}

// ---------------------------------------------------------------------------------------------- //

const char* redex_lv_get_last_error()
{
    return g_lastError.c_str();
}

// ---------------------------------------------------------------------------------------------- //
// ---------------------------------------------------------------------------------------------- //

void ListenerImpl::setNodeInfoRef(redex_lv_event_ref* ref)
{
    m_nodeInfoRef = *ref;
}

// ---------------------------------------------------------------------------------------------- //

void ListenerImpl::setTestpointInfoRef(redex_lv_event_ref* ref)
{
    m_testpointInfoRef = *ref;
}

// ---------------------------------------------------------------------------------------------- //

void ListenerImpl::setStatusRef(redex_lv_event_ref* ref)
{
    m_statusRef = *ref;
}

// ---------------------------------------------------------------------------------------------- //

void ListenerImpl::setConductanceRef(redex_lv_event_ref* ref)
{
    m_conductanceRef = *ref;
}

// ---------------------------------------------------------------------------------------------- //

void ListenerImpl::setOrpRef(redex_lv_event_ref* ref)
{
    m_orpRef = *ref;
}

// ---------------------------------------------------------------------------------------------- //

void ListenerImpl::setPhRef(redex_lv_event_ref* ref)
{
    m_phRef = *ref;
}

// ---------------------------------------------------------------------------------------------- //

void ListenerImpl::setPotentiostatRef(redex_lv_event_ref* ref)
{
    m_potentiostatRef = *ref;
}

// ---------------------------------------------------------------------------------------------- //

void ListenerImpl::setTemperatureRef(redex_lv_event_ref* ref)
{
    m_temperatureRef = *ref;
}

// ---------------------------------------------------------------------------------------------- //

void ListenerImpl::setHubStatusRef(redex_lv_event_ref* ref)
{
    m_hubStatusRef = *ref;
}

// ---------------------------------------------------------------------------------------------- //

void ListenerImpl::setNodeStatusRef(redex_lv_event_ref* ref)
{
    m_nodeStatusRef = *ref;
}

// ---------------------------------------------------------------------------------------------- //

void ListenerImpl::setErrorRef(redex_lv_event_ref* ref)
{
    m_errorRef = *ref;
}

// ---------------------------------------------------------------------------------------------- //

void ListenerImpl::onNodeInfoReceived(std::span<const NodeInfo> infos)
{
    static auto makeNodeInfo = [](const NodeInfo& info) -> redex_lv_node_info
    {
        redex_lv_node_info lvInfo = {};

        lvInfo.id = makeString(info.id);
        lvInfo.type = makeString(info.type);

        return lvInfo;
    };

    static auto freeNodeInfo = [](const redex_lv_node_info& lvInfo)
    {
        DSDisposeHandle(lvInfo.id);
        DSDisposeHandle(lvInfo.type);
    };

    if (!m_nodeInfoRef)
        return;

    UHandle handle = DSNewHandle(sizeof(int32) + infos.size() * sizeof(redex_lv_node_info));

    redex_lv_node_info_data data = {};
    data.infos = reinterpret_cast<redex_lv_node_info_array_handle>(handle);

    (*data.infos)->count = static_cast<int32_t>(infos.size());

    for (size_t i = 0; i < infos.size(); ++i)
    {
        const NodeInfo& info = infos[i];
        assert(!info.id.empty());

        (*data.infos)->entries[i] = makeNodeInfo(info);
    }

    PostLVUserEvent(m_nodeInfoRef, &data);

    for (size_t i = 0; i < infos.size(); ++i)
        freeNodeInfo((*data.infos)->entries[i]);

    DSDisposeHandle(handle);
}

// ---------------------------------------------------------------------------------------------- //

void ListenerImpl::onTestpointInfoReceived(std::span<const TestpointInfo> infos)
{
    static auto makeSensorInfo = [](const SensorInfo& info) -> redex_lv_sensor_info
    {
        redex_lv_sensor_info lvInfo = {};

        lvInfo.sensor_id = makeString(info.sensorId);
        lvInfo.node_id = makeString(info.nodeId);
        lvInfo.input = info.input;

        return lvInfo;
    };

    static auto freeSensorInfo = [](const redex_lv_sensor_info& lvInfo)
    {
        DSDisposeHandle(lvInfo.sensor_id);
        DSDisposeHandle(lvInfo.node_id);
    };

    static auto freeTestpointInfo = [](const redex_lv_testpoint_info& lvInfo)
    {
        DSDisposeHandle(lvInfo.testpoint_id);
        freeSensorInfo(lvInfo.conductance_info);
        freeSensorInfo(lvInfo.orp_info);
        freeSensorInfo(lvInfo.ph_info);
        freeSensorInfo(lvInfo.potentiostat_info);
        freeSensorInfo(lvInfo.temperature_info);
    };

    if (!m_testpointInfoRef)
        return;

    UHandle handle = DSNewHandle(sizeof(int32) + infos.size() * sizeof(redex_lv_testpoint_info));

    redex_lv_testpoint_info_data data = {};
    data.infos = reinterpret_cast<redex_lv_testpoint_info_array_handle>(handle);

    (*data.infos)->count = static_cast<int32_t>(infos.size());

    for (size_t i = 0; i < infos.size(); ++i)
    {
        const TestpointInfo& info = infos[i];
        assert(!info.testpointId.empty());

        redex_lv_testpoint_info& lvInfo = (*data.infos)->entries[i];

        lvInfo.testpoint_id = makeString(info.testpointId);
        lvInfo.conductance_info = makeSensorInfo(info.conductanceInfo);
        lvInfo.orp_info = makeSensorInfo(info.orpInfo);
        lvInfo.ph_info = makeSensorInfo(info.phInfo);
        lvInfo.potentiostat_info = makeSensorInfo(info.potentiostatInfo);
        lvInfo.temperature_info = makeSensorInfo(info.temperatureInfo);
    }

    PostLVUserEvent(m_testpointInfoRef, &data);

    for (size_t i = 0; i < infos.size(); ++i)
        freeTestpointInfo((*data.infos)->entries[i]);

    DSDisposeHandle(handle);
}

// ---------------------------------------------------------------------------------------------- //

void ListenerImpl::onStatusChanged(Status status)
{
    static const auto toLvStatus = [](Status status) -> redex_lv_status
    {
        if (status == Status::MeasurementStarted)
            return REDEX_LV_MEASUREMENT_STARTED;

        if (status == Status::MeasurementStopped)
            return REDEX_LV_MEASUREMENT_STOPPED;

        return REDEX_LV_MEASUREMENT_ERROR;
    };

    if (m_statusRef)
    {
        redex_lv_status_data data = { toLvStatus(status) };
        PostLVUserEvent(m_statusRef, &data);
    }
}

// ---------------------------------------------------------------------------------------------- //

void ListenerImpl::onConductanceReceived(const std::string& id,
                                         double voltage, double current, double admittance)
{
    if (m_conductanceRef)
    {
        redex_lv_conductance_data data = { makeString(id), voltage, current, admittance };
        PostLVUserEvent(m_conductanceRef, &data);
        DSDisposeHandle(data.testpoint_id);
    }
}

// ---------------------------------------------------------------------------------------------- //

void ListenerImpl::onOrpValueReceived(const std::string& id, double value)
{
    if (m_orpRef)
    {
        redex_lv_orp_data data = { makeString(id), value };
        PostLVUserEvent(m_orpRef, &data);
        DSDisposeHandle(data.testpoint_id);
    }
}

// ---------------------------------------------------------------------------------------------- //

void ListenerImpl::onPhValueReceived(const std::string& id, double value)
{
    if (m_phRef)
    {
        redex_lv_ph_data data = { makeString(id), value };
        PostLVUserEvent(m_phRef, &data);
        DSDisposeHandle(data.testpoint_id);
    }
}

// ---------------------------------------------------------------------------------------------- //

void ListenerImpl::onTemperatureReceived(const std::string& id, double value)
{
    if (m_temperatureRef)
    {
        redex_lv_temperature_data data = { makeString(id), value };
        PostLVUserEvent(m_temperatureRef, &data);
        DSDisposeHandle(data.testpoint_id);
    }
}

// ---------------------------------------------------------------------------------------------- //

void ListenerImpl::onVoltammogramReceived(const std::string& id,
                                          std::span<const double> voltage,
                                          std::span<const double> current)
{
    static auto makeArray = [](std::span<const double> samples) -> redex_lv_double_array_handle
    {
        redex_lv_double_array_handle handle = nullptr;
        NumericArrayResize(::fD, 1, reinterpret_cast<UHandle*>(&handle), samples.size());

        (*handle)->count = static_cast<int32_t>(samples.size());
        MoveBlock(samples.data(), (*handle)->entries, samples.size() * sizeof(double));

        return handle;
    };

    if (!m_potentiostatRef)
        return;

    redex_lv_potentiostat_data data = {
        makeString(id),
        makeArray(voltage),
        makeArray(current)
    };

    PostLVUserEvent(m_potentiostatRef, &data);

    DSDisposeHandle(data.testpoint_id);
    DSDisposeHandle(data.voltage);
    DSDisposeHandle(data.current);
}

// ---------------------------------------------------------------------------------------------- //

void ListenerImpl::onHubStatusReceived(double temperature)
{
    if (m_hubStatusRef)
    {
        redex_lv_hub_status_data data = { temperature };
        PostLVUserEvent(m_hubStatusRef, &data);
    }
}

// ---------------------------------------------------------------------------------------------- //

void ListenerImpl::onNodeStatusReceived(const std::string& nodeId,
                                        double voltage, double current, double temperature)
{
    if (m_nodeStatusRef)
    {
        redex_lv_node_status_data data = {
            makeString(nodeId),
            voltage,
            current,
            temperature
        };

        PostLVUserEvent(m_nodeStatusRef, &data);
        DSDisposeHandle(data.node_id);
    }
}

// ---------------------------------------------------------------------------------------------- //

void ListenerImpl::onError(const std::string& msg)
{
    if (m_errorRef)
    {
        redex_lv_error_data data = { makeString(msg) };
        PostLVUserEvent(m_errorRef, &data);
        DSDisposeHandle(data.message);
    }
}

// ---------------------------------------------------------------------------------------------- //

auto ListenerImpl::makeString(const std::string& str) -> redex_lv_string_handle
{
    const size_t length = str.length();
    auto string = reinterpret_cast<redex_lv_string_handle>(DSNewHandle(sizeof(int32_t) + length));

    (*string)->count = static_cast<int32_t>(length);
    MoveBlock(str.c_str(), (*string)->bytes, length);

    return string;
}

// ---------------------------------------------------------------------------------------------- //
