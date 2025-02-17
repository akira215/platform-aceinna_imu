/** ***************************************************************************
 * @file   UserAlgorithm.c
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 ******************************************************************************/
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

#include <stddef.h>
#include <string.h>

#include "GlobalConstants.h"

#include "userAPI.h"
#include "ekfAPI.h"


/******************************************************************************
 * @brief
 *  
 ******************************************************************************/
static void Algorithm()
{
    // Aceinna VG/AHRS/INS algorithm
    EKF_Algorithm();
} 

/******************************************************************************
 * @brief
 *  
 ******************************************************************************/
void InitUserAlgorithm()
{
    // Initialize algorithm variables
    EKF_Initialize((uint32_t)FREQ_200_HZ, MTLT335);
}


/******************************************************************************
 * @brief
 * @param accels [in] 
 * @param rates [in] 
 * @param mags [in] 
 * @param gps [in] 
 * @param odo [in] 
 * @param ppsDetected [in] 
 *  
 ******************************************************************************/
void RunUserNavAlgorithm(float64_t accels[],
                         float64_t rates[],
                         float64_t mags[],
                         gpsDataStruct_t* const gps,
                         odoDataStruct_t* const odo,
                         BOOL const ppsDetected)
{
    // Populate the EKF input data structure
    EKF_SetInputStruct(accels, rates, mags, gps, odo, ppsDetected);

    // Call the desired algorithm based on the EKF with different
    //   calling rates and different settings.
    Algorithm();

    // Fill the output data structure with the EKF states and other 
    //   desired information
    EKF_SetOutputStruct();

}
