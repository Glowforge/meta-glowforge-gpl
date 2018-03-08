#!/bin/sh
### BEGIN INIT INFO
# Provides:          hostname
# Required-Start:
# Required-Stop:
# Default-Start:     S
# Default-Stop:
# Short-Description: Set hostname based on machine serial number
### END INIT INFO

hostname `/usr/bin/machine_hostname || cat /etc/hostname || echo localhost`
