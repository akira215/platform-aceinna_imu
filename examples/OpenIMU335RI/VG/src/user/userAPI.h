/** ******************************************************************************
 * @file userAPI.h API functions for Interfacing with user algorithm
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 *****************************************************************************/
/*******************************************************************************
Copyright 2018 ACEINNA, INC

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*******************************************************************************/

#ifndef USER_API_H
#define USER_API_H

#include <stdint.h>
#include "gpsAPI.h"
#include "odoAPI.h"

/*******************************************
 * @brief 
 * 
 * @param dacqRate ==
********************************************/
void  inertialAndPositionDataProcessing(int32_t const dacqRate);

/*******************************************
 * @brief 
 * 
 * @param accels ==
 * @param rates ==
 * @param mags ==
 * @param gps ==
 * @param odo ==
 * @param ppsDetected ==
********************************************/
void RunUserNavAlgorithm(float64_t accels[],
                         float64_t rates[],
                         float64_t mags[],
                         gpsDataStruct_t* const gps,
                         odoDataStruct_t* const odo,
                         BOOL const ppsDetected);

/*******************************************
 * @brief 
 * 
********************************************/
void  InitUserAlgorithm();     

/*******************************************
 * @brief 
 * 
********************************************/
void  initUserDataProcessingEngine();

#endif
