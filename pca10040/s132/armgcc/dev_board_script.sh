rm -rf build
make
nrfjprog --family nRF52 -e
nrfjprog --family nRF52 --program ../../../../../../components/softdevice/s132/hex/s132_nrf52_6.0.0_softdevice.hex
nrfjprog --family nRF52 -r
nrfjprog --family nRF52 --program _build/nrf52832_xxaa.hex
nrfjprog --family nRF52 -r