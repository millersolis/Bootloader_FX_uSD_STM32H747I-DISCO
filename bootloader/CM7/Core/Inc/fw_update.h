/*
 * fw_update.h
 *
 *  Created on: Jul 30, 2023
 *      Author: mille
 */

#ifndef INC_FW_UPDATE_H_
#define INC_FW_UPDATE_H_


// App address has to be aligned to a flash word -> 256-bit aligned
#define APP_FLASH_ADDR 0x08040000	// Bank1 - Sector 2 & Sector 3: Application (starting at 0x08040000)


void fw_update_thread_entry(void);

#endif /* INC_FW_UPDATE_H_ */
