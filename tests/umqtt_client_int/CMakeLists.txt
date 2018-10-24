#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

compileAsC11()
set(theseTestsName umqtt_client_int)

set(${theseTestsName}_test_files
    ${theseTestsName}.c
)

set(${theseTestsName}_c_files
    ../../src/mqtt_client.c
    ../../src/mqtt_codec.c
    ../../src/mqtt_message.c
    ${SHARED_UTIL_SRC_FOLDER}/strings.c
    ${TICKCOUTER_C_FILE}
    ${SHARED_UTIL_SRC_FOLDER}/buffer.c
)

if (NOT WIN32)
    set(${theseTestsName}_c_files
        ${SHARED_UTIL_ADAPTER_FOLDER}/linux_time.c
    )
endif()

set(${theseTestsName}_h_files
)

include_directories(${MQTT_SRC_FOLDER})

build_c_test_artifacts(${theseTestsName} ON "tests/umqtt_tests")
