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

MOCKABLE_FUNCTION(, BUFFER_HANDLE, mqtt_codec_publishAck, uint16_t, packetId);
MOCKABLE_FUNCTION(, BUFFER_HANDLE, mqtt_codec_publishReceived, uint16_t, packetId);
MOCKABLE_FUNCTION(, BUFFER_HANDLE, mqtt_codec_publishRelease, uint16_t, packetId);
MOCKABLE_FUNCTION(, BUFFER_HANDLE, mqtt_codec_publishComplete, uint16_t, packetId);
MOCKABLE_FUNCTION(, BUFFER_HANDLE, mqtt_codec_ping);

extern const CODEC_PROVIDER* mqtt_codec_v3_get_provider(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // MQTT_CODEC_H
