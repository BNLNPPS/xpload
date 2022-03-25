#!/usr/bin/env bash

SCRIPT_DIR=`dirname "$0"`
DATA_DIR=`xpl config path`
mkdir -p $DATA_DIR

echo $SCRIPT_DIR

dd if=/dev/random of=$DATA_DIR/payload_10.data bs=1k count=1 &> /dev/null
dd if=/dev/random of=$DATA_DIR/payload_11.data bs=1k count=1 &> /dev/null

ls -la $DATA_DIR/

md5sum $DATA_DIR/payload_10.data
md5sum $DATA_DIR/payload_11.data

xpl insert tag_1 domain_1 payload_10.data -s 3
xpl insert tag_1 domain_2 payload_11.data -s 5
xpl insert tag_2 domain_1 payload_10.data -s 7
xpl insert tag_2 domain_2 payload_11.data -s 9

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
} |& tee -a /tmp/test_cli.out


diff /tmp/test_cli.out $SCRIPT_DIR/test_cli.out
