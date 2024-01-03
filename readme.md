# Peismo
Seismograph based on Raspberry Pi (tested with pi2 and pi3), a A/D Expansion board by waveshare https://www.waveshare.com/high-precision-ad-da-board.htm and any GPS with PPS output https://www.adafruit.com/product/2324
## Installation
### Waveshare Board Setup
Add jumper to connect AINCOM to AGND

Add jumper to connect VCC to 5V

Other jumpers can be omitted

Connect sensor between AD0 and AD1 and connect any shield to GND (if isolated from the sensor)

### Get the latest img from RaspberryPi.org
Use the official Raspberry Pi Imager from https://www.raspberrypi.com/software/

Change the password from the default. 

**!!! Set timezone to UTC !!!**

Boot to CLI instead of desktop

(In Advanced) Enable SSH if you want to do the rest of this from a remote console

### The following instructions come from this tutorial: https://austinsnerdythings.com/2021/04/19/microsecond-accurate-ntp-with-a-raspberry-pi-and-pps-gps/ (except our pps signal is gpio4 instead of 18)

```bash
sudo apt update
sudo apt upgrade
sudo apt install pps-tools gpsd gpsd-clients chrony
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

```bash
sudo nano /etc/default/gpsd
```
Change change DEVICES=”” to DEVICES=”/dev/ttyS0 /dev/pps0″ and GPSD_OPTIONS=”” to GPSD_OPTIONS=”-n” then reboot.
```bash
gpsmon
```

```bash
sudo bash -c "echo 'refclock SHM 0 refid NMEA offset 0.200' >> /etc/chrony/chrony.conf"
sudo bash -c "echo 'refclock PPS /dev/pps0 refid PPS lock NMEA' >> /etc/chrony/chrony.conf"
```
```bash
sudo systemctl restart chrony
```
Test the PPS input
```bash
sudo ppstest /dev/pps0
```
Test Chrony
```bash
chronyc sources
```
At first you will see #? next to PPS. After some time, that should change to #* to indicate it is using it as a source of time.

### The following instructions come from waveshare (particularly the WiringPi section) https://www.waveshare.com/wiki/High-Precision_AD/DA_Board
```bash
cd ~
git clone https://github.com/WiringPi/WiringPi
cd WiringPi
./build
```

### Install miniseed conversion utility

```bash
cd ~
git clone https://github.com/iris-edu/ascii2mseed
cd ascii2mseed
make
```

### Copy this repository
```bash
cd ~
git clone https://github.com/colinlove/peismo
cd peismo
make
```

You'll need to copy ftpconfig.ini.template to ftpconfig.ini and update ftp/sftp credentials and station name
```bash
cp ftpconfig.ini.template ftpconfig.ini
nano ftpconfig.ini
```
### Add SFTP support to Python
```bash
pip install pysftp
```
### Modify the Cron Schedule
```bash
crontab -e
```
insert
```bash
*/5 * * * * cd ~/peismo && python peismoadmin.py; python mseed.py; python ftpupload.py
```
