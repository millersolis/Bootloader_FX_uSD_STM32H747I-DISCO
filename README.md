# Bootloader_FX_uSD_STM32H747I-DISCO
Basic bootloader using microSD™ file handling with FileX and ThreadX for STM32H747XIH6 MCU using STM32H747I-DISCO board.

Includes sample application project compatible with the bootloader.


### FLASH

#### Bank 1
##### Sector 0: Bootloader	(starting at 0x08000000)
##### Sector 1: Configurations
##### Sector 2 & Sector 3: Application (starting at 0x08040000)