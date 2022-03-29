#!/usr/bin/env bash

SCRIPT_DIR=`dirname "$0"`

DATA_DIR_0=`xpl config path 0`
DATA_DIR_1=`xpl config path 1`
DATA_DIR_N=/tmp/path/not/from/config

rm -fr   $DATA_DIR_0 $DATA_DIR_1 $DATA_DIR_N
mkdir -p $DATA_DIR_0 $DATA_DIR_1 $DATA_DIR_N

dd if=/dev/random of=$DATA_DIR_0/payload_00.data bs=1k count=1 &> /dev/null
dd if=/dev/random of=$DATA_DIR_0/payload_01.data bs=1k count=1 &> /dev/null
dd if=/dev/random of=$DATA_DIR_1/payload_10.data bs=1k count=1 &> /dev/null
dd if=/dev/random of=$DATA_DIR_1/payload_11.data bs=1k count=1 &> /dev/null
dd if=/dev/random of=$DATA_DIR_N/payload_N0.data bs=1k count=1 &> /dev/null

md5sum $DATA_DIR_0/payload_00.data
md5sum $DATA_DIR_0/payload_01.data
md5sum $DATA_DIR_1/payload_10.data
md5sum $DATA_DIR_1/payload_11.data
md5sum $DATA_DIR_N/payload_N0.data

xpl insert tag_1 domain_1 payload_00.data -s 3
xpl insert tag_1 domain_2 payload_01.data -s 5
xpl insert tag_2 domain_1 payload_10.data -s 7
xpl insert tag_2 domain_2 payload_11.data -s 9
xpl insert tag_3 domain_1 payload_N0.data -s 1

rm -f /tmp/test_cli.out

{
xpl show tags
xpl show domains

xpl fetch tag_1 -d domain_1
xpl fetch tag_1 -d domain_2
xpl fetch tag_1 -s 3
xpl fetch tag_1 -s 5
xpl fetch tag_1 -s 3 -d domain_1
xpl fetch tag_1 -s 3 -d domain_2

xpl fetch tag_2 -d domain_1
xpl fetch tag_2 -d domain_2

xpl fetch tag_3 -d domain_1
} |& tee -a /tmp/test_cli.out


diff /tmp/test_cli.out $SCRIPT_DIR/test_cli.out
