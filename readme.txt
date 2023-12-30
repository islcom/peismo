Get the latest img from RaspberryPi.org


Follow this tutorial: https://austinsnerdythings.com/2021/04/19/microsecond-accurate-ntp-with-a-raspberry-pi-and-pps-gps/ (except our pps signal is gpio4 instead of 18
It involves (in case the link dies):
sudo apt update
sudo apt upgrade
sudo rpi-update
sudo apt install pps-tools gpsd gpsd-clients python-gps chrony
sudo bash -c "echo '# the next 3 lines are for GPS PPS signals' >> /boot/config.txt"
sudo bash -c "echo 'dtoverlay=pps-gpio,gpiopin=4' >> /boot/config.txt"
sudo bash -c "echo 'enable_uart=1' >> /boot/config.txt"
sudo bash -c "echo 'init_uart_baud=9600' >> /boot/config.txt"
sudo bash -c "echo 'pps-gpio' >> /etc/modules"
sudo reboot

Run raspi-config -> 3 – Interface options -> I6 – Serial Port ->
Would you like a login shell to be available over serial -> No. ->
Would you like the serial port hardware to be enabled -> Yes.
Reboot

sudo ppstest /dev/pps0

Edit /etc/default/gpsd and change GPSD_OPTIONS=”” to GPSD_OPTIONS=”-n” and change DEVICES=”” to DEVICES=”/dev/ttyS0 /dev/pps0″, then reboot.

gpsmon

For chrony, add these two lines to the /etc/chrony/chrony.conf file somewhere near the rest of the server lines:
refclock SHM 0 refid NMEA offset 0.200
refclock PPS /dev/pps0 refid PPS lock NMEA

sudo systemctl restart chrony

chronyc sources
