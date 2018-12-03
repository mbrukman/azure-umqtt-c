// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef MQTT_CODEC_INFO_H
#define MQTT_CODEC_INFO_H

#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/buffer_.h"
#include "azure_umqtt_c/mqttconst.h"
#include "azure_c_shared_utility/strings.h"

#ifdef __cplusplus
#include <cstdint>
#include <cstddef>
extern "C" {
#else
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#endif // __cplusplus

typedef void* MQTT_CODEC_HANDLE;

typedef void(*ON_PACKET_COMPLETE_CALLBACK)(void* context, CONTROL_PACKET_TYPE packet, int flags, BUFFER_HANDLE headerData);
typedef void(*TRACE_LOG_VALUE)(void* context, const char* log_value, ...);


typedef MQTT_CODEC_HANDLE(*pmqtt_codec_create)(ON_PACKET_COMPLETE_CALLBACK packetComplete, void* cb_ctx);
typedef void(*pmqtt_codec_destroy)(MQTT_CODEC_HANDLE handle);

typedef BUFFER_HANDLE(*pmqtt_codec_connect)(MQTT_CODEC_HANDLE handle, const MQTT_CLIENT_OPTIONS* mqttOptions);
typedef BUFFER_HANDLE(*pmqtt_codec_disconnect)(MQTT_CODEC_HANDLE handle);
typedef BUFFER_HANDLE(*pmqtt_codec_publish)(QOS_VALUE qosValue, bool duplicateMsg, bool serverRetain, uint16_t packetId, const char* topicName, const uint8_t* msgBuffer, size_t buffLen, STRING_HANDLE trace_log);
typedef BUFFER_HANDLE(*pmqtt_codec_publishAck)(uint16_t packetId);
typedef BUFFER_HANDLE(*pmqtt_codec_publishReceived)(uint16_t packetId);
typedef BUFFER_HANDLE(*pmqtt_codec_publishRelease)(uint16_t packetId);
typedef BUFFER_HANDLE(*pmqtt_codec_publishComplete)(uint16_t packetId);
typedef BUFFER_HANDLE(*pmqtt_codec_ping)(void);
typedef BUFFER_HANDLE(*pmqtt_codec_subscribe)(uint16_t packetId, SUBSCRIBE_PAYLOAD* subscribeList, size_t count, STRING_HANDLE trace_log);
typedef BUFFER_HANDLE(*pmqtt_codec_unsubscribe)(uint16_t packetId, const char** unsubscribeList, size_t count, STRING_HANDLE trace_log);
typedef ON_BYTES_RECEIVED(*pmqtt_codec_get_recv_func)(void);
typedef int(*pmqtt_codec_set_trace)(MQTT_CODEC_HANDLE handle, TRACE_LOG_VALUE trace_func, void* trace_ctx);

typedef struct CODEC_PROVIDER_TAG
{
    pmqtt_codec_create mqtt_codec_create;
    pmqtt_codec_destroy mqtt_codec_destroy;
    pmqtt_codec_connect mqtt_codec_connect;
    pmqtt_codec_disconnect mqtt_codec_disconnect;
    pmqtt_codec_publish mqtt_codec_publish;
    pmqtt_codec_publishAck mqtt_codec_publishAck;
    pmqtt_codec_publishReceived mqtt_codec_publishReceived;
    pmqtt_codec_publishRelease mqtt_codec_publishRelease;
    pmqtt_codec_publishComplete mqtt_codec_publishComplete;
    pmqtt_codec_ping mqtt_codec_ping;
    pmqtt_codec_subscribe mqtt_codec_subscribe;
    pmqtt_codec_unsubscribe mqtt_codec_unsubscribe;
    pmqtt_codec_get_recv_func mqtt_codec_get_recv_func;
    pmqtt_codec_set_trace mqtt_codec_set_trace;
} CODEC_PROVIDER;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // MQTT_CODEC_INFO_H
