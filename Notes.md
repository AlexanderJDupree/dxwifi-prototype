# Beaglebone Black Experimentation - 1/30/21


## Setup

- Beaglebone Black loaded w/ oresat-generic-2020-12-19.img
- Atheros AR9271 USB Wifi Adapter - [Used this one](https://www.amazon.com/gp/product/B07FVRKCZJ/ref=ppx_yo_dt_b_asin_title_o02_s00?ie=UTF8&psc=1)
- USB Serial Cable - [Used this one](https://www.amazon.com/gp/product/B00DDF8TV6/ref=ppx_yo_dt_b_asin_title_o06_s00?ie=UTF8&psc=1)

## Process

- Burned the oresat generic image onto a micro sd card
- Plug in USB serial Cable and ran `sudo screen /dev/ttyUSB0 115200`
- With SD card inserted into Beaglebone, hold down S2 and plug in the power cable
- Beaglebone should boot from the SD card and will eventually ask you to log in. 
- Used default credentials `oresat:temppwd`

### Notes
```
oresat@live:~$ sudo vim /etc/hostname # changed name from 'generic' to 'live'
oresat@live:~$ sudo vim /etc/hosts # changed name from 'generic' to 'live'
```

```
oresat@live:~$ uname -a
Linux live 4.19.94-ti-r55 #1buster SMP PREEMPT Tue Oct 27 21:48:45 UTC 2020 armv7l GNU/Linux
```

:( iw isn't installed
```
oresat@live:~$ iw dev
-bash: iw: command not found
```

Plugged in a ethernet cable but still wasn't getting internet since the network rules was prioritizing the USB adapters gateway. 
Made the following changes in `/etc/systemd/network/`

- Renamed `20-wired.network` to `30-wired-usb.network` and made the following changes
```
[Match]
Name=usb0

[Network]
Address=192.168.6.2/24
DNS=192.168.6.1
DNS=8.8.8.8
DNS=8.8.4.4

[Route]
Gateway=192.168.6.1
Destination=0.0.0.0/0
Metric=1024
```
- Created file `25-wired.network` with the following:
```
# 25-wired.network
[Match]
Name=eth0

[Network]
DHCP=ipv4

[DHCP]
SendHostname=True
UseDomains=True

[Route]
Destination=0.0.0.0/0
Metric=1000
```
- Power cycled beaglebone, should be using ethernet gateway now
```
oresat@live:/etc/systemd/network$ ip route
default via 192.168.0.1 dev eth0
default dev eth0 proto static metric 1000
default via 192.168.6.1 dev usb0 proto static metric 1024
default via 192.168.0.1 dev eth0 proto dhcp src 192.168.0.19 metric 1024
192.168.0.0/24 dev eth0 proto kernel scope link src 192.168.0.19
192.168.0.1 dev eth0 proto dhcp scope link src 192.168.0.19 metric 1024
192.168.6.0/24 dev usb0 proto kernel scope link src 192.168.6.2
```


