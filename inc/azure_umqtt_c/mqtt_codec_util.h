// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef MQTT_CODEC_UTIL_H
#define MQTT_CODEC_UTIL_H

#ifdef __cplusplus
#include <cstdint>
#include <cstddef>
extern "C" {
#else
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#endif // __cplusplus

#include "azure_umqtt_c/mqttconst.h"

extern const char* retrieve_qos_value(QOS_VALUE value);
extern void byteutil_writeByte(uint8_t** buffer, uint8_t value);
extern void byteutil_writeInt(uint8_t** buffer, uint16_t value);
extern void byteutil_writeUTF(uint8_t** buffer, const char* stringData, uint16_t len);
extern CONTROL_PACKET_TYPE processControlPacketType(uint8_t pktByte, int* flags);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // MQTT_CODEC_UTIL_H