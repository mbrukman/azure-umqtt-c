#!/bin/bash
# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.
#

set -e

git submodule update --init

script_dir=$(cd "$(dirname "$0")" && pwd)
build_root=$(cd "${script_dir}/.." && pwd)
log_dir=$build_root
make_install=
build_folder=$build_root"/cmake_debian"

rm -r -f $build_folder
mkdir -p $build_folder
pushd $build_folder
cmake .. -Drun_unittests:bool=ON  -Drun_valgrind:BOOL=ON
cmake --build . -- --jobs=$(nproc)
ctest -C "debug" -V

if [[ $make_install == 1 ]] ;
then
    echo "Installing packaging" 
    # install the package
    make install
fi

popd
: