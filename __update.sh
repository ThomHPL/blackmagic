#!bin/sh
sudo dfu-util -d 1d50:6018,:6017 -s 0x08002000:leave -D src/blackmagic.bin