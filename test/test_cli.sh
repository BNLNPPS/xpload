#!/usr/bin/env bash

SCRIPT_DIR=`dirname "$0"`

DATA_DIR_0=`xpl config path 0`
DATA_DIR_1=`xpl config path 1`

rm -fr   $DATA_DIR_0 $DATA_DIR_1
mkdir -p $DATA_DIR_1

randfile() { python -c 'import random, sys; random.seed(int(sys.argv[1])); sys.stdout.buffer.write(bytes([random.getrandbits(8) for _ in range(1000)]))' "$@"; }

randfile 1 > /tmp/payload_00.data
randfile 2 > /tmp/payload_01.data
randfile 3 > /tmp/payload_10.data
randfile 4 > /tmp/payload_11.data
randfile 5 > /tmp/payload_N0.data

rm -f /tmp/test_cli.out

{
xpl add tag tag_1 -t tag_1_type -s tag_1_status -d domain_1 domain_2
xpl add tag tag_2 -t tag_2_type -s tag_2_status -d domain_1 domain_2
xpl add tag tag_3 -t tag_3_type -s tag_3_status -d domain_3
xpl add pil tag_1 domain_1 /tmp/payload_00.data -s 3
xpl add pil tag_1 domain_2 /tmp/payload_01.data -s 5
xpl add pil tag_2 domain_1 /tmp/payload_10.data -s 7
xpl add pil tag_2 domain_2 /tmp/payload_11.data -s 9
xpl add pil tag_3 domain_3 /tmp/payload_N0.data -s 1
xpl push

xpl show tags
xpl show domains

xpl fetch tag_1 -d domain_1
xpl fetch tag_1 -d domain_2
xpl fetch tag_1 -t 3
xpl fetch tag_1 -t 5
xpl fetch tag_1 -t 3 -d domain_1
xpl fetch tag_1 -t 3 -d domain_2

xpl fetch tag_2 -d domain_1
xpl fetch tag_2 -d domain_2

xpl fetch tag_3 -d domain_1
} |& tee -a /tmp/test_cli.out


diff /tmp/test_cli.out $SCRIPT_DIR/test_cli.out
