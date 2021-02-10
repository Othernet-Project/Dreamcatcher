#!/bin/bash
set -eu
for target in esp32 esp32s2; do
    rm -rf build.${target}
    idf.py -B build.${target} -DSDKCONFIG=sdkconfig.${target} -DIDF_TARGET=${target} build
    cp sdkconfig.${target} build.${target}/sdkconfig  # extra step to match your script behavior
    cp sdkconfig-${target} sdkconfig.${target}  # extra step to match your script behavior    
done
