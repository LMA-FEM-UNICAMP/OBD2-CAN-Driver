# OBD2 CAN Driver for Cohda MK6 OBU

<p align="center">
  <img width="60%" height="80%" src="docs/OBU_OBD2-diagram.png">
</p>

## Clonning

```shell
cd /opt
git clone https://github.com/LMA-FEM-UNICAMP/OBD2-CAN-Driver.git
```

## Compiling

```shell
cd obd2_can_driver
cmake .
make
```

## Running (for testing)

```shell
sudo ./obd2_can_driver_deamon &
```

Checking:

```shell
# Process
ps aux | grep obd2_can_driver_deamon

# Log
sudo journalctl -xe | grep obd2_can_driver_deamon

# Output
nc -U /tmp/obd2_can_logging.sock
```

## Installing the Deamon

```ini
# In the file /etc/systemd/system/obd2_can_driver_deamon.service
[Unit]
Description=OBD2 CAN Driver Deamon
After=network.target

[Service]
ExecStart=/opt/OBD2-CAN-Driver/obd2_can_driver/obd2_can_driver_deamon
Restart=always

[Install]
WantedBy=multi-user.target
```

Or

```shell
# From OBD2-CAN-Driver/
cp obd2_can_driver_deamon.service /etc/systemd/system
```

Then to activate the deamon:

```shell
sudo systemctl enable obd2_can_driver_deamon
sudo systemctl start obd2_can_driver_deamon
```