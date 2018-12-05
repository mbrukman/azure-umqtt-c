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

#include "azure_umqtt_c/mqtt_codec_v5.h"
#include "azure_umqtt_c/mqtt_codec_info.h"

#include <inttypes.h>

#define PAYLOAD_OFFSET                      5

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

typedef struct CODEC_V5_INSTANCE_TAG
{
    CONTROL_PACKET_TYPE currPacket;
    TRACE_LOG_VALUE trace_func;
    void* trace_ctx;
} CODEC_V5_INSTANCE;

typedef struct PUBLISH_HEADER_INFO_TAG
{
    const char* topicName;
    uint16_t packetId;
    const char* msgBuffer;
    QOS_VALUE qualityOfServiceValue;
} PUBLISH_HEADER_INFO;

static void on_bytes_recv(void* context, const unsigned char* buffer, size_t size)
{
    if (context == NULL || buffer == NULL || size == 0)
    {
        LogError("Error: mqtt_client is NULL");
        //set_error_callback(mqtt_client, MQTT_CLIENT_PARSE_ERROR);
    }
    else
    {
        //CODEC_V5_INSTANCE* codec_data = (CODEC_V5_INSTANCE*)context;
    }
}

static MQTT_CODEC_HANDLE codec_v5_create(ON_PACKET_COMPLETE_CALLBACK on_packet_complete_cb, void* context)
{
    (void)on_packet_complete_cb;
    (void)context;
    CODEC_V5_INSTANCE* result;
    if ((result = malloc(sizeof(CODEC_V5_INSTANCE))) == NULL)
    {
        /* Codes_SRS_MQTT_CODEC_07_001: [If a failure is encountered then codec_v5_create shall return NULL.] */
        LogError("Failure allocating codec");
    }
    else
    {
        result->currPacket = UNKNOWN_TYPE;
    }
    return (MQTT_CODEC_HANDLE)result;
}

static void codec_v5_destroy(MQTT_CODEC_HANDLE handle)
{
    /* Codes_SRS_MQTT_CODEC_07_003: [If the handle parameter is NULL then codec_v5_destroy shall do nothing.] */
    if (handle != NULL)
    {
        CODEC_V5_INSTANCE* codec_data = (CODEC_V5_INSTANCE*)handle;
        /* Codes_SRS_MQTT_CODEC_07_004: [mqtt_codec_destroy shall deallocate all memory that has been allocated by this object.] */
        free(codec_data);
    }
}

BUFFER_HANDLE codec_v5_connect(MQTT_CODEC_HANDLE handle, const MQTT_CLIENT_OPTIONS* mqttOptions)
{
    BUFFER_HANDLE result;
    /* Codes_SRS_MQTT_CODEC_07_008: [If the parameters mqttOptions is NULL then codec_v5_connect shall return a null value.] */
    if (mqttOptions == NULL || handle == NULL)
    {
        result = NULL;
    }
    else
    {
        result = NULL;
    }
    return result;
}

BUFFER_HANDLE codec_v5_disconnect(MQTT_CODEC_HANDLE handle)
{
    (void)handle;
    /* Codes_SRS_MQTT_CODEC_07_011: [On success codec_v5_disconnect shall construct a BUFFER_HANDLE that represents a MQTT DISCONNECT packet.] */
    BUFFER_HANDLE result = BUFFER_new();
    if (result != NULL)
    {
    }
    return result;
}

BUFFER_HANDLE codec_v5_publish(MQTT_CODEC_HANDLE handle, QOS_VALUE qosValue, bool duplicateMsg, bool serverRetain, uint16_t packetId, const char* topicName, const uint8_t* msgBuffer, size_t buffLen)
{
    (void)qosValue;
    (void)duplicateMsg;
    (void)serverRetain;
    (void)packetId;
    (void)topicName;
    (void)msgBuffer;
    (void)buffLen;
    BUFFER_HANDLE result;
    /* Codes_SRS_MQTT_CODEC_07_005: [If the parameters topicName is NULL then codec_v5_publish shall return NULL.] */
    if (handle == NULL || topicName == NULL)
    {
        result = NULL;
    }
    /* Codes_SRS_MQTT_CODEC_07_036: [mqtt_codec_publish shall return NULL if the buffLen variable is greater than the MAX_SEND_SIZE (0xFFFFFF7F).] */
    else if (buffLen > MAX_SEND_SIZE)
    {
        /* Codes_SRS_MQTT_CODEC_07_006: [If any error is encountered then codec_v5_publish shall return NULL.] */
        result = NULL;
    }
    else
    {
        result = 0;
    }
    return result;
}

BUFFER_HANDLE codec_v5_publishAck(uint16_t packetId)
{
    (void)packetId;
    /* Codes_SRS_MQTT_CODEC_07_013: [On success codec_v5_publishAck shall return a BUFFER_HANDLE representation of a MQTT PUBACK packet.] */
    /* Codes_SRS_MQTT_CODEC_07_014 : [If any error is encountered then codec_v5_publishAck shall return NULL.] */
    BUFFER_HANDLE result = NULL;//constructPublishReply(PUBACK_TYPE, 0, packetId);
    return result;
}

BUFFER_HANDLE codec_v5_publishReceived(uint16_t packetId)
{
    (void)packetId;
    /* Codes_SRS_MQTT_CODEC_07_015: [On success codec_v5_publishRecieved shall return a BUFFER_HANDLE representation of a MQTT PUBREC packet.] */
    /* Codes_SRS_MQTT_CODEC_07_016 : [If any error is encountered then codec_v5_publishRecieved shall return NULL.] */
    BUFFER_HANDLE result = NULL;//constructPublishReply(PUBREC_TYPE, 0, packetId);
    return result;
}

BUFFER_HANDLE codec_v5_publishRelease(uint16_t packetId)
{
    (void)packetId;
    /* Codes_SRS_MQTT_CODEC_07_017: [On success codec_v5_publishRelease shall return a BUFFER_HANDLE representation of a MQTT PUBREL packet.] */
    /* Codes_SRS_MQTT_CODEC_07_018 : [If any error is encountered then codec_v5_publishRelease shall return NULL.] */
    BUFFER_HANDLE result = NULL;//constructPublishReply(PUBREL_TYPE, 2, packetId);
    return result;
}

BUFFER_HANDLE codec_v5_publishComplete(uint16_t packetId)
{
    (void)packetId;
    /* Codes_SRS_MQTT_CODEC_07_019: [On success codec_v5_publishComplete shall return a BUFFER_HANDLE representation of a MQTT PUBCOMP packet.] */
    /* Codes_SRS_MQTT_CODEC_07_020 : [If any error is encountered then codec_v5_publishComplete shall return NULL.] */
    BUFFER_HANDLE result = NULL;//constructPublishReply(PUBCOMP_TYPE, 0, packetId);
    return result;
}

BUFFER_HANDLE codec_v5_ping(void)
{
    /* Codes_SRS_MQTT_CODEC_07_021: [On success codec_v5_ping shall construct a BUFFER_HANDLE that represents a MQTT PINGREQ packet.] */
    BUFFER_HANDLE result = BUFFER_new();
    if (result != NULL)
    {
    }
    return result;
}

BUFFER_HANDLE codec_v5_subscribe(MQTT_CODEC_HANDLE handle, uint16_t packetId, SUBSCRIBE_PAYLOAD* subscribeList, size_t count)
{
    (void)packetId;
    (void)subscribeList;
    (void)count;
    (void)handle;
    BUFFER_HANDLE result;
    /* Codes_SRS_MQTT_CODEC_07_023: [If the parameters subscribeList is NULL or if count is 0 then codec_v5_subscribe shall return NULL.] */
    if (subscribeList == NULL || count == 0)
    {
        result = NULL;
    }
    else
    {
        /* Codes_SRS_MQTT_CODEC_07_026: [mqtt_codec_subscribe shall return a BUFFER_HANDLE that represents a MQTT SUBSCRIBE message.]*/
        result = BUFFER_new();
        if (result != NULL)
        {
        }
    }
    return result;
}

BUFFER_HANDLE codec_v5_unsubscribe(MQTT_CODEC_HANDLE handle, uint16_t packetId, const char** unsubscribeList, size_t count)
{
    (void)packetId;
    (void)unsubscribeList;
    (void)count;
    (void)handle;
    BUFFER_HANDLE result;
    /* Codes_SRS_MQTT_CODEC_07_027: [If the parameters unsubscribeList is NULL or if count is 0 then mqtt_codec_unsubscribe shall return NULL.] */
    if (unsubscribeList == NULL || count == 0)
    {
        result = NULL;
    }
    else
    {
        result = NULL;
    }
    return result;
}

ON_BYTES_RECEIVED codec_v5_get_recv_func(void)
{
    return on_bytes_recv;
}

static int codec_v5_set_trace(MQTT_CODEC_HANDLE handle, TRACE_LOG_VALUE trace_func, void* trace_ctx)
{
    int result;
    if (handle == NULL)
    {
        result = __FAILURE__;
    }
    else
    {
        CODEC_V5_INSTANCE* codec_data = (CODEC_V5_INSTANCE*)handle;
        codec_data->trace_func = trace_func;
        codec_data->trace_ctx = trace_ctx;
        result = 0;
    }
    return result;
}

static CODEC_PROVIDER codec_provider = 
{
    codec_v5_create,
    codec_v5_destroy,
    codec_v5_connect,
    codec_v5_disconnect,
    codec_v5_publish,
    codec_v5_publishAck,
    codec_v5_publishReceived,
    codec_v5_publishRelease,
    codec_v5_publishComplete,
    codec_v5_ping,
    codec_v5_subscribe,
    codec_v5_unsubscribe,
    codec_v5_get_recv_func,
    codec_v5_set_trace
};

const CODEC_PROVIDER* mqtt_codec_v5_get_provider(void)
{
    return &codec_provider;
}
