/*
 * fw_update.c
 *
 *  Created on: Jul 30, 2023
 *      Author: mille
 */

#include "fw_update.h"

#include <stdio.h>	// TODO: Replace with type-safe printf for embedded
#include <stdbool.h>

#include "sd_card.h"
#include "main.h"

/*         This parameter shall be aligned to the Flash word:
 *          - 256 bits for STM32H74x/5X devices (8x 32bits words)
 */
#define FLASHWORD_SIZE_WORD			(8)								// has to be 8 word aligned

/* Chunk to write should be at least 1 flash word to be written in IFLASH 1 flash word (8 words) at a time. */
#define uSD_FW_CHUNK_SIZE_WORD		(6 * (FLASHWORD_SIZE_WORD))

/* Firmware Size that we have received */
static uint32_t uSD_fw_file_size;						// in words

/* FW size that has been written to FLASH */
static uint32_t updated_fw_size = 0;					// in words

/* Buffer to store chunk of new fw from uSD
 * Must be 32bit aligned to be written to IFLASH	[Miller]
 */
static uint32_t fw_buff[uSD_FW_CHUNK_SIZE_WORD] __attribute__ ((aligned (32)));



/**
  * @brief Write data to the Application's actual flash location.
  * @param data data to be written
  * @param data_len data length in words
  * @is_first_block true - if this is first block, false - not first block
  * @retval True if write to IFLASH success. False otherwise.
  */
static _Bool write_data_to_app_flash(uint32_t *data, uint16_t data_len_word, _Bool is_first_block)
{
	HAL_StatusTypeDef ret;

	do {
//		ret = HAL_FLASH_Unlock();
		ret = HAL_FLASHEx_Unlock_Bank1();	// Should use this one instead? [Miller]
		if (ret != HAL_OK)	break;

		//Check if the FLASH_FLAG_BSY
		FLASH_WaitForLastOperation(HAL_MAX_DELAY, FLASH_BANK_1);

		// Clear all flags before you write it to flash
		// TODO: Needed? [Miller]

	    // clear all flags before you write it to flash
//	    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR |
//	                FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR);

		// Should use Bank 1 only flags? (i.e. FLASH_FLAG_EOP_BANK1, and others) [Miller]
//		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR |
//								FLASH_FLAG_WRPERR | FLASH_FLAG_PGSERR | FLASH_FLAG_PGPERR);

		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS_BANK1);

		// Only erase app IFLASH when writing first block of new FW
		if (is_first_block) {
			printf("Erasing app in IFLASH...\r\n");

			FLASH_EraseInitTypeDef EraseInitStruct;
			uint32_t SectorError;

			/* Bank 1:
			 * Sector 0: Bootloader	(starting at 0x08000000)
			 * Sector 1: Configurations
			 * Sector 2 & Sector 3: Application (starting at 0x08040000)
			 */

			EraseInitStruct.TypeErase		= FLASH_TYPEERASE_SECTORS;
			EraseInitStruct.Banks			= FLASH_BANK_1;				// H7 has 2 internal FLASH Banks
			EraseInitStruct.Sector			= FLASH_SECTOR_2;
			EraseInitStruct.NbSectors		= 2;
//			EraseInitStruct.VoltageRange	= FLASH_VOLTAGE_RANGE_3;	// needed? [Miller]

			ret = HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
			if (ret != HAL_OK)	break;
		}

		// Write new app in IFLASH
		uint32_t data_add = (uint32_t) data;
		uint32_t app_add = (uint32_t) APP_FLASH_ADDR;

		// Writing 32-bit (4-byte) word at a time
		for (int i = 0; i < data_len_word;) {
			ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD,
									(uint32_t)app_add,
									(uint32_t)data_add);
			if (ret != HAL_OK) {
				printf("ERROR\tCould not write new app to FLASH\r\n");
				break;
			}

			// Increase addresses by 4 bytes??
			// to align with flash word write by 8 words
			app_add += (8 *4);
			data_add += (8 *4);
			// TODO: Should be increased by 1 or 4? [Miller]

			i += 8;
		}

		//Check if the FLASH_FLAG_BSY
		FLASH_WaitForLastOperation(HAL_MAX_DELAY, FLASH_BANK_1);

		if (ret != HAL_OK) break;	// Last write to FLASH was not successful

		// Lock FLASH again
//		ret = HAL_FLASH_Lock();
		ret = HAL_FLASHEx_Lock_Bank1();

	} while (false);

	if (ret != HAL_OK) return false;
	else return true;
}


/* Thread entry function for firmware update thread */
void fw_update_thread_entry(void)
{
	sd_init();

	// Since we started the RTOS kernel, it means user wants to update if FW available in uSD

	updated_fw_size = 0;

	// Read file size from uSD --------------
	uSD_fw_file_size = uSD_FW_CHUNK_SIZE_WORD;	// temp for testing [Miller]

	// Write fw file to FLASH by chunks using buffer
	_Bool flash_write_success = true;
	while (updated_fw_size < uSD_fw_file_size) {
		// Zero out buffer
		for (int i = 0; i < uSD_FW_CHUNK_SIZE_WORD; i++)
			fw_buff[i] = (uint32_t) 0;

		// Read file from uSD --------------


		// Store chunk in buffer -----------------

		// Mock buffer data
		for (int i = 0; i < uSD_FW_CHUNK_SIZE_WORD; i++)
			fw_buff[i] = (uint32_t) i;

		// Write buffer to internal FLASH
		if (write_data_to_app_flash(fw_buff, uSD_FW_CHUNK_SIZE_WORD, true)) {
			updated_fw_size += uSD_FW_CHUNK_SIZE_WORD;	// Increased updated fw count
		}
		else {
			printf("ERROR\tCould not write new app chunk to FLASH\r\n");
			flash_write_success = false;
			break;
		}

	}

	// TODO: Report result [Miller]
	if (flash_write_success)
		printf("SUCCESS\tWrote test buffer to IFLASH\r\n");
	else
		printf("FAILURE\tErrors writing test buffer to IFLASH\r\n");

	// Jump to app (old or new if updated) -----------------
	do {
		// spinlock

	} while(true);
}
