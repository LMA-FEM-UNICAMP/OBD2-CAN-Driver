#!/usr/bin/env bash

rm -rf /opt/obd2_can_driver

sudo systemctl disable obd2_can_driver_daemon

rm /etc/systemd/system/obd2_can_driver_daemon.service

rm /usr/local/bin/obd2_can_driver_status