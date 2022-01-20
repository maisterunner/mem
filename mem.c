#include "mem.h"
#include "main.h"


/*
*
* F4 functions
*
*/

#ifdef STM32F4xx_HAL_H

uint32_t MEM_GetSector_F4( uint32_t Address ){

	/* DEFINE the SECTORS according to your reference manual
	 * STM32F405RE have:
	 *  Sector 0 to Sector 3 each 16KB
	 *  Sector 4 as 64KB
	 *  Sector 5 to Sector 11 each 128KB
	 *
	 * F405 has 1Mb of FLASH
	 */

	uint32_t sector = 0;

	if((Address < 0x08003FFF) && (Address >= 0x08000000))
	{
		sector = FLASH_SECTOR_0;									// 16K
	}
	else if((Address < 0x08007FFF) && (Address >= 0x08004000))
	{
		sector = FLASH_SECTOR_1;									// 32K
	}
	else if((Address < 0x0800BFFF) && (Address >= 0x08008000))
	{
		sector = FLASH_SECTOR_2;									// 48K
	}
	else if((Address < 0x0800FFFF) && (Address >= 0x0800C000))
	{
		sector = FLASH_SECTOR_3;									// 64K
	}
	else if((Address < 0x0801FFFF) && (Address >= 0x08010000))
	{
		sector = FLASH_SECTOR_4;									// 128K
	}
	else if((Address < 0x0803FFFF) && (Address >= 0x08020000))
	{
		sector = FLASH_SECTOR_5;									// 256K
	}
	else if((Address < 0x0805FFFF) && (Address >= 0x08040000))
	{
		sector = FLASH_SECTOR_6;									// 384K
	}
	else if((Address < 0x0807FFFF) && (Address >= 0x08060000))
	{
		sector = FLASH_SECTOR_7;									// 512K
	}
	else if((Address < 0x0809FFFF) && (Address >= 0x08080000))
	{
		sector = FLASH_SECTOR_8;									// 640K
	}
	else if((Address < 0x080BFFFF) && (Address >= 0x080A0000))
	{
		sector = FLASH_SECTOR_9;									// 768K
	}
	else if((Address < 0x080DFFFF) && (Address >= 0x080C0000))
	{
		sector = FLASH_SECTOR_10;									// 896K
	}
	else if((Address < 0x080FFFFF) && (Address >= 0x080E0000))
	{
		sector = FLASH_SECTOR_11;									// 1M
	}

  return sector;
}


uint32_t MEM_Struct_Init_F4( MEM_struct *input, uint32_t start_sector ){
	
	/* Initialize structure for FLASH
	* FLASH data may be separated into different regions
	* different areas may be used for different data
	* IF flash is to be separated multiple structures must be defined
	*
	*/
	
	// Set initial structure values
	input->current_sector_addr = start_sector;
	input->current_sector = MEM_GetSector_F4(input->current_sector_addr);
	input->end_sector_addr = input->current_sector_addr;
	input->end_sector = MEM_GetSector_F4(input->end_sector_addr);

	// Clear the flash where data is to be stored
	uint32_t out;
	out = MEM_Clear_Flash_F4();
	
	// return
	return out;

}


uint32_t MEM_Clear_Flash_F4( void ){

	/* Clear the sectors that are going to be used for storage
	* Sectors 4-11 are 128 kB sectors
	*
	* Sectors 0-3 are reserved for program
	*/

	// Initialize vars
	static FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t Sector_Error;

	// Unlock flash to enable write operation
	HAL_FLASH_Unlock();

	// Fill EraseInit structure
	EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.Sector        = 4;
	EraseInitStruct.NbSectors     = 8;

	if( HAL_FLASHEx_Erase(&EraseInitStruct, &Sector_Error) != HAL_OK ){
		return HAL_FLASH_GetError();
	}

	// Lock flash for protection
	HAL_FLASH_Lock();

	return 0;

}


uint32_t MEM_Write_Word_Flash_F4( MEM_struct *input, uint32_t *data, uint16_t num_words ){
	
	// Initialize vars
	int16_t i = 0;
	
	// Unlock flash to enable write operation
	HAL_FLASH_Unlock();
	
	// Get start and last sector for data
	input->current_sector = MEM_GetSector_F4(input->current_sector_addr);
	input->end_sector_addr = input->current_sector_addr + num_words*4;
	input->end_sector = MEM_GetSector_F4(input->end_sector_addr);
	
	// Write into the FLASH area defined by input.current_sector and input.end_sector
	while( i < num_words ){
		if( HAL_FLASH_Program( FLASH_TYPEPROGRAM_WORD, input->current_sector_addr, data[i] ) == HAL_OK ){
			input->current_sector_addr += 4;		// use +2 for 16bit +8 for 64bit
			i++;
		}
		else{
			// Error while writing data in FLASH
			return HAL_FLASH_GetError();
		}
	}
	
	// Lock flash for protection
	HAL_FLASH_Lock();
	
	return 0;
}


uint32_t MEM_Write_Char_Flash_F4( MEM_struct *input, char *data, uint16_t num_chars ){

	// Initialize vars
	int16_t i = 0;

	// Unlock flash to enable write operation
	HAL_FLASH_Unlock();

	// Get start and last sector for data
	input->current_sector = MEM_GetSector_F4(input->current_sector_addr);
	input->end_sector_addr = input->current_sector_addr + num_chars;
	input->end_sector = MEM_GetSector_F4(input->end_sector_addr);

	// Write into the FLASH area defined by input.current_sector and input.end_sector
	while( i < num_chars ){
		if( HAL_FLASH_Program( FLASH_TYPEPROGRAM_BYTE, input->current_sector_addr, data[i] ) == HAL_OK ){
			input->current_sector_addr += 1;		// use +2 for 16bit +8 for 64bit
			i++;
		}
		else{
			// Error while writing data in FLASH
			return HAL_FLASH_GetError();
		}
	}

	// Lock flash for protection
	HAL_FLASH_Lock();

	return 0;
}


void MEM_Read_Word_Flash( MEM_struct *input, uint32_t *out_data, uint16_t num_words ){
	
	// Read data
	do{
		*out_data = *(__IO uint32_t *)input->current_sector_addr;
		input->current_sector_addr += 4;
		out_data++;
		num_words--;
	}while( num_words > 0 );
}


void MEM_Read_Char_Flash( MEM_struct *input, char *out_data, uint16_t num_chars ){

	// Read data
	do{
		*out_data = *(__IO char *)input->current_sector_addr;
		input->current_sector_addr += 1;
		out_data++;
		num_chars--;
	}while( num_chars > 0 );
}


#endif

/*
*
* H7 functions
*
*/

#ifdef STM32H7xx_HAL_H

uint32_t MEM_GetSector_H7( uint32_t Address ){
	
	/*
	* Flash is divided up to 2 Banks
	* Each bank has a size of 128KB
	* Each Bank contains 8 Sectors
	* Total Flash size is 2MB
	*/

	uint32_t sector = 0;

	/* BANK 1 */
	if((Address >= 0x08000000) && (Address < 0x08020000))
	{
		sector = FLASH_SECTOR_0;
	}

	else if((Address >= 0x08020000) && (Address < 0x08040000))
	{
		sector = FLASH_SECTOR_1;
	}

	else if((Address >= 0x08040000) && (Address < 0x08060000))
	{
		sector = FLASH_SECTOR_2;
	}

	else if((Address >= 0x08060000) && (Address < 0x08080000))
	{
		sector = FLASH_SECTOR_3;
	}

	else if((Address >= 0x08080000) && (Address < 0x080A0000))
	{
		sector = FLASH_SECTOR_4;
	}

	else if((Address >= 0x080A0000) && (Address < 0x080C0000))
	{
		sector = FLASH_SECTOR_5;
	}

	else if((Address >= 0x080C0000) && (Address < 0x080E0000))
	{
		sector = FLASH_SECTOR_6;
	}

	else if((Address >= 0x080E0000) && (Address < 0x08100000))
	{
		sector = FLASH_SECTOR_7;
	}


	/* BANK 2 */
	else if((Address >= 0x08100000) && (Address < 0x08120000))
	{
		sector = FLASH_SECTOR_0;
	}

	else if((Address >= 0x08120000) && (Address < 0x08140000))
	{
		sector = FLASH_SECTOR_1;
	}

	else if((Address >= 0x08140000) && (Address < 0x08160000))
	{
		sector = FLASH_SECTOR_2;
	}

	else if((Address >= 0x08160000) && (Address < 0x08180000))
	{
		sector = FLASH_SECTOR_3;
	}

	else if((Address >= 0x08180000) && (Address < 0x081A0000))
	{
		sector = FLASH_SECTOR_4;
	}

	else if((Address >= 0x081A0000) && (Address < 0x081C0000))
	{
		sector = FLASH_SECTOR_5;
	}

	else if((Address >= 0x081C0000) && (Address < 0x081E0000))
	{
		sector = FLASH_SECTOR_6;
	}

	else if((Address >= 0x081E0000) && (Address < 0x08200000))
	{
		sector = FLASH_SECTOR_7;
	}

  return sector;
}


uint32_t MEM_Struct_Init_H7( MEM_struct *input, uint32_t start_sector ){
	
	/* Initialize structure for FLASH
	* FLASH data may be separated into different regions
	* different areas may be used for different data
	* IF flash is to be separated multiple structures must be defined
	*
	*/
	
	// Set initial structure values
	input->current_sector_addr = start_sector;
	input->current_sector = MEM_GetSector_H7(input->current_sector_addr);
	input->end_sector_addr = input->current_sector_addr;
	input->end_sector = MEM_GetSector_H7(input->end_sector_addr);

	// Clear the flash where data is to be stored
	uint32_t out;
	out = MEM_Clear_Flash_H7();
	
	// return
	return out;

}


uint32_t MEM_Clear_Flash_H7( void ){

	/* Clear the sectors that are going to be used for storage
	* Sectors 4-11 are 128 kB sectors
	*
	* Sectors 0-3 are reserved for program
	*/

	// Initialize vars
	static FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t Sector_Error;

	// Unlock flash to enable write operation
	HAL_FLASH_Unlock();

	// Fill EraseInit structure
	EraseInitStruct.TypeErase     	= FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange  	= FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.Banks     		= FLASH_BANK_1;
	EraseInitStruct.Sector        	= 1;
	EraseInitStruct.NbSectors     	= 7;

	if( HAL_FLASHEx_Erase(&EraseInitStruct, &Sector_Error) != HAL_OK ){
		return HAL_FLASH_GetError();
	}
	
	// Fill EraseInit structure
	EraseInitStruct.TypeErase     	= FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange  	= FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.Banks     		= FLASH_BANK_1;
	EraseInitStruct.Sector        	= 2;
	EraseInitStruct.NbSectors     	= 8;

	if( HAL_FLASHEx_Erase(&EraseInitStruct, &Sector_Error) != HAL_OK ){
		return HAL_FLASH_GetError();
	}

	// Lock flash for protection
	HAL_FLASH_Lock();

	return 0;

}


uint32_t MEM_Write_Word_Flash_H7( MEM_struct *input, char *data, uint16_t num_words ){

	// Initilaize vars
	int16_t i = 0;

	// Unlock the Flash to enable write operation
	HAL_FLASH_Unlock();


	// Get start and last sector for data
	input->current_sector = MEM_GetSector_H7(input->current_sector_addr);
	input->end_sector_addr = input->current_sector_addr + num_words*4;
	input->end_sector = MEM_GetSector_H7(input->end_sector_addr);
	if (input->current_sector_addr < 0x08100000){
		input->current_bank = FLASH_BANK_1;
	}
	else{
		input->current_bank = FLASH_BANK_2;
	}
	
	if (input->end_sector_addr < 0x08100000){
		input->end_bank = FLASH_BANK_1;
	}
	else{
		input->end_bank = FLASH_BANK_2;
	}


	// Write into the FLASH area defined by input.current_sector and input.end_sector
	while( i < num_words ){
		if( HAL_FLASH_Program( FLASH_TYPEPROGRAM_FLASHWORD, input->current_sector_addr, data[i] ) == HAL_OK ){
			input->current_sector_addr += 4;		// use +2 for 16bit +8 for 64bit
			i++;
		}
		else{
			// Error while writing data in FLASH
			return HAL_FLASH_GetError();
		}
	}

	// Lock flash for protection
	HAL_FLASH_Lock();

	return 0;
}


uint32_t MEM_Write_Char_Flash_H7( MEM_struct *input, char *data, uint16_t num_chars ){

	// Initilaize vars
	int16_t i = 0;

	// Unlock the Flash to enable write operation
	HAL_FLASH_Unlock();


	// Get start and last sector for data
	input->current_sector = MEM_GetSector_H7(input->current_sector_addr);
	input->end_sector_addr = input->current_sector_addr + num_chars;
	input->end_sector = MEM_GetSector_H7(input->end_sector_addr);
	if (input->current_sector_addr < 0x08100000){
		input->current_bank = FLASH_BANK_1;
	}
	else{
		input->current_bank = FLASH_BANK_2;
	}
	
	if (input->end_sector_addr < 0x08100000){
		input->end_bank = FLASH_BANK_1;
	}
	else{
		input->end_bank = FLASH_BANK_2;
	}


	// Write into the FLASH area defined by input.current_sector and input.end_sector
	while( i < num_chars ){
		if( HAL_FLASH_Program( 0, input->current_sector_addr, data[i] ) == HAL_OK ){
			input->current_sector_addr += 1;		// use +2 for 16bit +8 for 64bit
			i++;
		}
		else{
			// Error while writing data in FLASH
			return HAL_FLASH_GetError();
		}
	}

	// Lock flash for protection
	HAL_FLASH_Lock();

	return 0;
}

#endif



/*
*
* SD functions
*
*/

#ifdef __fatfs_H


FRESULT MEM_SD_Initialize(void){

	FRESULT stat;

	stat = FR_OK;

	  // Re-initialize SD
	  if ( BSP_SD_Init() != MSD_OK ) {
	    return FR_NOT_READY;
	  }

	  // Re-initialize FATFS
	  if ( FATFS_UnLinkDriver(SDPath) != 0 ) {
	    return FR_NOT_READY;
	  }
	  if ( FATFS_LinkDriver(&SD_Driver, SDPath) != 0 ) {
	    return FR_NOT_READY;
	  }

	  return stat;
}


FRESULT MEM_SD_AppendToFile(char* path, size_t path_len, char* msg, size_t msg_len) {

	/* Write to the end of a file in SD card
	* path ex.: "/PYLG.TXT"
	* path_len: strlen(path_len)
	* msg: char array written, msg_len: strlen(msg)
	*/

	FRESULT stat;
	FATFS fs;
	FIL myFILE;
	UINT testByte;

   // Bounds check on strings
   if ( (path[path_len] != 0) || (msg[msg_len] != 0) ) {
		return FR_INVALID_NAME;
   }

	// Mount filesystem
	stat = f_mount(&fs, SDPath, 0);
	if (stat != FR_OK) {
    f_mount(0, SDPath, 0);
    return stat;
	}

	// Open file for appending
	stat = f_open(&myFILE, path, FA_WRITE | FA_OPEN_APPEND | FA_OPEN_ALWAYS);
	if (stat != FR_OK) {
    f_mount(0, SDPath, 0);
    return stat;
	}

	// Write message to end of file
	stat = f_write(&myFILE, msg, msg_len, &testByte);
	if (stat != FR_OK) {
    f_mount(0, SDPath, 0);
    return stat;
	}

	// Sync, close file, unmount
	stat = f_close(&myFILE);
	f_mount(0, SDPath, 0);

	return stat;
}

#endif







