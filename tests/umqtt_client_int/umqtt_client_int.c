// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#include <cstdint>
#else
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#endif

#include "testrunnerswitcher.h"
#include "umock_c.h"
#include "umock_c_negative_tests.h"
#include "umocktypes_bool.h"
#include "umocktypes_stdint.h"

#if defined _MSC_VER
#pragma warning(disable: 4054) /* MSC incorrectly fires this */
#endif

static void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

static void my_gballoc_free(void* ptr)
{
    free(ptr);
}

#define ENABLE_MOCKS
#include "azure_c_shared_utility/agenttime.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/xio.h"
#undef ENABLE_MOCKS

#include "azure_umqtt_c/mqtt_client.h"
#include "azure_umqtt_c/mqtt_message.h"
#include "azure_umqtt_c/mqttconst.h"

#include "azure_c_shared_utility/strings.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/umock_c_prod.h"

MOCKABLE_FUNCTION(, void, on_mqtt_operation_callback, MQTT_CLIENT_HANDLE, handle, MQTT_CLIENT_EVENT_RESULT, actionResult, const void*, msgInfo, void*, callbackCtx);
MOCKABLE_FUNCTION(, void, on_mqtt_disconnected_callback, void*, callback_ctx);

#undef ENABLE_MOCKS

#ifdef __cplusplus
extern "C"
{
#endif

    int STRING_sprintf(STRING_HANDLE handle, const char* format, ...);
    STRING_HANDLE STRING_construct_sprintf(const char* format, ...);

#ifdef __cplusplus
}
#endif

TEST_DEFINE_ENUM_TYPE(QOS_VALUE, QOS_VALUE_VALUES);
IMPLEMENT_UMOCK_C_ENUM_TYPE(QOS_VALUE, QOS_VALUE_VALUES);

static const char* TEST_USERNAME = "testuser";
static const char* TEST_PASSWORD = "testpassword";

static const char* TEST_TOPIC_NAME = "topic Name";
static const APP_PAYLOAD TEST_APP_PAYLOAD = { (uint8_t*)"Message to send", 15 };
static const char* TEST_CLIENT_ID = "test_client_id";
static const char* TEST_WILL_MSG = "test_will_msg";
static const char* TEST_WILL_TOPIC = "test_will_topic";
static const char* TEST_SUBSCRIPTION_TOPIC = "subTopic";
static SUBSCRIBE_PAYLOAD TEST_SUBSCRIBE_PAYLOAD[] = { {"subTopic1", DELIVER_AT_LEAST_ONCE }, {"subTopic2", DELIVER_EXACTLY_ONCE } };
static const char* TEST_UNSUBSCRIPTION_TOPIC[] = { "subTopic1", "subTopic2" };
static const uint8_t* TEST_MESSAGE = (const uint8_t*)"Message to send";
static const int TEST_MSG_LEN = sizeof(TEST_MESSAGE)/sizeof(TEST_MESSAGE[0]);

static const XIO_HANDLE TEST_IO_HANDLE = (XIO_HANDLE)0x11;
static const uint16_t TEST_KEEP_ALIVE_INTERVAL = 20;
static bool g_continue;

typedef enum OPERATION_STATE_TAG
{
    OPERATION_STATE_CONNECT = 0x0001,
    OPERATION_STATE_SUBSCRIBE = 0x0002,
    OPERATION_STATE_UNSUBSCRIBE = 0x0004,
    OPERATION_STATE_PUBLISH = 0x0008,
    OPERATION_STATE_PING = 0x00010
} OPERATION_STATE;

//ON_PACKET_COMPLETE_CALLBACK g_packetComplete;
ON_IO_OPEN_COMPLETE g_openComplete;
ON_BYTES_RECEIVED g_bytesRecv;
ON_IO_ERROR g_ioError;
ON_SEND_COMPLETE g_sendComplete;
void* g_onCompleteCtx;
void* g_onSendCtx;
void* g_bytesRecvCtx;
void* g_ioErrorCtx;
typedef struct TEST_COMPLETE_DATA_INSTANCE_TAG
{
    MQTT_CLIENT_EVENT_RESULT actionResult;
    void* msgInfo;
} TEST_COMPLETE_DATA_INSTANCE;

TEST_MUTEX_HANDLE test_serialize_mutex;

#define TEST_CONTEXT ((const void*)0x4242)
#define MAX_CLOSE_RETRIES               2
#define CLOSE_SLEEP_VALUE               2

#ifdef __cplusplus
extern "C" {
#endif

    static int my_xio_open(XIO_HANDLE handle, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
    {
        (void)handle;
        /* Bug? : This is a bit wierd, why are we not using on_io_error and on_bytes_received? */
        g_openComplete = on_io_open_complete;
        g_onCompleteCtx = on_io_open_complete_context;
        g_bytesRecv = on_bytes_received;
        g_bytesRecvCtx = on_bytes_received_context;
        g_ioError = on_io_error;
        g_ioErrorCtx = on_io_error_context;
        return 0;
    }

    static int my_xio_send(XIO_HANDLE xio, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
    {
        (void)xio;
        (void)buffer;
        (void)size;
        g_sendComplete = on_send_complete;
        g_onSendCtx = callback_context;
        return 0;
    }

    static int TEST_mallocAndStrcpy_s(char** destination, const char* source)
    {
        size_t src_len = strlen(source);
        *destination = (char*)my_gballoc_malloc(src_len + 1);
        memcpy(*destination, source, src_len + 1);
        return 0;
    }


#ifdef __cplusplus
}
#endif

DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

static void on_msg_recv_cb(MQTT_MESSAGE_HANDLE handle, void* context)
{
    (void)handle;
    (void)context;
}

static void operation_cb(MQTT_CLIENT_HANDLE handle, MQTT_CLIENT_EVENT_RESULT action_result, const void* msg_info, void* ctx)
{
    (void)handle;
    (void)msg_info;
    (void)ctx;
    switch (action_result)
    {
        case MQTT_CLIENT_ON_CONNACK:
            break;
        case MQTT_CLIENT_ON_PUBLISH_ACK:
        case MQTT_CLIENT_ON_PUBLISH_RECV:
        case MQTT_CLIENT_ON_PUBLISH_REL:
        case MQTT_CLIENT_ON_PUBLISH_COMP:
        case MQTT_CLIENT_ON_SUBSCRIBE_ACK:
        case MQTT_CLIENT_ON_UNSUBSCRIBE_ACK:
        case MQTT_CLIENT_ON_DISCONNECT:
        case MQTT_CLIENT_ON_PING_RESPONSE:
            break;
    }
}

static void error_cb(MQTT_CLIENT_HANDLE handle, MQTT_CLIENT_EVENT_ERROR error, void* context)
{
    (void)handle;
    (void)context;
    switch (error)
    {
        case MQTT_CLIENT_CONNECTION_ERROR:
        case MQTT_CLIENT_PARSE_ERROR:
        case MQTT_CLIENT_MEMORY_ERROR:
        case MQTT_CLIENT_COMMUNICATION_ERROR:
        case MQTT_CLIENT_NO_PING_RESPONSE:
        case MQTT_CLIENT_UNKNOWN_ERROR:
            //g_errorCallbackInvoked = true;
            break;
    }
}

static void setup_options(MQTT_CLIENT_OPTIONS* options, const char* clientId,
    const char* willMsg,
    const char* willTopic,
    const char* username,
    const char* password,
    uint16_t keepAlive,
    bool messageRetain,
    bool cleanSession,
    QOS_VALUE qos)
{
    options->clientId = (char*)clientId;
    options->willMessage = (char*)willMsg;
    options->willTopic = (char*)willTopic;
    options->username = (char*)username;
    options->password = (char*)password;
    options->keepAliveInterval = keepAlive;
    options->useCleanSession = cleanSession;
    options->qualityOfServiceValue = qos;
    options->messageRetain = messageRetain;
}

static void send_mqtt_msg(uint32_t op_state)
{
    g_continue = true;

    uint16_t packet_id = 4;
    int result;

    MQTT_CLIENT_OPTIONS mqtt_options = { 0 };
    setup_options(&mqtt_options, TEST_CLIENT_ID, TEST_WILL_MSG, TEST_WILL_TOPIC, TEST_USERNAME, TEST_PASSWORD, TEST_KEEP_ALIVE_INTERVAL, false, true, DELIVER_AT_MOST_ONCE);

    MQTT_CLIENT_HANDLE mqtt_handle = mqtt_client_init(on_msg_recv_cb, operation_cb, NULL, error_cb, NULL);
    ASSERT_IS_NOT_NULL(mqtt_handle);

    result = mqtt_client_connect(mqtt_handle, TEST_IO_HANDLE, &mqtt_options);
    ASSERT_ARE_EQUAL(int, 0, result);

    if (op_state & OPERATION_STATE_SUBSCRIBE)
    {
        result = mqtt_client_subscribe(mqtt_handle, packet_id++, TEST_SUBSCRIBE_PAYLOAD, 2);
        ASSERT_ARE_EQUAL(int, 0, result);
    }
    if (op_state & OPERATION_STATE_UNSUBSCRIBE)
    {
        //result = mqtt_client_unsubscribe(mqtt_handle, packet_id++, TEST_UNSUBSCRIPTION_TOPIC, 2);
        //ASSERT_ARE_EQUAL(int, 0, result);
    }
    if (op_state & OPERATION_STATE_PUBLISH)
    {
        MQTT_MESSAGE_HANDLE handle = mqttmessage_create(packet_id++, TEST_TOPIC_NAME, DELIVER_AT_MOST_ONCE, NULL, 0);
 
        result = mqtt_client_publish(mqtt_handle, handle);
        ASSERT_ARE_EQUAL(int, 0, result);

        mqttmessage_destroy(handle);
    }

    do
    {
        mqtt_client_dowork(mqtt_handle);
    } while (g_continue);

    // act
    mqtt_client_deinit(mqtt_handle);
}

BEGIN_TEST_SUITE(umqtt_client_int)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);
    result = umocktypes_bool_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    result = umocktypes_stdint_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    REGISTER_UMOCK_ALIAS_TYPE(ON_PACKET_COMPLETE_CALLBACK, void*);
    REGISTER_UMOCK_ALIAS_TYPE(TICK_COUNTER_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(MQTTCODEC_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(XIO_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_SEND_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(BUFFER_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(MQTT_MESSAGE_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_OPEN_COMPLETE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_BYTES_RECEIVED, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_ERROR, void*);
    REGISTER_UMOCK_ALIAS_TYPE(STRING_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_CLOSE_COMPLETE, void*)

    REGISTER_TYPE(QOS_VALUE, QOS_VALUE);

    REGISTER_GLOBAL_MOCK_HOOK(mallocAndStrcpy_s, TEST_mallocAndStrcpy_s);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_malloc, NULL);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);

    REGISTER_GLOBAL_MOCK_HOOK(xio_open, my_xio_open);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(xio_open, __FAILURE__);
    REGISTER_GLOBAL_MOCK_HOOK(xio_send, my_xio_send);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(xio_send, __FAILURE__);
    REGISTER_GLOBAL_MOCK_RETURN(xio_close, 0);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(xio_close, __FAILURE__);

    REGISTER_GLOBAL_MOCK_RETURN(get_time, time(NULL) );

    //REGISTER_GLOBAL_MOCK_RETURN(platform_init, 0);
    //REGISTER_GLOBAL_MOCK_FAIL_RETURN(platform_init, __FAILURE__);

    REGISTER_GLOBAL_MOCK_RETURN(mallocAndStrcpy_s, 0);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(mallocAndStrcpy_s, __FAILURE__);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    if (TEST_MUTEX_ACQUIRE(test_serialize_mutex))
    {
        ASSERT_FAIL("Could not acquire test serialization mutex.");
    }
}

TEST_FUNCTION_CLEANUP(method_cleanup)
{
    umock_c_reset_all_calls();
    TEST_MUTEX_RELEASE(test_serialize_mutex);
}

static int should_skip_index(size_t current_index, const size_t skip_array[], size_t length)
{
    int result = 0;
    for (size_t index = 0; index < length; index++)
    {
        if (current_index == skip_array[index])
        {
            result = __FAILURE__;
            break;
        }
    }
    return result;
}

TEST_FUNCTION(fuzzing_CONNECT_succeeds)
{
    // arrange

    send_mqtt_msg(0);
    // assert
}

END_TEST_SUITE(umqtt_client_int)
