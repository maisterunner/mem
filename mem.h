#ifndef MEM_H
#define MEM_H

#include "main.h"
// #include "fatfs.h"


/* ----------------
*
*	Defines
*
 ----------------- */

#define MEM_START_SECTOR_ADDR_F4					0x08020000 // write correct addr that can be written (128kB is given for program)


/* ----------------
*
*	Typedefs
*
 ----------------- */
 
typedef struct MEM_struct{
	uint32_t current_sector_addr;
	uint32_t current_sector;
	uint32_t end_sector_addr;
	uint32_t end_sector;
	uint8_t current_bank;
	uint8_t end_bank;
}MEM_struct;


/* ----------------
*
*	Function prototypes
*
 ----------------- */

// F4
#ifdef STM32F4xx_HAL_H

uint32_t MEM_GetSector_F4( uint32_t Address );
uint32_t MEM_Struct_Init_F4( MEM_struct *input, uint32_t start_sector );
uint32_t MEM_Clear_Flash_F4( void );
uint32_t MEM_Write_Word_Flash_F4( MEM_struct *input, uint32_t *data, uint16_t num_words );
uint32_t MEM_Write_Char_Flash_F4( MEM_struct *input, char *data, uint16_t num_chars );
void MEM_Read_Word_Flash( MEM_struct *input, uint32_t *out_data, uint16_t num_words );
void MEM_Read_Char_Flash( MEM_struct *input, char *out_data, uint16_t num_chars );

#endif


// H7
#ifdef STM32H7xx_HAL_H

uint32_t MEM_GetSector_H7( uint32_t Address );
uint32_t MEM_Struct_Init_H7( MEM_struct *input, uint32_t start_sector );
uint32_t MEM_Clear_Flash_H7( void );
uint32_t MEM_Write_Word_Flash_H7( MEM_struct *input, char *data, uint16_t num_words );
uint32_t MEM_Write_Char_Flash_H7( MEM_struct *input, char *data, uint16_t num_chars );

#endif


// SD
#ifdef __fatfs_H

FRESULT MEM_SD_Initialize(void);
FRESULT MEM_SD_AppendToFile(char* path, size_t path_len, char* msg, size_t msg_len);

#endif

// Endo of file


#endif
