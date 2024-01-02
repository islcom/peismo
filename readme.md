# Peismo
Seismograph based on a A/D Expansion board by waveshare https://www.waveshare.com/high-precision-ad-da-board.htm and any GPS with PPS output https://www.adafruit.com/product/2324
## Installation
### Get the latest img from RaspberryPi.org
Use the official Raspberry Pi Imager from https://www.raspberrypi.com/software/

Change the password from the default. Keep the dafault username as the current version of the scripts assumes code is in /home/pi/peismo

If you get a chance, turn on SPI, turn on Serial, turn off the Serial Console, boot to CLI instead of desktop

**!!! Set timezone to UTC !!!**

### Follow the instructions from waveshare (particularly the WiringPi section) https://www.waveshare.com/wiki/High-Precision_AD/DA_Board
```bash
sudo raspi-config
```
Choose Interfacing Options -> SPI -> Yes to enable the SPI interface
```bash
#Open the Raspberry Pi terminal and run the following command
sudo apt-get install wiringpi
#For Raspberry Pi systems after May 2019 (earlier than that can be executed without), an upgrade may be required:
wget https://project-downloads.drogon.net/wiringpi-latest.deb
sudo dpkg -i wiringpi-latest.deb
gpio -v
# Run gpio -v and version 2.52 will appear, if it doesn't it means there was an installation error

# Bullseye branch system using the following command:
git clone https://github.com/WiringPi/WiringPi
cd WiringPi
. /build
gpio -v
# Run gpio -v and version 2.70 will appear, if it doesn't it means there was an installation error
```

### Follow this tutorial: https://austinsnerdythings.com/2021/04/19/microsecond-accurate-ntp-with-a-raspberry-pi-and-pps-gps/ (except our pps signal is gpio4 instead of 18)
It involves (in case the link dies):
```bash
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
```
```bash
raspi-config
```
Interface options -> Serial Port ->
Would you like a login shell to be available over serial -> No. ->
Would you like the serial port hardware to be enabled -> Yes. Reboot

Test the PPS input
```bash
sudo ppstest /dev/pps0
```

Edit /etc/default/gpsd and change GPSD_OPTIONS=”” to GPSD_OPTIONS=”-n” and change DEVICES=”” to DEVICES=”/dev/ttyS0 /dev/pps0″, then reboot.
```bash
gpsmon
```
For chrony, add these two lines to the /etc/chrony/chrony.conf file somewhere near the rest of the server lines:
```bash
refclock SHM 0 refid NMEA offset 0.200
refclock PPS /dev/pps0 refid PPS lock NMEA
```
```bash
sudo systemctl restart chrony
```
```bash
chronyc sources
```

### Copy this repository
```bash
git clone https://github.com/colinlove/peismo
cd peismo
```

You'll need to copy ftpconfig.ini.template to ftpconfig.ini and update ftp/sftp credentials and station name
```bash
cp ftpconfig.ini.template ftpconfig.ini
nano ftpconfig.ini
```

### Modify the Cron Schedule
```bash
crontab -e
```
insert
```bash
*/5 * * * * cd /home/pi/peismo && python peismoadmin.py; python mseed.py; python ftpupload.py
```
