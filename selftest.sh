#!/bin/bash

POKEREV="./pokerev -c"

# Pre-flop KQo vs KQs
echo -n "Test 1.1 - "
$POKEREV --hands ksqh:kdqd | grep -q "KsQh,47.5079,2.1732,90.6695"
if [ $? -ne 0 ]
then
	echo "FAIL"
	exit 1
fi
echo "pass"

echo -n "Test 1.2 - "
$POKEREV --hands ksqh:kdqd | grep -q "KdQd,52.4921,7.1574,90.6695"
if [ $? -ne 0 ]
then
	echo "FAIL"
	exit 1
fi
echo "pass"

# Nut flush draw vs set on flop
echo -n "Test 2.1 - "
$POKEREV --hands askd:4d4c --board ts9s4s | grep -q "AsKd,28.2828,28.2828,0.0000"
if [ $? -ne 0 ]
then
	echo "FAIL"
	exit 1
fi
echo "pass"

echo -n "Test 2.2 - "
$POKEREV --hands askd:4d4c --board ts9s4s | grep -q "4d4c,71.7172,71.7172,0.0000"
if [ $? -ne 0 ]
then
	echo "FAIL"
	exit 1
fi
echo "pass"

# Nut straight vs nut straight + flush draw on turn
echo -n "Test 3.1 - "
$POKEREV --hands kd3d:kcks --board adjstcqd | grep -q "Kd3d,60.2273,20.4545,79.5455"
if [ $? -ne 0 ]
then
	echo "FAIL"
	exit 1
fi
echo "pass"

echo -n "Test 3.2 - "
$POKEREV --hands kd3d:kcks --board adjstcqd | grep -q "KcKs,39.7727,0.0000,79.5455"
if [ $? -ne 0 ]
then
	echo "FAIL"
	exit 1
fi
echo "pass"

