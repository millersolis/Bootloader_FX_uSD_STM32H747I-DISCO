/*
 * sd_fw_update.c
 *
 *  Created on: Jul 30, 2023
 *      Author: mille
 */


#include "sd_card.h"

#include <stdio.h>	// TODO: Replace with type-safe printf for embedded
#include "tx_api.h"
#include "fx_api.h"
#include "fx_stm32_sd_driver.h"
#include "main.h"
#include "utils.h"

#define SD_DETECT_Pin			uSD_Detect_Pin
#define SD_DETECT_GPIO_Port		uSD_Detect_GPIO_Port

#define SD_WRITE_BUF_SIZE			(60)

typedef enum {
CARD_STATUS_CHANGED             = 99,
CARD_STATUS_DISCONNECTED        = 88,
CARD_STATUS_CONNECTED           = 77
} SD_ConnectionStateTypeDef;


TX_SEMAPHORE			sd_detect_semaphore;

/* Define FileX global data structures.  */
static FX_MEDIA        	sdio_disk;
static FX_FILE         	fx_file;

/* Buffer for FileX FX_MEDIA sector cache. */
ALIGN_32BYTES (uint32_t fx_sd_media_memory[FX_STM32_SD_DEFAULT_SECTOR_SIZE / sizeof(uint32_t)]);

static ULONG last_status = CARD_STATUS_DISCONNECTED;
static UINT sd_status;

CHAR data[] = "This is a LOG test ";

/* Buffer to be written to SD. Can only be read by the SD thread
 * and can only be written to by a single thread (FileX app thread currently)
 */
CHAR sd_write_buf[SD_WRITE_BUF_SIZE];


void sd_update_card_state(void);
void sd_fx_start(void);
void sd_fx_stop(void);
UINT sd_fx_file_create(char* filename);
UINT sd_fx_file_write(char* filename);
int32_t SD_IsDetected(uint32_t Instance);


/**
  * @brief  EXTI line detection callback.
  * @param  GPIO_Pin: Specifies the port pin connected to corresponding EXTI line.
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == SD_DETECT_Pin) {
	  tx_semaphore_put(&sd_detect_semaphore);
  }
}

void sd_init(void)
{
	if (tx_semaphore_create(&sd_detect_semaphore, "sd detect semaphore", 0) != TX_SUCCESS) {
		assert(0);	// TODO: Handle error
	}

	/* Check initial status of card */
	if(SD_IsDetected(FX_STM32_SD_INSTANCE) == SD_PRESENT) {
		last_status = CARD_STATUS_CONNECTED;
		/* SD card is already inserted, place the info into the queue */
		tx_semaphore_put(&sd_detect_semaphore);
	 }
}

void sd_fx_start(void)
{
	/* Open the SD disk driver */
	sd_status =  fx_media_open(&sdio_disk,
								FX_SD_VOLUME_NAME,
								fx_stm32_sd_driver,
								(VOID *)FX_NULL,
								(VOID *) fx_sd_media_memory,
								sizeof(fx_sd_media_memory));
}

void sd_fx_stop(void)
{
	/* Close the media.  */
	sd_status =  fx_media_close(&sdio_disk);
}

void sd_update_card_state(void)
{
	/* Check semaphore to update card status if needed */
	if (tx_semaphore_get(&sd_detect_semaphore, TX_NO_WAIT) == TX_SUCCESS) {
		/* for debouncing purpose we wait a bit till it settles down */
		tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 2);

		if (SD_IsDetected(FX_STM32_SD_INSTANCE) == SD_PRESENT) {
			last_status = CARD_STATUS_CONNECTED;
			sd_fx_start();
		}
		else {
			last_status = CARD_STATUS_DISCONNECTED;
			sd_fx_stop();
		}
	}
}

/* To be called by a single thread (FileX app thread currently)
 * because it writes to the sd_write_buf without mutex.
 */
void sd_print_to_file(char* filename, uint8_t* buf, uint8_t len /*, TX_SEMAPHORE* sem_notify*/)
{
	uint8_t sd_write_buf_len = sizeof(sd_write_buf);

	// Truncate buf if its length is bigger than sd_write_buf
	copy_mem((void*)buf, (void*)sd_write_buf, U_MIN(len, sd_write_buf_len));
	sd_write_buf[sd_write_buf_len -1] = '\0';	// Ensure buf is null terminated to use strlen later

	// Write buffer to file
	sd_fx_file_write(filename);
}	// UNUSED FOR NOW (not tested) [Miller]

/* To be called by a single thread (FileX app thread currently)
 * because it writes to the sd_write_buf without mutex.
 */
void sd_print_to_file_CR(char* filename, uint8_t* buf, uint8_t len /*, TX_SEMAPHORE* sem_notify*/)
{
	uint8_t idx = 0;
	uint8_t sd_write_buf_len = sizeof(sd_write_buf);

	// Append CR at the beginning of sd write buf
	sd_write_buf[idx++] = '\r';
	sd_write_buf[idx++] = '\n';

	// Truncate buf if its length is bigger than sd_write_buf
	copy_mem((void*)buf, (void*)&sd_write_buf[idx], U_MIN(len, sd_write_buf_len - idx));
	sd_write_buf[sd_write_buf_len -1] = '\0';	// Ensure buf is null terminated to use strlen later

	// Write buffer to file
	sd_fx_file_write(filename);
}	// UNUSED FOR NOW (not tested) [Miller]

/* To be called by a single thread (FileX app thread currently)
 * because it writes to the sd_write_buf without mutex.
 */
void sd_create_file(char* filename /*, TX_SEMAPHORE* sem_notify*/)
{
	sd_update_card_state();

	UINT status;
	if (last_status == CARD_STATUS_CONNECTED) {
		status = sd_fx_file_create(filename);
		if (status != FX_SUCCESS) {
			printf("ERROR\tFX error while creating file in uSD\r\n");
		}
	}
	// TODO: Error handling
	printf("ERROR\tCould not create file in uSD\r\n");
}

UINT sd_fx_file_create(char* filename)
{
	UINT status;
	/* Create a file called TEST.TXT in the root directory. */
	status = fx_file_create(&sdio_disk, filename);
	/* Check the create status. */
	if ((status == FX_SUCCESS) || (status == FX_ALREADY_CREATED)) {
		/* File Creation success. */
		status = FX_SUCCESS;
	}
	else {
		/* File Creation Fail. */
		status = FX_INVALID_STATE;
	}

	return status;
}

UINT sd_fx_file_write(char* filename)
{
	UINT status;
    /* Open the test file.  */
    status =  fx_file_open(&sdio_disk, &fx_file, filename, FX_OPEN_FOR_WRITE);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {
      /* Error opening file, call error handler.  */
      Error_Handler();
    }

    /* Seek end of test file offset */
    status = fx_file_relative_seek(&fx_file, 0, FX_SEEK_END);

    /* Check the file seek status.  */
	if (status != FX_SUCCESS)
	{
	/* Error performing file seek, call error handler.  */
	Error_Handler();
	}

	/* Write a string to the test file.  */
	status =  fx_file_write(&fx_file, sd_write_buf, str_len(sd_write_buf));

	/* Check the file write status.  */
	if (status != FX_SUCCESS)
	{
	  /* Error writing to a file, call error handler.  */
	  Error_Handler();
	}
	/* Close the test file.  */
	sd_status =  fx_file_close(&fx_file);

	/* Check the file close status.  */
	if (sd_status != FX_SUCCESS)
	{
	  /* Error closing the file, call error handler.  */
	  Error_Handler();
	}

	sd_status = fx_media_flush(&sdio_disk);

	/* Check the media flush  status.  */
	if (sd_status != FX_SUCCESS)
	{
	  /* Error closing the file, call error handler.  */
	  Error_Handler();
	}

	return status;
}

/**
 * @brief  Detects if SD card is correctly plugged in the memory slot or not.
 * @param Instance  SD Instance
 * @retval Returns if SD is detected or not
 */
int32_t SD_IsDetected(uint32_t Instance)
{
  int32_t ret;
  if(Instance >= 1) {
    ret = HAL_ERROR;
  }
  else {
    /* Check SD card detect pin */
    if (HAL_GPIO_ReadPin(SD_DETECT_GPIO_Port, SD_DETECT_Pin) == GPIO_PIN_SET) {
      ret = SD_NOT_PRESENT;
    }
    else {
      ret = SD_PRESENT;
    }
  }

  return(int32_t)ret;
}
