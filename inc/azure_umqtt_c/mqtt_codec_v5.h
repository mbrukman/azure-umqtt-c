// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef MQTT_CODEC_V5_H
#define MQTT_CODEC_V5_H

#include "azure_c_shared_utility/umock_c_prod.h"
#include "azure_umqtt_c/mqtt_codec_info.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

extern const CODEC_PROVIDER* mqtt_codec_v5_get_provider(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // MQTT_CODEC_V5_H
