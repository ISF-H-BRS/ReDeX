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

#ifndef __USB_VENDOR_CORE_H
#define __USB_VENDOR_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include  "usbd_ioreq.h"

typedef struct _USBD_VENDOR_Handle
{
    void (*InitComplete)(void *pUserData);
    void (*ControlInReceived)(uint8_t request, uint16_t length, void *pUserData);
    void (*ControlOutReceived)(uint8_t request, uint16_t length, void *pUserData);
    void (*ControlRxReady)(void *pUserData);
    void (*BulkInComplete)(void *pUserData);
    void (*BulkOutReceived)(uint16_t length, void *pUserData);

    void *pUserData;

} USBD_VENDOR_HandleTypeDef;

uint8_t USBD_VENDOR_RegisterHandle(USBD_HandleTypeDef *pdev, USBD_VENDOR_HandleTypeDef *pvend);
uint8_t USBD_VENDOR_BulkSendData(USBD_HandleTypeDef *pdev, uint8_t *data, uint16_t length);
uint8_t USBD_VENDOR_BulkPrepareRx(USBD_HandleTypeDef *pdev, uint8_t *data, uint16_t length);

extern USBD_ClassTypeDef USBD_VENDOR_ClassDriver;

#ifdef __cplusplus
}
#endif

#endif  /* __USB_VENDOR_CORE_H */
