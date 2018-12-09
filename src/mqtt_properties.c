// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/buffer_.h"

#include "azure_umqtt_c/mqtt_properties.h"

typedef struct MQTT_PROP_ITEM_TAG
{
    MQTT_PROPERTY_TYPE type;
    union
    {
        uint8_t byte_value;
        uint16_t two_byte_value;
        uint32_t four_byte_value;
        char* string_value;
        BUFFER_HANDLE binary_value;

    } value_item;
} MQTT_PROP_ITEM;

typedef struct MQTT_PROPERTY_TAG
{
} MQTT_PROPERTY;

typedef struct PROPERTY_ITERATOR_TAG
{
    size_t index;
} PROPERTY_ITERATOR;

static int get_prop_value_type(MQTT_PROPERTY_TYPE prop_type)
{
    return 0;
/*    switch (prop_type)
    {
    PAYLOAD_FORMAT_INDICATOR
    MSG_EXPIRY_INTERVAL
    CONTENT_TYPE
    RESPONSE_TOPIC
    CORRELATION_DATA
    SUBSCRIPTION_ID
    SESSION_EXPIRY_INTERVAL
    ASSIGNED_CLIENT_ID
    SERVER_KEEP_ALIVE
    AUTHENTICATION_METHOD
    AUTHENTICATION_DATA
    REQUEST_PROBLEM_INFO
    WILL_DELAY_INTERVAL
    REQUEST_RESPONSE_INFO
    RESPONSE_INFO
    SERVER_REFERENCE
    REASON_STRING
    RECEIVE_MAXIMUM
    TOPIC_ALIAS_MAXIMUM
    TOPIC_ALIAS
    MAXIMUM_QOS
    RETAIN_AVAILABLE
    USER_PROPERTY
    MAXIMUM_PACKET_SIZE
    WILDCARD_SUB_AVAILABLE
    SUBSCRIPTION_ID_AVAILABLE
    SHARED_SUB_AVAILABLE
    }*/
}

MQTT_PROPERTY_HANDLE mqtt_prop_create(void)
{
    MQTT_PROPERTY* result;
    if ((result = malloc(sizeof(MQTT_PROPERTY))) == NULL)
    {
        LogError("failure allocating property object");
    }
    else
    {

    }
    return result;
}

void mqtt_prop_destroy(MQTT_PROPERTY_HANDLE handle)
{
    if (handle != NULL)
    {
        free(handle);
    }
}

int mqtt_prop_add_byte_property(MQTT_PROPERTY_TYPE type, uint8_t value)
{
    int result;
    MQTT_PROP_ITEM* prop_item;
    if ((prop_item = (MQTT_PROP_ITEM*)malloc(sizeof(MQTT_PROP_ITEM))) == NULL)
    {
        LogError("failure allocating property item");
        result = __FAILURE__;
    }
    else
    {
        prop_item->type = type;
        prop_item->value_item.byte_value = value;
        result = 0;
    }
    return result;
}
