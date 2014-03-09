//-----------------------------------------------------------------------
// PFF - Low level disk control module for AT                     
//-----------------------------------------------------------------------

#include <avr/io.h>
#include "diskio.h"
//#include "hwdep.h"

#define _USE_WRITE 1
// Definitions for MMC/SDC connection
/*#define SD_DI   5   // MOSI *
#define SD_DO   6   // MISO *
#define SD_CLK  7   // CLK *
#define SD_CS   4   // SS *
*/
//#define SD_INS  0   // CD
//#define SD_WP   1   // WP
#define INIT_TIMEOUT 10000 //*100us

// Definitions for MMC/SDC command 
#define CMD0	(0x40+0)	// GO_IDLE_STATE 
#define CMD1	(0x40+1)	// SEND_OP_COND (MMC) 
#define	ACMD41	(0xC0+41)	// SEND_OP_COND (SDC) 
#define CMD8	(0x40+8)	// SEND_IF_COND 
#define CMD16	(0x40+16)	// SET_BLOCKLEN 
#define CMD17	(0x40+17)	// READ_SINGLE_BLOCK 
#define CMD24	(0x40+24)	// WRITE_BLOCK 
#define CMD55	(0x40+55)	// APP_CMD
#define CMD58	(0x40+58)	// READ_OCR 


// Port Controls (Platform dependent) 
//#define SELECT()	SDCS_PORT &= ~_BV(SDCS)		// MMC CS = L 
//#define	DESELECT()	SDCS_PORT |=  _BV(SDCS)		// MMC CS = H 
#define	MMC_SEL		!(CS_PORT & CS_SD)	// MMC CS status (true:selected) 

//#define	INIT_SPI()	{ PORTC=_BV(SD_CS)|_BV(SD_DO)|_BV(SD_DI)|_BV(SD_WP)|_BV(SD_INS); DDRB=_BV(SD_CS)|_BV(SD_DI)|_BV(SD_CLK); }
	

// Счётчик байтов
//extern BYTE Counter;

//-----------------------------------------------------------------------
//   Module Private Function
//-----------------------------------------------------------------------
static BYTE CardType;

//-----------------------------------------------------------------------
// SPI functions
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Deselect the card and release SPI bus                                 
//-----------------------------------------------------------------------
static void release_spi( void ) {

//	spiread();
	spiswitch(CS_NONE);
	spiread();
}


//-----------------------------------------------------------------------
// Send a command packet to MMC                                          
//-----------------------------------------------------------------------
static BYTE send_cmd (
	BYTE cmd,		// Command byte 
	DWORD arg		// Argument 
    ) {

	BYTE n, res;

	// ACMD<n> is the command sequense of CMD55-CMD<n> 
    if ( cmd & 0x80 ) {	
		
        cmd &= 0x7F;
		res = send_cmd( CMD55, 0 );
		if ( res > 1 ) return res;

	}

	// Select the card 
//	SPI_PORT &= ~_BV(SCLK);
//	SPI_PORT |= _BV(SCLK);
	spiswitch(CS_NONE);
	spiread();
	spiswitch(CS_SD);
	spiread();

	// Send a command packet 
	spiwrite(cmd);						// Start + Command index 
	spiwrite(( BYTE )( arg >> 24 ));		// Argument[31..24] 
	spiwrite(( BYTE )( arg >> 16 ));		// Argument[23..16] 
	spiwrite(( BYTE )( arg >> 8 ));		// Argument[15..8] 
	spiwrite(( BYTE ) arg);				// Argument[7..0] 
	
    n = 0x01;							// Dummy CRC + Stop 
	
    if ( cmd == CMD0 ) n = 0x95;			// Valid CRC for CMD0(0) 
	if ( cmd == CMD8 ) n = 0x87;			// Valid CRC for CMD8(0x1AA) 
	
    spiwrite(n);

	// Receive a command response 
	n = 100;								// Wait for a valid response in timeout of 10 attempts 

    do {
		
        res = spiread();

	} while ( ( res & 0x80 ) && --n );

	// Return with the response value 
    return res;			

}

//--------------------------------------------------------------------------
//
//   Public Functions
//
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// Initialize Disk Drive                                                 
//--------------------------------------------------------------------------
DSTATUS disk_initialize( void ) {

	BYTE n, cmd, ty, ocr[4];
	WORD tmr;

//	INIT_SPI();
//	SDCS_DDR |= _BV(SDCS);
	spiswitch(CS_NONE);  

//	if ( ( PINB & _BV( SD_INS ) ) != 0x00 ) return STA_NOINIT;//TODO check without adds pin

#if _USE_WRITE
	// Finalize write process if it is in progress 
    if ( MMC_SEL ) disk_writep( 0, 0 );
#endif

	// Dummy clocks 
    for ( n = 100; n; n-- ) spiread();	

//	spiswitch(CS_SD); 
	ty = 0;

	// Enter Idle state 
    if (send_cmd(CMD0, 0) == 1)
	{

		// SDv2 
        if ( send_cmd( CMD8, 0x1AA ) == 1 ) {	

			// Get trailing return value of R7 resp 
            for ( n = 0; n < 4; n++ ) ocr[n] = spiread();		

			// The card can work at vdd range of 2.7-3.6V 
            if ( ocr[2] == 0x01 && ocr[3] == 0xAA ) {		

				// Wait for leaving idle state (ACMD41 with HCS bit) 
                for ( tmr = 12000; tmr && send_cmd( ACMD41, 1UL << 30 ); tmr-- ) ;	
				
                // Check CCS bit in the OCR 
                if ( tmr && send_cmd( CMD58, 0 ) == 0 ) {		

					for ( n = 0; n < 4; n++ ) ocr[n] = spiread();

					// SDv2 (HC or SC)
                    ty = ( ocr[0] & 0x40 ) ? CT_SD2 | CT_BLOCK : CT_SD2;	

				}

			}

		// SDv1 or MMCv3 
        } else {		

			if ( send_cmd( ACMD41, 0 ) <= 1 ) {

				ty = CT_SD1; 
                cmd = ACMD41;	// SDv1 

			} else {

				ty = CT_MMC; 
                cmd = CMD1;	// MMCv3 
			}

			// Wait for leaving idle state 
            for ( tmr = 25000; tmr && send_cmd( cmd, 0 ); tmr-- ) ;	

			// Set R/W block length to 512
            if ( !tmr || send_cmd( CMD16, 512 ) != 0 ) ty = 0;

		}

	}

	CardType = ty;

	release_spi();

	return ty ? 0 : STA_NOINIT;

}


//-----------------------------------------------------------------------
// Read partial sector                                                   
//-----------------------------------------------------------------------

DRESULT disk_readp (
	BYTE *buff,		// Pointer to the read buffer (NULL:Read bytes are forwarded to the stream) 
	DWORD lba,		// Sector number (LBA) 
	WORD ofs,		// Byte offset to read from (0..511) 
	WORD cnt		// Number of bytes to read (ofs + cnt mus be <= 512) 
    ) {
    
    DRESULT res;
	BYTE rc;
	WORD bc;

	BYTE ch,in;

//	if ( ( PINB & _BV( SD_INS ) ) != 0x00 ) return RES_ERROR;//TODO check

	// Convert to byte address if needed
    if ( !( CardType & CT_BLOCK ) ) lba *= 512;		

	res = RES_ERROR;

	// READ_SINGLE_BLOCK 
    if ( send_cmd( CMD17, lba ) == 0 ) {

		bc = 30000;

		// Wait for data packet in timeout of 100ms 
        do {							

			rc = spiread();

		} while ( rc == 0xFF && --bc );

		// A data packet arrived 
        if ( rc == 0xFE ) {				

			res = RES_OK;

            bc = 514 - ofs - cnt;

			// Skip leading bytes 
			if ( ofs ) {

				do spiread(); while ( --ofs );
			}

			// Receive a part of the sector 
			// Store data to the memory 
            if ( buff ) {	
				do
					* buff++ = spiread();

				while ( --cnt );

			// Forward data to the outgoing stream (depends on the project) 
            } else {	

				in=0;
                do {
//**************************Read Prog***************************************
						ch =spiread();
/*uart_putchar('\r');
uart_putchar('+');
uart_putchar(':');
uart_putchar(ch);
uart_putchar(':');
uart_put_dec(ch);
uart_putchar('\r');
*/
						if(ch==0 || res){
							while(--cnt) spiread();
							break;
						}
						if(ch=='\n'){
							CmdInp[in] = 0;
							in=0;
							if(!(res = LoadPrgLine(0)))
								CLine++;
						}
						else{
							CmdInp[in] = ch;
							if(in<(BMAX-1))	in++;
						}
//*******************************************************************
                } while ( --cnt );

			}

			// Skip trailing bytes and CRC 
			do spiread(); while (--bc);

		}

	}

	release_spi();

	return res;

}

#if _USE_WRITE
//-----------------------------------------------------------------------
// Write partial sector                                                  
//-----------------------------------------------------------------------


DRESULT disk_writep (
	const BYTE *buff,	// Pointer to the bytes to be written (NULL:Initiate/Finalize sector write) 
	DWORD sa			// Number of bytes to send, Sector number (LBA) or zero 
    ) {

	DRESULT res;
	WORD bc;
	static WORD wc;
	BYTE ch;

//	if ( ( PINB & _BV( SD_INS ) ) != 0x00 ) return RES_ERROR; //TODO
//	if ( ( PINB & _BV( SD_WP ) ) != 0x00 ) return RES_ERROR;

	res = RES_ERROR;

	// Send data bytes 
    if ( buff ) {		

		bc = ( WORD ) sa;
	
        // Send data bytes to the card 
        while ( bc && wc ) {
//*****************************************************************	
			if(PrgLineP){
				ch = CmdInp[CLine];
				if(ch != 0){
					spiwrite(ch);
					CLine++;
				}else{
					spiwrite('\n');
					CLine=0;
				    PrgLineP = PrgLineP->next;
					if(PrgLineP) PrepToSavPrgLine(PrgLineP);
				}
			}
			else spiwrite(0);
//***************************************************************
//			spiwrite( * buff++);
			wc--; 
            bc--;

		}
		res = RES_OK;

	} else {

		// Initiate sector write process 
        if ( sa ) {	

			// Convert to byte address if needed 
            if ( !( CardType & CT_BLOCK ) ) sa *= 512;	

			// WRITE_SINGLE_BLOCK 
            if ( send_cmd( CMD24, sa ) == 0) {			

				// Data block header 
                spiwrite( 0xFF );
                spiwrite( 0xFE );		
				
                // Set byte counter 
                wc = 512;							
				res = RES_OK;

			}

		// Finalize sector write process 
        } else {	

			bc = wc + 2;

			// Fill left bytes and CRC with zeros 
            while ( bc-- ) spiwrite(0);	

			// Receive data resp and wait for end of write process in timeout of 300ms 
            if ( ( spiread() & 0x1F ) == 0x05 ) {

				// Wait ready
                for ( bc = 65000; spiread() != 0xFF && bc; bc-- );

				if ( bc ) res = RES_OK;

			}

			release_spi();

		}

	}

	return res;

}

#endif
