/*
It is an open source software to implement SD routines to
small embedded systems. This is a free software and is opened for education,
research and commercial developments under license policy of following trems.

(C) 2013 vinxru (aleksey.f.morozov@gmail.com)

It is a free software and there is NO WARRANTY.
No restriction on use. You can use, modify and redistribute it for
personal, non-profit or commercial use UNDER YOUR RESPONSIBILITY.
Redistributions of source code must retain the above copyright notice.

Version 0.99 5-05-2013
*/

#include "common.h"
#include <delay.h>
#include "sd.h"
#include "fs.h"

BYTE sd_sdhc; /* ������������ SDHC ����� */

/**************************************************************************
*  �������� SPI ��� ATMega8                                               *
*  ����� ���������� ��� ������ ��.                                        *
**************************************************************************/

/* ���� ���������� ����� CS ����� */
#define SD_CS_ENABLE    PORTB &= ~0x04;
#define SD_CS_DISABLE   PORTB |= 0x04;

/* ������������� � ������� �������� CodeVisionAVR */
#ifndef SPI2X
#define SPI2X 0
#endif

#define SPI_INIT      { SPCR = 0x52; SPSR = 0x00; }    
#define SPI_HIGHSPEED { SPCR = 0x50; SPSR |= (1<<SPI2X); delay_ms(1); }

static void spi_transmit(BYTE data) {
  SPDR = data;  
  while((SPSR & 0x80) == 0);
}

static BYTE spi_receive() {        
  SPDR = 0xFF;
  while((SPSR & 0x80) == 0);
  return SPDR;
}

/**************************************************************************
*  �������� �������                                                       *
**************************************************************************/

/* ������������ ������� SD ����� */

#define GO_IDLE_STATE      (0x40 | 0 )
#define SEND_IF_COND       (0x40 | 8 )
#define READ_SINGLE_BLOCK  (0x40 | 17)
#define WRITE_SINGLE_BLOCK (0x40 | 24)
#define SD_SEND_OP_COND    (0x40 | 41)
#define APP_CMD            (0x40 | 55)
#define READ_OCR           (0x40 | 58)

static BYTE sd_sendCommand(BYTE cmd, DWORD arg) {
  BYTE response, retry;
                  
  /* ���������� ����� ���� ��� -4 �������, ���� ����� ������ �������� */
  if(sd_sdhc == 0 && (cmd==READ_SINGLE_BLOCK || cmd==WRITE_SINGLE_BLOCK))  
    arg <<= 9;
  
  /* �������� ����� */
  SD_CS_ENABLE

  /* ��������� ������� */
  spi_transmit(cmd);
  spi_transmit(((BYTE*)&arg)[3]);
  spi_transmit(((BYTE*)&arg)[2]);
  spi_transmit(((BYTE*)&arg)[1]);
  spi_transmit(((BYTE*)&arg)[0]);

  /* ���� ������� ������� CRC. ��������� �� ������� ���������� ���, ������� �������� ��� */
  spi_transmit(cmd == SEND_IF_COND ? 0x87 : 0x95);

  /* ���� ������������� (256 ������) */  
  retry = 0;
  while((response = spi_receive()) == 0xFF) 
    if(++retry == 0) break;

  /* ��������� ������� READ_OCR ������������ ���, ��� ��� � ����� ���� ������� �� ������ CS � ���������� 1 ���� */
  if(response == 0 && cmd == READ_OCR) {  
    /* 32 ���� �� ������� ��� ���������� ���� ��� */
    sd_sdhc = spi_receive() & 0x40;
    spi_receive();
    spi_receive();
    spi_receive(); 
  }

  /* ��������� CS � ����� � 1 ����*/
  SD_CS_DISABLE
  spi_receive(); 

  return response;
}

/**************************************************************************
*  �������� ����������/������� �����                                      *
**************************************************************************/

BYTE sd_check() {
  BYTE i = 0;
  do { 
    sd_sendCommand(APP_CMD, 0);
    if(sd_sendCommand(SD_SEND_OP_COND, 0x40000000) == 0) return 0;
  } while(--i);
  return 1;
}

/**************************************************************************
*  ������������� ����� (��� ������� ���������� �������� sd_init)          *
**************************************************************************/

static BYTE sd_init_int() {
  BYTE i;

  /* ���������� SDHC ���� */
  sd_sdhc = 0;

  /* ������� 80 ������ ������ */
  for(i=10; i; --i)
    spi_receive();   

  /* CMD0 �������� ������� ������ */
  if(sd_sendCommand(GO_IDLE_STATE, 0) != 1) goto abort;

  /* CMD8 ������ ������ ����� */
  i = 0;
  if(sd_sendCommand(SEND_IF_COND, 0x000001AA)) 
    i = 1;

  /* CMD41 �������� ��������� ������������� */
  if(sd_check()) goto abort;

  /* ������ ��� ������ ������ ����� */
  if(i) {
    /* CMD58 ����������� SDHC �����. ����� �������������� � ������� sd_sendCommand */
    if(sd_sendCommand(READ_OCR, 0) != 0) goto abort;
  }
  
  return 0;    
abort:
  return 1;
}                            

/**************************************************************************
*  ������������� �����                                                    *
**************************************************************************/

BYTE sd_init() {  
  BYTE tries;

  /* ����������� CS �� ������ ������ */
  SD_CS_DISABLE

  /* �������� SPI */
  SPI_INIT

  /* ������ ��������� ������� ������������� */
  tries = 10;  
  while(sd_init_int()) 
    if(--tries == 0) {
      lastError = ERR_DISK_ERR;
      return 1;       
    }
          
  /* �������� ������������ �������� */
  SPI_HIGHSPEED     
  
  return 0;
}

/**************************************************************************
*  �������� ������������� ����� �� ����                                   *
**************************************************************************/

static BYTE sd_waitBus(BYTE byte) {
  WORD retry = 0;
  do {
    if(spi_receive() == byte) return 0;
  } while(++retry); 
  return 1;
}

/**************************************************************************
*  ������ ������������� ������� �������                                   *
**************************************************************************/

BYTE sd_read(BYTE* buffer, DWORD sector, WORD offsetInSector, WORD length) {
  BYTE b;
  WORD i;
    
  /* �������� ������� */
  if(sd_sendCommand(READ_SINGLE_BLOCK, sector)) goto abort;

  /* ����� �� ��������� CS, ��� �� ������� ����� ������� */
  SD_CS_ENABLE

  /* ���� ��������� ���� */
  if(sd_waitBus(0xFE)) goto abort;

  /* ��������� 512 ���� */
  for(i=512; i; --i) {
    b = spi_receive();
    if(offsetInSector) { offsetInSector--; continue; }
    if(length == 0) continue;
    length--;
    *buffer++ = b;
  }

  /* CRC ���������� */
  spi_receive();
  spi_receive();

  /* ��������� CS � ����� � 1 ����*/
  SD_CS_DISABLE
  spi_receive(); 

  /* �� */
  return 0;

  /* ������ � ��������� CS.*/
abort:
  SD_CS_DISABLE
  lastError = ERR_DISK_ERR;
  return 1;
}

/**************************************************************************
*  ������ ������� (512 ����)                                              *
**************************************************************************/

BYTE sd_write512(BYTE* buffer, DWORD sector) {
  WORD n;
  
  /* �������� ������� */
  if(sd_sendCommand(WRITE_SINGLE_BLOCK, sector)) goto abort;

  /* ����� �� ��������� CS, ��� �� ��������� ���� ������ */
  SD_CS_ENABLE

  /* �������� ��������� ���� */
  spi_transmit(0xFE);
  
  /* ������ */
  for(n=512; n; --n)    
    spi_transmit(*buffer++);
      
  /* CRC ������������ */
  spi_transmit(0xFF);
  spi_transmit(0xFF);

  /* ����� �� */
  if((spi_receive() & 0x1F) != 0x05) goto abort;
                    
  /* ���� ��������� ������, �.�. ���� �� ����������� ���� */
  if(sd_waitBus(0xFF)) goto abort;
  
  /* ��������� CS � ����� � 1 ����*/
  SD_CS_DISABLE
  spi_receive();

  /* �� */
  return 0;
              
  /* ������.*/
abort:  
  SD_CS_DISABLE 
  lastError = ERR_DISK_ERR;
  return 1;
}
