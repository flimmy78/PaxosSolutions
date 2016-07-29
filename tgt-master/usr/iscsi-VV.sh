#!/bin/sh

./tgtadm --lld iscsi --op new --mode target --tid 1 --targetname AAA
./tgtadm --lld iscsi --op new --mode logicalunit --tid 1 --lun 1 --bstype VV -b VV_1
#./tgtadm --lld iscsi --op new --mode account --user user1 --password 12345678
#./tgtadm --lld iscsi --op bind --mode account --tid 1 --user user1
./tgtadm --lld iscsi --op bind --mode target --tid 1 -I ALL
