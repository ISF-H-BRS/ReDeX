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

#ifdef _WIN32
  #ifdef REDEX_BUILD_PROCESS
    #define REDEX_EXPORT __declspec(dllexport)
  #else
    #define REDEX_EXPORT __declspec(dllimport)
  #endif

  #if defined (__i386__) || defined(_M_IX86)
    #define REDEX_ARCH_WIN32
  #endif
#else
  #define REDEX_EXPORT __attribute__((visibility("default")))
#endif

#include <stdint.h>

typedef struct _redex_lv_handle *redex_lv_handle;
typedef uint32_t redex_lv_event_ref;

typedef enum {
    REDEX_LV_SUCCESS,
    REDEX_LV_CONNECTION_FAILED,
    REDEX_LV_CONNECTION_LOST,
    REDEX_LV_INVALID_DATA
} redex_lv_result;

typedef enum {
    REDEX_LV_MEASUREMENT_STARTED,
    REDEX_LV_MEASUREMENT_STOPPED,
    REDEX_LV_MEASUREMENT_ERROR
} redex_lv_status;

#ifdef REDEX_ARCH_WIN32
#pragma pack(push,1)
#endif

typedef struct {
    int32_t count;
    unsigned char bytes[1];
} **redex_lv_string_handle;

typedef struct
{
    int32_t count;
    double entries[1];
} **redex_lv_double_array_handle;

typedef struct
{
    redex_lv_string_handle id;
    redex_lv_string_handle type;
} redex_lv_node_info;

typedef struct {
    int32_t count;
    redex_lv_node_info entries[1];
} **redex_lv_node_info_array_handle;

typedef struct {
    redex_lv_node_info_array_handle infos;
} redex_lv_node_info_data;

typedef struct {
    redex_lv_string_handle sensor_id;
    redex_lv_string_handle node_id;
    uint32_t input;
} redex_lv_sensor_info;

typedef struct {
    redex_lv_string_handle testpoint_id;
    redex_lv_sensor_info conductance_info;
    redex_lv_sensor_info orp_info;
    redex_lv_sensor_info ph_info;
    redex_lv_sensor_info potentiostat_info;
    redex_lv_sensor_info temperature_info;
} redex_lv_testpoint_info;

typedef struct {
    int32_t count;
    redex_lv_testpoint_info entries[1];
} **redex_lv_testpoint_info_array_handle;

typedef struct {
    redex_lv_testpoint_info_array_handle infos;
} redex_lv_testpoint_info_data;

typedef struct {
    redex_lv_status status;
} redex_lv_status_data;

typedef struct {
    redex_lv_string_handle testpoint_id;
    double voltage;
    double current;
    double admittance;
} redex_lv_conductance_data;

typedef struct {
    redex_lv_string_handle testpoint_id;
    double value;
} redex_lv_sensors_data;

typedef redex_lv_sensors_data redex_lv_orp_data;
typedef redex_lv_sensors_data redex_lv_ph_data;
typedef redex_lv_sensors_data redex_lv_temperature_data;

typedef struct
{
    redex_lv_string_handle testpoint_id;
    redex_lv_double_array_handle voltage;
    redex_lv_double_array_handle current;
} redex_lv_potentiostat_data;

typedef struct {
    double temperature;
} redex_lv_hub_status_data;

typedef struct {
    redex_lv_string_handle node_id;
    double voltage;
    double current;
    double temperature;
} redex_lv_node_status_data;

typedef struct
{
    redex_lv_string_handle message;
} redex_lv_error_data;

#ifdef REDEX_ARCH_WIN32
#pragma pack(pop)
#endif

#ifdef __cplusplus
extern "C" {
#endif

REDEX_EXPORT
redex_lv_result redex_lv_connect(const char *host, redex_lv_handle *handle);

REDEX_EXPORT
redex_lv_result redex_lv_close(redex_lv_handle handle);

REDEX_EXPORT
redex_lv_result redex_lv_register_node_info_event(redex_lv_handle handle,
                                                  redex_lv_event_ref *ref);
REDEX_EXPORT
redex_lv_result redex_lv_register_testpoint_info_event(redex_lv_handle handle,
                                                       redex_lv_event_ref *ref);
REDEX_EXPORT
redex_lv_result redex_lv_register_status_event(redex_lv_handle handle,
                                               redex_lv_event_ref *ref);
REDEX_EXPORT
redex_lv_result redex_lv_register_conductance_event(redex_lv_handle handle,
                                                    redex_lv_event_ref *ref);
REDEX_EXPORT
redex_lv_result redex_lv_register_orp_event(redex_lv_handle handle,
                                            redex_lv_event_ref *ref);
REDEX_EXPORT
redex_lv_result redex_lv_register_ph_event(redex_lv_handle handle,
                                           redex_lv_event_ref *ref);
REDEX_EXPORT
redex_lv_result redex_lv_register_potentiostat_event(redex_lv_handle handle,
                                                     redex_lv_event_ref *ref);
REDEX_EXPORT
redex_lv_result redex_lv_register_temperature_event(redex_lv_handle handle,
                                                    redex_lv_event_ref *ref);
REDEX_EXPORT
redex_lv_result redex_lv_register_hub_status_event(redex_lv_handle handle,
                                                   redex_lv_event_ref *ref);
REDEX_EXPORT
redex_lv_result redex_lv_register_node_status_event(redex_lv_handle handle,
                                                    redex_lv_event_ref *ref);
REDEX_EXPORT
redex_lv_result redex_lv_register_error_event(redex_lv_handle handle,
                                              redex_lv_event_ref *ref);
REDEX_EXPORT
redex_lv_result redex_lv_request_node_info(redex_lv_handle handle);

REDEX_EXPORT
redex_lv_result redex_lv_request_testpoint_info(redex_lv_handle handle);

REDEX_EXPORT
redex_lv_result redex_lv_start_power_monitor(redex_lv_handle handle);

REDEX_EXPORT
redex_lv_result redex_lv_stop_power_monitor(redex_lv_handle handle);

REDEX_EXPORT
redex_lv_result redex_lv_start_measurement(redex_lv_handle handle);

REDEX_EXPORT
redex_lv_result redex_lv_stop_measurement(redex_lv_handle handle);

REDEX_EXPORT
redex_lv_result redex_lv_filter_voltammetry_data(const redex_lv_double_array_handle input,
                                                 redex_lv_double_array_handle output);
REDEX_EXPORT
const char* redex_lv_get_last_error(void);

#ifdef __cplusplus
} // extern "C"
#endif
