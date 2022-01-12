#!/bin/bash

. /opt/sphenix/core/bin/sphenix_setup.sh
export XPLOAD_CONFIG_NAME=test
test_xpload_rand "$@"
