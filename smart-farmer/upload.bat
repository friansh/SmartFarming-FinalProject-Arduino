scp smart-farmer.ino.with_bootloader.mega.hex friansh@192.168.1.120:/home/friansh
ssh friansh@192.168.1.120 avrdude -Cavrdude.conf -v -patmega2560 -cwiring -P/dev/ttyACM0 -b115200 -D -Uflash:w:smart-farmer.ino.with_bootloader.mega.hex:i