rm -rf _build
make
openocd  -f interface/stlink.cfg -f target/nrf52.cfg -c init -c "reset halt" -c "nrf51 mass_erase 0" -c "flash write_image ../../../../nRF5_SDK_15.0.0_a53641a/components/softdevice/s132/hex/s132_nrf52_6.0.0_softdevice.hex" -c reset -c exit
openocd  -f interface/stlink.cfg  -f target/nrf52.cfg -c init -c "reset halt"  -c "flash write_image _build/nrf52832_xxaa.hex" -c reset -c exit