#!/usr/bin/env bash

ip link set can0 down
ip link set can0 type can bitrate 250000 
ip link set can0 up 

ip link add dev vcan0 type vcan