#!/bin/sh
sudo echo "SUBSYSTEM==\"usb\", SYSFS{idVendor}==\"04e8\", SYSFS{idProduct}==\"6601\", MODE=\"0666\"" > /etc/udev/rules.d/60-heimdall-galaxy-s.rules
sudo service udev reload
exit 0

