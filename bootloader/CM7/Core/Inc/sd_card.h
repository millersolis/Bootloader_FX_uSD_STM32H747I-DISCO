/*
 * sd_fw_update.h
 *
 *  Created on: Jul 30, 2023
 *      Author: mille
 */

#ifndef INC_SD_CARD_H_
#define INC_SD_CARD_H_

#include <stdint.h>
#include "tx_api.h"

#define FX_SD_VOLUME_NAME 				"STM32_SDIO_DISK"
#define SD_FILENAME_MAXSIZE				(25)

#define SD_PRESENT                      1UL
#define SD_NOT_PRESENT                  0UL


void sd_init(void);
void sd_print_to_file(char* filename, uint8_t* buf, uint8_t len);
void sd_print_to_file_CR(char* filename, uint8_t* buf, uint8_t len);
void sd_create_file(char* filename);

#endif /* INC_SD_CARD_H_ */
