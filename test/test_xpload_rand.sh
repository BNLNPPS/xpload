#!/bin/bash

. /opt/sphenix/core/bin/sphenix_setup.sh
export XPLOAD_CONFIG=test
test_xpload_rand "$@"
