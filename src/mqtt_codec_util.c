// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <limits.h>
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/buffer_.h"
#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/macro_utils.h"
#include "azure_c_shared_utility/xlogging.h"

#include "azure_umqtt_c/mqtt_codec_util.h"
#include "azure_umqtt_c/mqtt_codec_info.h"

#include <inttypes.h>

#define PAYLOAD_OFFSET                      5
#define PACKET_TYPE_BYTE(p)                 (CONTROL_PACKET_TYPE)((uint8_t)(((uint8_t)(p)) & 0xf0))
#define FLAG_VALUE_BYTE(p)                  ((uint8_t)(((uint8_t)(p)) & 0xf))

#define USERNAME_FLAG                       0x80
#define PASSWORD_FLAG                       0x40
#define WILL_RETAIN_FLAG                    0x20
#define WILL_QOS_FLAG_                      0x18
#define WILL_FLAG_FLAG                      0x04
#define CLEAN_SESSION_FLAG                  0x02

#define NEXT_128_CHUNK                      0x80
#define PUBLISH_DUP_FLAG                    0x8
#define PUBLISH_QOS_EXACTLY_ONCE            0x4
#define PUBLISH_QOS_AT_LEAST_ONCE           0x2
#define PUBLISH_QOS_RETAIN                  0x1

#define PROTOCOL_NUMBER                     4
#define CONN_FLAG_BYTE_OFFSET               7

#define CONNECT_FIXED_HEADER_SIZE           2
#define CONNECT_VARIABLE_HEADER_SIZE        10
#define SUBSCRIBE_FIXED_HEADER_FLAG         0x2
#define UNSUBSCRIBE_FIXED_HEADER_FLAG       0x2

#define MAX_SEND_SIZE                       0xFFFFFF7F

#define CODEC_STATE_VALUES      \
    CODEC_STATE_FIXED_HEADER,   \
    CODEC_STATE_VAR_HEADER,     \
    CODEC_STATE_PAYLOAD

static const char* const TRUE_CONST = "true";
static const char* const FALSE_CONST = "false";

DEFINE_ENUM(CODEC_STATE_RESULT, CODEC_STATE_VALUES);

static const char* retrieve_qos_value(QOS_VALUE value)
{
    switch (value)
    {
        case DELIVER_AT_MOST_ONCE:
            return "DELIVER_AT_MOST_ONCE";
        case DELIVER_AT_LEAST_ONCE:
            return "DELIVER_AT_LEAST_ONCE";
        case DELIVER_EXACTLY_ONCE:
        default:
            return "DELIVER_EXACTLY_ONCE";
    }
}

static void byteutil_writeByte(uint8_t** buffer, uint8_t value)
{
    if (buffer != NULL)
    {
        **buffer = value;
        (*buffer)++;
    }
}

static void byteutil_writeInt(uint8_t** buffer, uint16_t value)
{
    if (buffer != NULL)
    {
        **buffer = (char)(value / 256);
        (*buffer)++;
        **buffer = (char)(value % 256);
        (*buffer)++;
    }
}

static void byteutil_writeUTF(uint8_t** buffer, const char* stringData, uint16_t len)
{
    if (buffer != NULL)
    {
        byteutil_writeInt(buffer, len);
        (void)memcpy(*buffer, stringData, len);
        *buffer += len;
    }
}

static CONTROL_PACKET_TYPE processControlPacketType(uint8_t pktByte, int* flags)
{
    CONTROL_PACKET_TYPE result;
    result = PACKET_TYPE_BYTE(pktByte);
    if (flags != NULL)
    {
        *flags = FLAG_VALUE_BYTE(pktByte);
    }
    return result;
}

static int addListItemsToUnsubscribePacket(BUFFER_HANDLE ctrlPacket, const char** payloadList, size_t payloadCount, STRING_HANDLE trace_log)
{
    int result = 0;
    if (payloadList == NULL || ctrlPacket == NULL)
    {
        result = __FAILURE__;
    }
    else
    {
        size_t index = 0;
        for (index = 0; index < payloadCount && result == 0; index++)
        {
            // Add the Payload
            size_t offsetLen = BUFFER_length(ctrlPacket);
            size_t topicLen = strlen(payloadList[index]);
            if (topicLen > USHRT_MAX)
            {
                result = __FAILURE__;
            }
            else if (BUFFER_enlarge(ctrlPacket, topicLen + 2) != 0)
            {
                result = __FAILURE__;
            }
            else
            {
                uint8_t* iterator = BUFFER_u_char(ctrlPacket);
                iterator += offsetLen;
                byteutil_writeUTF(&iterator, payloadList[index], (uint16_t)topicLen);
            }
            if (trace_log != NULL)
            {
                STRING_sprintf(trace_log, " | TOPIC_NAME: %s", payloadList[index]);
            }
        }
    }
    return result;
}

static int addListItemsToSubscribePacket(BUFFER_HANDLE ctrlPacket, SUBSCRIBE_PAYLOAD* payloadList, size_t payloadCount, STRING_HANDLE trace_log)
{
    int result = 0;
    if (payloadList == NULL || ctrlPacket == NULL)
    {
        result = __FAILURE__;
    }
    else
    {
        size_t index = 0;
        for (index = 0; index < payloadCount && result == 0; index++)
        {
            // Add the Payload
            size_t offsetLen = BUFFER_length(ctrlPacket);
            size_t topicLen = strlen(payloadList[index].subscribeTopic);
            if (topicLen > USHRT_MAX)
            {
                result = __FAILURE__;
            }
            else if (BUFFER_enlarge(ctrlPacket, topicLen + 2 + 1) != 0)
            {
                result = __FAILURE__;
            }
            else
            {
                uint8_t* iterator = BUFFER_u_char(ctrlPacket);
                iterator += offsetLen;
                byteutil_writeUTF(&iterator, payloadList[index].subscribeTopic, (uint16_t)topicLen);
                *iterator = payloadList[index].qosReturn;

                if (trace_log != NULL)
                {
                    STRING_sprintf(trace_log, " | TOPIC_NAME: %s | QOS: %d", payloadList[index].subscribeTopic, (int)payloadList[index].qosReturn);
                }
            }
        }
    }
    return result;
}

static int constructConnectVariableHeader(BUFFER_HANDLE ctrlPacket, const MQTT_CLIENT_OPTIONS* mqttOptions, STRING_HANDLE trace_log)
{
    int result = 0;
    if (BUFFER_enlarge(ctrlPacket, CONNECT_VARIABLE_HEADER_SIZE) != 0)
    {
        result = __FAILURE__;
    }
    else
    {
        uint8_t* iterator = BUFFER_u_char(ctrlPacket);
        if (iterator == NULL)
        {
            result = __FAILURE__;
        }
        else
        {
            if (trace_log != NULL)
            {
                STRING_sprintf(trace_log, " | VER: %d | KEEPALIVE: %d | FLAGS:", PROTOCOL_NUMBER, mqttOptions->keepAliveInterval);
            }
            byteutil_writeUTF(&iterator, "MQTT", 4);
            byteutil_writeByte(&iterator, PROTOCOL_NUMBER);
            byteutil_writeByte(&iterator, 0); // Flags will be entered later
            byteutil_writeInt(&iterator, mqttOptions->keepAliveInterval);
            result = 0;
        }
    }
    return result;
}

static int constructPublishVariableHeader(BUFFER_HANDLE ctrlPacket, const PUBLISH_HEADER_INFO* publishHeader, STRING_HANDLE trace_log)
{
    int result = 0;
    size_t topicLen = 0;
    size_t spaceLen = 0;
    size_t idLen = 0;

    size_t currLen = BUFFER_length(ctrlPacket);

    topicLen = strlen(publishHeader->topicName);
    spaceLen += 2;

    if (publishHeader->qualityOfServiceValue != DELIVER_AT_MOST_ONCE)
    {
        // Packet Id is only set if the QOS is not 0
        idLen = 2;
    }

    if (topicLen > USHRT_MAX)
    {
        result = __FAILURE__;
    }
    else if (BUFFER_enlarge(ctrlPacket, topicLen + idLen + spaceLen) != 0)
    {
        result = __FAILURE__;
    }
    else
    {
        uint8_t* iterator = BUFFER_u_char(ctrlPacket);
        if (iterator == NULL)
        {
            result = __FAILURE__;
        }
        else
        {
            iterator += currLen;
            /* The Topic Name MUST be present as the first field in the PUBLISH Packet Variable header.It MUST be 792 a UTF-8 encoded string [MQTT-3.3.2-1] as defined in section 1.5.3.*/
            byteutil_writeUTF(&iterator, publishHeader->topicName, (uint16_t)topicLen);
            if (trace_log != NULL)
            {
                STRING_sprintf(trace_log, " | TOPIC_NAME: %s", publishHeader->topicName);
            }
            if (idLen > 0)
            {
                if (trace_log != NULL)
                {
                    STRING_sprintf(trace_log, " | PACKET_ID: %"PRIu16, publishHeader->packetId);
                }
                byteutil_writeInt(&iterator, publishHeader->packetId);
            }
            result = 0;
        }
    }
    return result;
}

static int constructSubscibeTypeVariableHeader(BUFFER_HANDLE ctrlPacket, uint16_t packetId)
{
    int result = 0;
    if (BUFFER_enlarge(ctrlPacket, 2) != 0)
    {
        result = __FAILURE__;
    }
    else
    {
        uint8_t* iterator = BUFFER_u_char(ctrlPacket);
        if (iterator == NULL)
        {
            result = __FAILURE__;
        }
        else
        {
            byteutil_writeInt(&iterator, packetId);
            result = 0;
        }
    }
    return result;
}

static BUFFER_HANDLE constructPublishReply(CONTROL_PACKET_TYPE type, uint8_t flags, uint16_t packetId)
{
    BUFFER_HANDLE result = BUFFER_new();
    if (result != NULL)
    {
        if (BUFFER_pre_build(result, 4) != 0)
        {
            BUFFER_delete(result);
            result = NULL;
        }
        else
        {
            uint8_t* iterator = BUFFER_u_char(result);
            if (iterator == NULL)
            {
                BUFFER_delete(result);
                result = NULL;
            }
            else
            {
                *iterator = (uint8_t)type | flags;
                iterator++;
                *iterator = 0x2;
                iterator++;
                byteutil_writeInt(&iterator, packetId);
            }
        }
    }
    return result;
}

static int constructFixedHeader(BUFFER_HANDLE ctrlPacket, CONTROL_PACKET_TYPE packetType, uint8_t flags)
{
    int result;
    if (ctrlPacket == NULL)
    {
        return __FAILURE__;
    }
    else
    {
        size_t packetLen = BUFFER_length(ctrlPacket);
        uint8_t remainSize[4] ={ 0 };
        size_t index = 0;

        // Calculate the length of packet
        do
        {
            uint8_t encode = packetLen % 128;
            packetLen /= 128;
            // if there are more data to encode, set the top bit of this byte
            if (packetLen > 0)
            {
                encode |= NEXT_128_CHUNK;
            }
            remainSize[index++] = encode;
        } while (packetLen > 0);

        BUFFER_HANDLE fixedHeader = BUFFER_new();
        if (fixedHeader == NULL)
        {
            result = __FAILURE__;
        }
        else if (BUFFER_pre_build(fixedHeader, index + 1) != 0)
        {
            BUFFER_delete(fixedHeader);
            result = __FAILURE__;
        }
        else
        {
            uint8_t* iterator = BUFFER_u_char(fixedHeader);
            *iterator = (uint8_t)packetType | flags;
            iterator++;
            (void)memcpy(iterator, remainSize, index);

            result = BUFFER_prepend(ctrlPacket, fixedHeader);
            BUFFER_delete(fixedHeader);
        }
    }
    return result;
}

static int constructConnPayload(BUFFER_HANDLE ctrlPacket, const MQTT_CLIENT_OPTIONS* mqttOptions, STRING_HANDLE trace_log)
{
    int result = 0;
    if (mqttOptions == NULL || ctrlPacket == NULL)
    {
        result = __FAILURE__;
    }
    else
    {
        size_t clientLen = 0;
        size_t usernameLen = 0;
        size_t passwordLen = 0;
        size_t willMessageLen = 0;
        size_t willTopicLen = 0;
        size_t spaceLen = 0;

        if (mqttOptions->clientId != NULL)
        {
            spaceLen += 2;
            clientLen = strlen(mqttOptions->clientId);
        }
        if (mqttOptions->username != NULL)
        {
            spaceLen += 2;
            usernameLen = strlen(mqttOptions->username);
        }
        if (mqttOptions->password != NULL)
        {
            spaceLen += 2;
            passwordLen = strlen(mqttOptions->password);
        }
        if (mqttOptions->willMessage != NULL)
        {
            spaceLen += 2;
            willMessageLen = strlen(mqttOptions->willMessage);
        }
        if (mqttOptions->willTopic != NULL)
        {
            spaceLen += 2;
            willTopicLen = strlen(mqttOptions->willTopic);
        }

        size_t currLen = BUFFER_length(ctrlPacket);
        size_t totalLen = clientLen + usernameLen + passwordLen + willMessageLen + willTopicLen + spaceLen;

        // Validate the Username & Password
        if (clientLen > USHRT_MAX)
        {
            result = __FAILURE__;
        }
        else if (usernameLen == 0 && passwordLen > 0)
        {
            result = __FAILURE__;
        }
        else if ((willMessageLen > 0 && willTopicLen == 0) || (willTopicLen > 0 && willMessageLen == 0))
        {
            result = __FAILURE__;
        }
        else if (BUFFER_enlarge(ctrlPacket, totalLen) != 0)
        {
            result = __FAILURE__;
        }
        else
        {
            uint8_t* packet = BUFFER_u_char(ctrlPacket);
            uint8_t* iterator = packet;

            iterator += currLen;
            byteutil_writeUTF(&iterator, mqttOptions->clientId, (uint16_t)clientLen);

            // TODO: Read on the Will Topic
            if (willMessageLen > USHRT_MAX || willTopicLen > USHRT_MAX || usernameLen > USHRT_MAX || passwordLen > USHRT_MAX)
            {
                result = __FAILURE__;
            }
            else
            {
                STRING_HANDLE connect_payload_trace = NULL;
                if (trace_log != NULL)
                {
                    connect_payload_trace = STRING_new();
                }
                if (willMessageLen > 0 && willTopicLen > 0)
                {
                    if (trace_log != NULL)
                    {
                        (void)STRING_sprintf(connect_payload_trace, " | WILL_TOPIC: %s", mqttOptions->willTopic);
                    }
                    packet[CONN_FLAG_BYTE_OFFSET] |= WILL_FLAG_FLAG;
                    byteutil_writeUTF(&iterator, mqttOptions->willTopic, (uint16_t)willTopicLen);
                    packet[CONN_FLAG_BYTE_OFFSET] |= (mqttOptions->qualityOfServiceValue << 3);
                    if (mqttOptions->messageRetain)
                    {
                        packet[CONN_FLAG_BYTE_OFFSET] |= WILL_RETAIN_FLAG;
                    }
                    byteutil_writeUTF(&iterator, mqttOptions->willMessage, (uint16_t)willMessageLen);
                }
                if (usernameLen > 0)
                {
                    packet[CONN_FLAG_BYTE_OFFSET] |= USERNAME_FLAG;
                    byteutil_writeUTF(&iterator, mqttOptions->username, (uint16_t)usernameLen);
                    if (trace_log != NULL)
                    {
                        (void)STRING_sprintf(connect_payload_trace, " | USERNAME: %s", mqttOptions->username);
                    }
                }
                if (passwordLen > 0)
                {
                    packet[CONN_FLAG_BYTE_OFFSET] |= PASSWORD_FLAG;
                    byteutil_writeUTF(&iterator, mqttOptions->password, (uint16_t)passwordLen);
                    if (trace_log != NULL)
                    {
                        (void)STRING_sprintf(connect_payload_trace, " | PWD: XXXX");
                    }
                }
                // TODO: Get the rest of the flags
                if (trace_log != NULL)
                {
                    (void)STRING_sprintf(connect_payload_trace, " | CLEAN: %s", mqttOptions->useCleanSession ? "1" : "0");
                }
                if (mqttOptions->useCleanSession)
                {
                    packet[CONN_FLAG_BYTE_OFFSET] |= CLEAN_SESSION_FLAG;
                }
                if (trace_log != NULL)
                {
                    (void)STRING_sprintf(trace_log, " %zu", packet[CONN_FLAG_BYTE_OFFSET]);
                    (void)STRING_concat_with_STRING(trace_log, connect_payload_trace);
                    STRING_delete(connect_payload_trace);
                }
                result = 0;
            }
        }
    }
    return result;
}

static int prepareheaderDataInfo(MQTTCODEC_INSTANCE* codecData, uint8_t remainLen)
{
    int result;
    if (codecData == NULL)
    {
        result = __FAILURE__;
    }
    else
    {
        result = 0;
        codecData->storeRemainLen[codecData->remainLenIndex++] = remainLen;
        if (remainLen <= 0x7f)
        {
            int multiplier = 1;
            int totalLen = 0;
            size_t index = 0;
            uint8_t encodeByte = 0;
            do
            {
                encodeByte = codecData->storeRemainLen[index++];
                totalLen += (encodeByte & 127) * multiplier;
                multiplier *= NEXT_128_CHUNK;

                if (multiplier > 128 * 128 * 128)
                {
                    result = __FAILURE__;
                    break;
                }
            } while ((encodeByte & NEXT_128_CHUNK) != 0);

            codecData->codecState = CODEC_STATE_VAR_HEADER;

            // Reset remainLen Index
            codecData->remainLenIndex = 0;
            memset(codecData->storeRemainLen, 0, 4 * sizeof(uint8_t));

            if (totalLen > 0)
            {
                codecData->bufferOffset = 0;
                codecData->headerData = BUFFER_new();
                if (codecData->headerData == NULL)
                {
                    /* Codes_SRS_MQTT_CODEC_07_035: [ If any error is encountered then the packet state will be marked as error and mqtt_codec_bytesReceived shall return a non-zero value. ] */
                    LogError("Failed BUFFER_new");
                    result = __FAILURE__;
                }
                else
                {
                    if (BUFFER_pre_build(codecData->headerData, totalLen) != 0)
                    {
                        /* Codes_SRS_MQTT_CODEC_07_035: [ If any error is encountered then the packet state will be marked as error and mqtt_codec_bytesReceived shall return a non-zero value. ] */
                        LogError("Failed BUFFER_pre_build");
                        result = __FAILURE__;
                    }

                }
            }
        }
    }
    return result;
}

static void completePacketData(MQTTCODEC_INSTANCE* codecData)
{
    if (codecData)
    {
        if (codecData->packetComplete != NULL)
        {
            codecData->packetComplete(codecData->callContext, codecData->currPacket, codecData->headerFlags, codecData->headerData);
        }

        // Clean up data
        codecData->currPacket = UNKNOWN_TYPE;
        codecData->codecState = CODEC_STATE_FIXED_HEADER;
        codecData->headerFlags = 0;
        BUFFER_delete(codecData->headerData);
        codecData->headerData = NULL;
    }
}

