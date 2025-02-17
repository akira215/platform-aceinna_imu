/** ***************************************************************************
 * @file   UserConfiguration.c
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

#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "algorithmAPI.h"
#include "sensorsAPI.h"
#include "userAPI.h"
#include "appVersion.h"

#include "UserMessagingUART.h"
#include "UserConfigurationUart.h"

#include "Indices.h"   // For X_AXIS, etc
#include "CommonMessages.h"
#include "configurationAPI.h"
#include "halAPI.h"
#include "EcuSettings.h"




/// List of allowed packet codes 
usr_packet_t userInputPackets[] = {
    {USR_IN_NONE,               {0,0}},
    {USR_IN_PING,               "pG"}, 
    {USR_IN_UPDATE_CONFIG,      "uC"}, 
    {USR_IN_UPDATE_PARAM,       "uP"}, 
    {USR_IN_UPDATE_ALL,         "uA"}, 
    {USR_IN_SAVE_CONFIG,        "sC"}, 
    {USR_IN_GET_CONFIG,         "gC"}, 
    {USR_IN_GET_PARAM,          "gP"}, 
    {USR_IN_GET_ALL,            "gA"}, 
    {USR_IN_GET_VERSION,        "gV"}, 
    {USR_IN_RESET,              "rS"}, 
// place new input packet code here, before USR_IN_MAX
    {USR_IN_MAX,                {0xff, 0xff}},   //  "" 
};


// packet codes here should be unique - 
// should not overlap codes for input packets and system packets
// First byte of Packet code should have value  >= 0x61  
usr_packet_t userOutputPackets[] = {	
//   Packet Type                Packet Code
    {USR_OUT_NONE,              {0x00, 0x00}},
    {USR_OUT_TEST,              "zT"},
    {USR_OUT_DATA1,             "z1"},
    {USR_OUT_DATA2,             "z2"},
// place new type and code here
    {USR_OUT_SCALED1,           "s1"},
    {USR_OUT_ANG1,              "a1"},   
    {USR_OUT_ANG2,              "a2"},   
    {USR_OUT_AID1,              "d1"},
    {USR_OUT_AID2,              "d2"},
    {USR_OUT_AID3,              "d3"},
    {USR_OUT_MAX,               {0xff, 0xff}},   //  "" 
};

volatile char   *info;
static   int    _userPayloadLen = 0;
static   int    _outputPacketType  = USR_OUT_MAX;
static   int    _inputPacketType   = USR_IN_MAX;


/*******************************************
 * @brief 
 * 
 * @param receivedCode ==
 * @return int 
********************************************/
int32_t CheckUserPacketType(uint16_t receivedCode)
{
    int res     = UCB_ERROR_INVALID_TYPE;
    usr_packet_t *packet  = &userInputPackets[1];
    uint16_t code;

    // validate packet code here and memorise for further processing
    while(packet->packetType != USR_IN_MAX){
        code = (packet->packetCode[0] << 8) | packet->packetCode[1];
        if(code == receivedCode){
            _inputPacketType = packet->packetType;
            return UCB_USER_IN; 
        }
        packet++;
    }

    packet  = &userOutputPackets[1];
    
    // validate packet code here and memorize for further processing
    while(packet->packetType != USR_OUT_MAX){
        code = (packet->packetCode[0] << 8) | packet->packetCode[1];
        if(code == receivedCode){
            _outputPacketType = packet->packetType;
            return UCB_USER_OUT; 
        }
        packet++;
    }

    return res; 
}


void   UserPacketTypeToBytes(uint8_t bytes[])
{
    if(_inputPacketType && _inputPacketType <  USR_IN_MAX){
        // response to request. Return same packet code
        bytes[0] = userInputPackets[_inputPacketType].packetCode[0];
        bytes[1] = userInputPackets[_inputPacketType].packetCode[1];
        _inputPacketType = USR_IN_MAX;  // wait for next input packet
        return;
    }
    
    if(_outputPacketType && _outputPacketType < USR_OUT_MAX){
        // continuous packet
        bytes[0] = userOutputPackets[_outputPacketType].packetCode[0];
        bytes[1] = userOutputPackets[_outputPacketType].packetCode[1];
    }else {
        bytes[0] = 0;
        bytes[1] = 0;
    }

}


/** ***************************************************************************
 * @name setUserPacketType - set user output packet type 
 * @brief
 * @param [in] packet type
 * @retval  - TRUE if success, FALSE otherwise
 ******************************************************************************/
BOOL SetUserPacketType(uint8_t *data, BOOL fApply)
{
    int type = -1;
    uint16_t *code = (uint16_t*)data;
    uint16_t tmp;
    BOOL result = TRUE;

    usr_packet_t *packet = &userOutputPackets[1];
    for(int i = 0; i < USR_OUT_MAX; i++, packet++){
        if(*code == *((uint16_t*)packet->packetCode)){
            type = packet->packetType;
            break;
        }
    }

    switch(type){
        case USR_OUT_TEST:              // simple test packet to check communication
            _outputPacketType = type;
            _userPayloadLen   = USR_OUT_TEST_PAYLOAD_LEN;
            break;
        case USR_OUT_DATA1:            // packet with sensors data. Change at will
            _outputPacketType = type;
            _userPayloadLen   = USR_OUT_DATA1_PAYLOAD_LEN;
            break;
        case USR_OUT_DATA2:            // packet with arbitrary data
            _outputPacketType = type;
            _userPayloadLen   = sizeof(data2_payload_t);
            break;
        case USR_OUT_SCALED1:          // packet with arbitrary data
            _outputPacketType = type;
            _userPayloadLen   = USR_OUT_SCALED1_PAYLOAD_LEN;
        case USR_OUT_ANG1:            // packet with sensors data. Change at will
            _outputPacketType = type;
            _userPayloadLen   = USR_OUT_ANG1_PAYLOAD_LEN;
            break;
        case USR_OUT_ANG2:            // packet with sensors data. Change at will
            _outputPacketType = type;
            _userPayloadLen   = USR_OUT_ANG2_PAYLOAD_LEN;
            break;
        case USR_OUT_AID1:
            _outputPacketType = type;
            _userPayloadLen   = USR_OUT_AID1_PAYLOAD_LEN;
            break;
        case USR_OUT_AID2:
            _outputPacketType = type;
            _userPayloadLen   = USR_OUT_AID2_PAYLOAD_LEN;
            break;
        case USR_OUT_AID3:
            _outputPacketType = type;
            _userPayloadLen   = USR_OUT_AID3_PAYLOAD_LEN;
            break;
        default:
            result = FALSE;
            break; 
    }

    if(result == FALSE){
        return FALSE;
    }

    tmp = (data[0] << 8) | data[1];

    result = config_SetOutputPacketCode(tmp, fApply);

    return result;
}


/** ***************************************************************************
 * @name getUserPayloadLength - get user payload length for sanity check 
 * @brief
 *
 * @retval  - user payload length
 ******************************************************************************/
int32_t GetUserPayloadLength(void)
{
    // ATTENTION: return actual user payload length, if user packet used    
    return _userPayloadLen;
}

/*******************************************
 * @brief 
 * 
 * @param ptrUcbPacket ==
 * @return int 
********************************************/
int32_t HandleUserInputPacket(UcbPacketStruct *ptrUcbPacket)
{
    BOOL valid = TRUE;
    int ret = USER_PACKET_OK;

    // Fill existing data before update
    BackFillUartDataStructure();

    /// call appropriate function based on packet type
	switch (_inputPacketType) {
		case USR_IN_RESET:
            HW_SystemReset();
            break;
		case USR_IN_PING:
            {
                uint8_t len; 
                Fill_PingPacketPayload(ptrUcbPacket->payload, &len);
                ptrUcbPacket->payloadLength = len;
            }
            // leave all the same - it will be bounced back unchanged
            break;
		case USR_IN_GET_VERSION:
            {
                uint8_t len;
                Fill_VersionPacketPayload(ptrUcbPacket->payload, &len);
                ptrUcbPacket->payloadLength = len;
            }
            break;
		case USR_IN_SAVE_CONFIG:
            // payload length does not change
             if(!SaveEcuSettings(FALSE, TRUE)){
                valid = FALSE;
             }
             break;
		case USR_IN_UPDATE_CONFIG:
             UpdateUserUartConfig((userConfigPayload*)ptrUcbPacket->payload, &ptrUcbPacket->payloadLength);
             break;
		case USR_IN_UPDATE_PARAM:
             UpdateUserUartParam((userParamPayload*)ptrUcbPacket->payload, &ptrUcbPacket->payloadLength);
             break;
		case USR_IN_UPDATE_ALL:
             UpdateAllUserUartParams((allUserParamsPayload*)ptrUcbPacket->payload, &ptrUcbPacket->payloadLength);
             break;
        case USR_IN_GET_CONFIG:
             if(!GetUserUartConfig((userConfigPayload*)ptrUcbPacket->payload, &ptrUcbPacket->payloadLength)){
                valid = FALSE;
             }
             break;
        case USR_IN_GET_PARAM:
             if(!GetUserUartParam((userParamPayload*)ptrUcbPacket->payload, &ptrUcbPacket->payloadLength)){
                valid = FALSE;
             }
             break;
        case USR_IN_GET_ALL:
             if(!GetAllUserUartParams((allUserParamsPayload*)ptrUcbPacket->payload, &ptrUcbPacket->payloadLength)){
                valid = FALSE;
             }
             break;
        default:
             /// default handler - unknown packet
             valid = FALSE;
             break; 
        }

        if(!valid){
             ptrUcbPacket->payloadLength = 0;
             ret = USER_PACKET_ERROR;
        }

        ptrUcbPacket->packetType = UCB_USER_OUT;    // do not remove - done for proper packet routing
        
        return ret;
}


/******************************************************************************
 * @name HandleUserOutputPacket - API call ro prepare continuous user output packet
 * @brief general handler
 * @param [in] payload pointer to put user data to
 * @param [in/out] number of bytes in user payload
 * @retval N/A
 ******************************************************************************/
BOOL HandleUserOutputPacket(uint8_t *payload, uint8_t *payloadLen)
{
    static uint32_t _testVal  = 0;
    static uint64_t ppsTstamp = 0; 
    BOOL ret = TRUE;

	switch (_outputPacketType) {
        case USR_OUT_TEST:
            {
                uint32_t *testParam = (uint32_t*)(payload);
                *payloadLen = USR_OUT_TEST_PAYLOAD_LEN;
                *testParam  = _testVal++;
            }
            break;

        case USR_OUT_DATA1:
            {
                uint8_t len;
                Fill_z1PacketPayload(payload, &len);
                *payloadLen = len;
            }
            break;
        case USR_OUT_DATA2:
            {   
                data2_payload_t *pld = (data2_payload_t *)payload;  
                pld->timer  = TIMER_GetCurrTimeStamp()/1000;      // in miliseconds
                pld->c  = 'A';
                pld->s  = 1234;
                pld->i  = -5;
                ppsTstamp = TIMER_GetCurrTimeStamp()/1000000;      // in seconds
                pld->ll = ppsTstamp;                        // time stamp of last PPS edge in microseconds from system start
                pld->d  = 1.23456789;
                *payloadLen = sizeof(data2_payload_t);
            }
            break;
        case USR_OUT_SCALED1:
            {
                uint8_t len;
                Fill_s1PacketPayload(payload, &len);
                *payloadLen = len;
            }
            break;
        case USR_OUT_ANG1:
            {
                // Variables used to hold the EKF values
                uint8_t len;
                Fill_a1PacketPayload(payload, &len);
                *payloadLen = len;
            }
            break;
        case USR_OUT_ANG2:
            {
                uint8_t len;

                Fill_a2PacketPayload(payload, &len);
                *payloadLen = len;
            }
            break;
        // place additional user packet preparing calls here
        // case USR_OUT_XXXX:
        //      *payloadLen = YYYY; // total user payload length, including user packet type
        //      payload[0]  = ZZZZ; // user packet type 
        //      prepare dada here
        //      break;

        case USR_OUT_AID1:
        {
            uint8_t len;
            Fill_d1PacketPayload(payload, &len);
            *payloadLen = len;
            break;
        }
        case USR_OUT_AID2:
        {
            uint8_t len;
            Fill_d2PacketPayload(payload, &len);
            *payloadLen = len;
            break;
        }
        case USR_OUT_AID3:
        {
            uint8_t len;
            Fill_d3PacketPayload(payload, &len);
            *payloadLen = len;
            break;
        }
        default:
            {
             *payloadLen = 0;  
             ret         = FALSE;
            }
             break;      /// unknown user packet, will send error in response
        }

        return ret;
}

