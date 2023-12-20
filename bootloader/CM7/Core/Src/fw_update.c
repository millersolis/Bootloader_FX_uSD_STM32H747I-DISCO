/*
 * fw_update.c
 *
 *  Created on: Jul 30, 2023
 *      Author: mille
 */

#include "fw_update.h"

#include <stdio.h>	// TODO: Replace with type-safe printf for embedded
#include <stdbool.h>
#include "fx_api.h"	//temp

#include "sd_card.h"
#include "main.h"


//#define FW_FILENAME					"test.txt"
#define FW_FILENAME					"sample.bin"

#define FLASHWORD_SIZE_WORD			(8)								// has to be aligned to the Flash word:256 bits for STM32H74x/5X devices (8x 32bits words)

/* Chunk to write should be at least 1 flash word to be written in IFLASH 1 flash word (8 words) at a time. */
#define uSD_FW_CHUNK_SIZE_WORD		(6 * (FLASHWORD_SIZE_WORD))

#define uSD_FW_CHUNK_SIZE_BYTE		(4 * (uSD_FW_CHUNK_SIZE_WORD))

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
  * @param dest_add address in IFLASH to write to. Has to be aligned to a flash word -> 256-bit aligned
  * @param data_len data length in words
  * @is_first_block true - if this is first block, false - not first block
  * @retval True if write to IFLASH success. False otherwise.
  */
static _Bool write_data_to_app_flash(uint32_t *data, uint32_t* dest_add,
									uint16_t data_len_word, _Bool is_first_block)
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
			 * Sector 2 & Sector 3: Application (starting at 0x08040000)	// Has to be aligned to a flash word -> 256-bit aligned
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
		uint32_t app_add = (uint32_t) dest_add;	// Has to be aligned to a flash word -> 256-bit aligned

		// Writing 1 FLASH word at a time
		for (int i = 0; i < data_len_word;) {
			ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD,
									(uint32_t)app_add,
									(uint32_t)data_add);
			if (ret != HAL_OK) {
				printf("ERROR\tCould not write new app to FLASH\r\n");
				break;
			}

			// Increase by a flash word -> 8 32-bit words -> 8 4-byte words -> 256 bits
			app_add += (8 *4);
			data_add += (8 *4);

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

_Bool sd_fw_file_exists(void)
{
	return true;
}

_Bool sd_get_fw_file_size(uint32_t* size)
{
	if(sd_fx_get_file_size(FW_FILENAME, size) != true) {
		printf("ERROR\tCould not get fw file size\r\n");
		return false;
	}

	return true;
}

_Bool get_sd_fw_chunk(uint8_t* dest_buf, uint32_t buf_size,
		uint32_t request_size,uint32_t* bytes_read, uint64_t offset)
{
	if (sd_fx_file_read(FW_FILENAME,
						dest_buf,
						buf_size,
						request_size,
						bytes_read,
						offset)
			!= FX_SUCCESS) {
		printf("ERROR\tCould not read chunk of fw file\r\n");
		return false;
	}

	return true;
}


/* Thread entry function for firmware update thread */
void fw_update_thread_entry(void)
{
	sd_init();


	// Since we started the RTOS kernel, it means user wants to update if FW available in uSD

	updated_fw_size = 0;

	// Read file size from uSD --------------

//	uSD_fw_file_size = uSD_FW_CHUNK_SIZE_WORD;	// temp for testing [Miller]
	if (sd_get_fw_file_size(&uSD_fw_file_size) == false || uSD_fw_file_size == 0) {
		printf("INFO\tCould not find app fw file\r\n");
		return;
	}


//	uint8_t data[5];
//	uint32_t bytes_read;
//	uint32_t fw_filesize;
//	sd_fx_file_read(FW_FILENAME, data, 5, 5, &bytes_read);	// reading "test.txt" for testing [Miller]


	// Write fw file to FLASH by chunks using buffer
	_Bool flash_write_success = true;
	_Bool is_first_chunk = true;
	uint32_t bytes_read = 0;


	while (updated_fw_size < uSD_fw_file_size) {
		// Zero out buffer
		for (int i = 0; i < uSD_FW_CHUNK_SIZE_WORD; i++)
			fw_buff[i] = (uint32_t) 0;

		// Read file from uSD --------------

		// Mock buffer data for testing [Miller]
//		for (int i = 0; i < uSD_FW_CHUNK_SIZE_WORD; i++)
//			fw_buff[i] = (uint32_t) i;

		if (get_sd_fw_chunk((uint8_t*)fw_buff, uSD_FW_CHUNK_SIZE_BYTE,
								uSD_FW_CHUNK_SIZE_BYTE, &bytes_read, updated_fw_size) == false
			|| bytes_read == 0) {
			printf("WARNING\tCould not read new chunk of app fw file\r\n");
		}

		// Store chunk in buffer -----------------

		// Write buffer to internal FLASH
		if (write_data_to_app_flash(fw_buff, (uint32_t*)(APP_FLASH_ADDR + updated_fw_size),
									uSD_FW_CHUNK_SIZE_WORD, is_first_chunk)) {
			updated_fw_size += uSD_FW_CHUNK_SIZE_BYTE;	// Increased updated fw count
			if (is_first_chunk) is_first_chunk = false;
			printf("%d / %d \r\n", (int)updated_fw_size, (int)uSD_fw_file_size);
		}
		else {
			printf("ERROR\tCould not write new app chunk to FLASH\r\n");
			flash_write_success = false;
			break;
		}

	}

	// Jump to app (old or new if updated) -----------------
	if (flash_write_success) {
		printf("SUCCESS\tWrote test buffer to IFLASH\r\n");
		goto_application();
	}
	else {
		printf("FAILURE\tErrors writing test buffer to IFLASH\r\n");
	}

	/* Should never reach here */
	Error_Handler();
}

