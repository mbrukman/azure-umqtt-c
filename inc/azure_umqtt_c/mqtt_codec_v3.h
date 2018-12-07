// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef MQTT_CODEC_H
#define MQTT_CODEC_H

#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/buffer_.h"
#include "azure_c_shared_utility/umock_c_prod.h"
#include "azure_umqtt_c/mqttconst.h"
#include "azure_umqtt_c/mqtt_codec_info.h"
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

typedef struct MQTTCODEC_INSTANCE_TAG* MQTTCODEC_HANDLE;

extern const CODEC_PROVIDER* mqtt_codec_v3_get_provider(void);

MOCKABLE_FUNCTION(, MQTT_CODEC_HANDLE, codec_v3_create, ON_PACKET_COMPLETE_CALLBACK, packetComplete, void*, callbackCtx);
MOCKABLE_FUNCTION(, void, codec_v3_destroy, MQTT_CODEC_HANDLE, handle);
MOCKABLE_FUNCTION(, BUFFER_HANDLE, codec_v3_connect, MQTT_CODEC_HANDLE, handle, const MQTT_CLIENT_OPTIONS*, mqttOptions);
MOCKABLE_FUNCTION(, BUFFER_HANDLE, codec_v3_disconnect, MQTT_CODEC_HANDLE, handle);
MOCKABLE_FUNCTION(, BUFFER_HANDLE, codec_v3_publish, MQTT_CODEC_HANDLE, handle, QOS_VALUE, qosValue, bool, duplicateMsg, bool, serverRetain, uint16_t, packetId, const char*, topicName, const uint8_t*, msgBuffer, size_t, buffLen);
MOCKABLE_FUNCTION(, BUFFER_HANDLE, codec_v3_publishAck, uint16_t, packetId);
MOCKABLE_FUNCTION(, BUFFER_HANDLE, codec_v3_publishReceived, uint16_t, packetId);
MOCKABLE_FUNCTION(, BUFFER_HANDLE, codec_v3_publishRelease, uint16_t, packetId);
MOCKABLE_FUNCTION(, BUFFER_HANDLE, codec_v3_publishComplete, uint16_t, packetId);
MOCKABLE_FUNCTION(, BUFFER_HANDLE, codec_v3_ping);
MOCKABLE_FUNCTION(, BUFFER_HANDLE, codec_v3_subscribe, MQTT_CODEC_HANDLE, handle, uint16_t, packetId, SUBSCRIBE_PAYLOAD*, subscribeList, size_t, count);
MOCKABLE_FUNCTION(, BUFFER_HANDLE, codec_v3_unsubscribe, MQTT_CODEC_HANDLE, handle, uint16_t, packetId, const char**, unsubscribeList, size_t, count);
MOCKABLE_FUNCTION(, int, codec_v3_set_trace, MQTT_CODEC_HANDLE, handle, TRACE_LOG_VALUE, trace_func, void*, trace_ctx);
MOCKABLE_FUNCTION(, ON_BYTES_RECEIVED, codec_v3_get_recv_func);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // MQTT_CODEC_H
