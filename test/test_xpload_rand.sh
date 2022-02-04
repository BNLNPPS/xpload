#!/usr/bin/env bash

# Set up the environment. We use sPHENIX start up script
. /opt/sphenix/core/bin/sphenix_setup.sh
export XPLOAD_CONFIG=test
# Pass all arguments to the test program
test_xpload_rand "$@"
