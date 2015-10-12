// USED FOR ESP8266 BUILT-IN FLASH
/*
 * @author: Thorsten von Eicken
 */

//#include <c_types.h>
#include <user_interface.h>
#include <mem.h>
#include <osapi.h>
#include <espmissingincludes.h>

#define OTA_BUFF_SZ 1460


#include "platform_config.h"

#include "ff.h"
#include "diskio.h"

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
  BYTE drv /* Physical drive number (0) */
  )
{
  INT szMap = system_get_flash_size_map();
  return szMap > FLASH_SIZE_8M_MAP_512_512 ? RES_OK : RES_NOTRDY;
}

/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
  BYTE drv /* Physical drive number (0) */
  )
{
  INT szMap = system_get_flash_size_map();
  return szMap > FLASH_SIZE_8M_MAP_512_512 ? RES_OK : RES_NOTRDY;
}


/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
  BYTE drv, /* Physical drive number (0) */
  BYTE *buff, /* Pointer to the data buffer to store read data */
  DWORD sector, /* Start sector number (LBA) */
  UINT count /* Sector count */
  )
{
  os_printf("FAT: read [0x%lx,%d]->%p\n", sector, count, buff);
  sint8 err = spi_flash_read(FAT_BASE+sector, (uint32*)buff, count*4096);
  if (err != SPI_FLASH_RESULT_OK) return RES_ERROR;
  return RES_OK;
#if 0
  uint32 buf[4096/4];
  BYTE err;
  err = spi_flash_read(FAT_BASE+(sector>>8), buf, 4096);
  if (err != SPI_FLASH_RESULT_OK) return RES_ERROR;
  os_memcpy(buff, buf+(sector&0x7)*512, 512);
  return RES_OK;
#endif
}


/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
  BYTE drv, /* Physical drive number (0) */
  const BYTE *buff, /* Pointer to the data to be written */
  DWORD sector, /* Start sector number (LBA) */
  UINT count /* Sector count */
  )
{
  os_printf("FAT: write [0x%lx,%d]<-%p\n", sector, count, buff);
  sint8 err = spi_flash_write(FAT_BASE+sector, (uint32*)buff, count*4096);
  if (err != SPI_FLASH_RESULT_OK) return RES_ERROR;
  return RES_OK;
#if 0
  uint32 buf[4096/4];
  BYTE err;
  err = spi_flash_read(FAT_BASE+(sector>>8), buf, 4096);
  if (err != SPI_FLASH_RESULT_OK) return RES_ERROR;
  os_memcpy(buf+(sector&0x7)*512, buff, 512);
  err = spi_flash_write(FAT_BASE+(sector>>8), buf, 4096);
  if (err != SPI_FLASH_RESULT_OK) return RES_ERROR;
  return RES_OK;
#endif
}


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
  BYTE drv, // Physical drive number (0)
  BYTE ctrl, // Control code
  void *buff // Buffer to send/receive control data
  )
{
  DRESULT res = RES_OK;

  switch (ctrl) {
  case CTRL_SYNC : /// Make sure that no pending write process
    break;

  case GET_SECTOR_COUNT :   // Get number of sectors on the disk (DWORD)
    *(DWORD*)buff = 1024*1024/_MAX_SS; // 1MB
    break;

  case GET_SECTOR_SIZE :   // Get R/W sector size (WORD)
    *(WORD*)buff = 4096;
    break;

  case GET_BLOCK_SIZE :     // Get erase block size in unit of sector (DWORD)
    *(DWORD*)buff = 8;
    break;

  case CTRL_ERASE_SECTOR : {// Erase a sector
    DWORD start = ((DWORD*)buff)[0];
    DWORD end   = ((DWORD*)buff)[1];
    os_printf("FAT: erase 0x%lx..0x%lx\n", start, end);
    for (DWORD i=start; i<=end; i++) {
      sint8 err = spi_flash_erase_sector(FAT_BASE+i);
      if (err != SPI_FLASH_RESULT_OK) return RES_ERROR;
    }
                           }
  }

  return res;
}
