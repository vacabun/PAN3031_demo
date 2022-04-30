/*******************************************************************************
 * @note Copyright (C) 2020 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file pan3031.c
 * @brief
 *
 * @history - V3.0, 2021-07-12
*******************************************************************************/
#include "stdio.h"
#include "stm32f0xx_hal.h"
#include "math.h"
#include "pan3031_port.h"
#include "pan3031.h" 
#include "radio.h" 
uint8_t RadioRxPayload[255];
uint8_t plhd_buf[16];

/**
 * @brief read one byte from register in current page
 * @param[in] <addr> register address to write
 * @return value read from register
 */
static uint8_t PAN3031_read_reg(uint8_t addr)
{ 
	uint8_t temreg = 0x00;  
	
	rf_port.spi_cs_low();                               
	rf_port.spi_readwrite(0x00 | (addr<<1));
	temreg=rf_port.spi_readwrite(0x00);          
	rf_port.spi_cs_high();          
	return temreg;   
} 

/**
 * @brief write global register in current page and chick
 * @param[in] <addr> register address to write
 * @param[in] <value> address value to write to rgister
 * @return result
 */
static uint32_t PAN3031_write_reg(uint8_t addr,uint8_t value)
{ 
	uint16_t tmpreg = 0;  
	uint16_t addr_w = (0x01 | (addr << 1));	
		
	rf_port.spi_cs_low();	  
	rf_port.spi_readwrite(addr_w);
	rf_port.spi_readwrite(value);	
	rf_port.spi_cs_high();	
	tmpreg = PAN3031_read_reg(addr);
	if(tmpreg == value)
	{
		return OK;
	}
	else
	{
		return FAIL;
	}	
} 

/**
 * @brief rf send data fifo,send bytes register
 * @param[in] <addr> register address to write
 * @param[in] <buffer> send data buffer
 * @param[in] <size> send data size
 * @return none
 */
static void PAN3031_write_fifo(uint8_t addr,uint8_t *buffer,int size)
{ 
	int i;
	uint8_t addr_w = (0x01 | (addr << 1));
	
	rf_port.spi_cs_low();	
	rf_port.spi_readwrite(addr_w);
	for(i =0;i<size;i++)
	{
		rf_port.spi_readwrite(buffer[i]);
	}
	rf_port.spi_cs_high();	
}

/**
 * @brief rf receive data fifo,read bytes from register
 * @param[in] <addr> register address to write
 * @param[in] <buffer> receive data buffer
 * @param[in] <size> receive data size
 * @return none
 */
static void PAN3031_read_fifo(uint8_t addr,uint8_t *buffer,int size)
{   
	int i;
	uint8_t addr_w = (0x00 | (addr<<1));
	
	rf_port.spi_cs_low();	
	rf_port.spi_readwrite(addr_w);
	for(i =0;i<size;i++)
	{
		buffer[i] = rf_port.spi_readwrite(0x00);	
	}
	rf_port.spi_cs_high();	
}

/**
 * @brief switch page
 * @param[in] <page> page to switch
 * @return result
 */
static uint32_t PAN3031_switch_page(enum PAGE_SEL page)
{	
	uint8_t page_sel = 0x00;
	uint8_t tmpreg = 0x00;
	
	tmpreg = PAN3031_read_reg(REG_SYS_CTL);
	page_sel  = (tmpreg & 0xfc )| page;
	PAN3031_write_reg(REG_SYS_CTL,page_sel);
	if((PAN3031_read_reg(REG_SYS_CTL) &0x03) == page)
	{
		return OK;
	}else
	{
		return FAIL;
	}
}

/**
 * @brief This function write a value to register in specific page
 * @param[in] <page> the page of register
 * @param[in] <addr> register address
 * @param[in] <value> value to write
 * @return result
 */
uint32_t PAN3031_write_spec_page_reg(enum PAGE_SEL page,uint8_t addr,uint8_t value)
{ 
	if(PAN3031_switch_page(page) != OK)
	{
		return FAIL;
	}

	if(PAN3031_write_reg(addr,value) != OK)
	{
		return FAIL;
	}
	else
	{
		return OK;
	}
} 

/**
 * @brief read a value to register in specific page
 * @param[in] <page> the page of register
 * @param[in] <addr> register address
 * @return success(register value) or failure
 */
uint8_t PAN3031_read_spec_page_reg(enum PAGE_SEL page,uint8_t addr)
{ 	 
	if(PAN3031_switch_page(page) != OK)
	{
		return FAIL;
	}
	return PAN3031_read_reg(addr);
} 

/**
 * @brief PAN3031 clear all irq
 * @param[in] <none> 
 * @return none
 */
void PAN3031_clr_irq(void)
{
	PAN3031_write_spec_page_reg(PAGE0_SEL,0x6C,0x1f);
}

/**
 * @brief get irq status
 * @param[in] <none> 
 * @return ira status
 */
uint8_t PAN3031_get_irq(void)
{
	uint8_t tmpreg;
	
	tmpreg = PAN3031_read_spec_page_reg(PAGE0_SEL,0x6C);
	
	return tmpreg;
}

/**
 * @brief software reset
 * @param[in] <none> 
 * @return result
 */
uint32_t PAN3031_rst(void)
{
	uint8_t tmpreg = 0;
	
	tmpreg = PAN3031_read_reg(REG_SYS_CTL);
	tmpreg |= 0x80;
	PAN3031_write_reg(REG_SYS_CTL,tmpreg);
	
	tmpreg = PAN3031_read_reg(REG_SYS_CTL);
	tmpreg &= 0x7F;

	PAN3031_write_reg(REG_SYS_CTL,tmpreg);
	
	return OK;
}

/**
 * @brief clear packet count register
 * @param[in] <none> 
 * @return none
 */
void PAN3031_clr_pkt_cnt(void)
{
	uint8_t tmpreg;
	
	tmpreg = PAN3031_read_reg(REG_SYS_CTL);
	tmpreg = (tmpreg & 0xbf) | 0x40 ;
	PAN3031_write_reg(REG_SYS_CTL,tmpreg);
	
	tmpreg = PAN3031_read_reg(REG_SYS_CTL);
	tmpreg = (tmpreg & 0xbf);
	PAN3031_write_reg(REG_SYS_CTL,tmpreg);
}

/**
 * @brief enable AGC function
 * @param[in] <state>  
 *			  AGC_OFF/AGC_ON
 * @return result
 */
uint32_t PAN3031_agc_enable(uint32_t state)
{
	uint8_t reg_val = 0x02;
    
	if(state == AGC_OFF)
	{
		reg_val = 0x03;
	}
	else
	{
		reg_val = 0x02;
	}
    
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x06, reg_val)  != OK)
	{
		return FAIL;
	}
    
	return OK;
}

/**
 * @brief configure AGC function
 * @param[in] <none>  
 * @return result
 */
uint32_t PAN3031_agc_config(void)
{
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x07,0x90) != OK)
	{		
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x08,0xff) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x09,0x64) != OK)
	{		
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x0A,0x27) != OK)
	{		
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x0B,0x00) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x0C,0x00) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x0D,0x27) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x0E,0x27) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x0F,0x00) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x10,0x00) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x11,0x27) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x12,0x27) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x13,0x00) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x14,0x00) != OK)
	{
		return FAIL;
	}

	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x15,0x27) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x16,0x27) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x17,0x00) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x18,0x00) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x19,0x27) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x1A,0x27) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x1B,0x00) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x1C,0x00) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x1D,0x27) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x1E,0x27) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x1F,0x00) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x20,0x00) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x21,0x27) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x22,0x27) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x23,0x00) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x24,0x00) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x25,0x27) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x26,0x27) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x27,0x00) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x28,0x00) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x29,0x27) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x2A,0x27) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x2B,0x00) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x2C,0x00) != OK)
	{
		return FAIL;
	}

	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x2D,0x27) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x2E,0x2B) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x2F,0x00) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x30,0xF8) != OK)
	{
		return FAIL;
	}

	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x31,0x2B) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x32,0x31) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x33,0x00) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x34,0xFC) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x35,0x31) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x36,0x37) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x37,0x00) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x38,0xFF) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x39,0x37) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x3A,0x3C) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x3B,0x20) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x3C,0xFF) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x3D,0x3C) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x3E,0x42) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x3F,0x40) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x40,0xFF) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x41,0x42) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x42,0x48) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x43,0x60) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x44,0xFF) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x45,0x48) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x46,0x4D) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x47,0x80) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x48,0xFF) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x49,0x4D) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x4A,0x53) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x4B,0x84) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x4C,0xFF) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x4D,0x53) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x4E,0x59) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x4F,0x88) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x50,0xFF) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x51,0x59) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x52,0x5F) != OK)
	{
		return FAIL;
	}

	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x53,0x8C) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x54,0xFF) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x55,0x5F) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x56,0x64) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x57,0x90) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x58,0xFF) != OK)
	{
		return FAIL;
	}

	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x59,0x64) != OK)
	{
		return FAIL;
	}

	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x5A,0x06) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x5B,0xFF) != OK)
	{
		return FAIL;
	}

	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x5C,0x40) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x5D,0x42) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x5E,0x0F) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x5F,0x00) != OK)
	{
		return FAIL;
	}

	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x60,0x00) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x61,0x01) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x62,0xF4) != OK)
	{
		return FAIL;
	}

	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x63,0x2F) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x64,0xF3) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x65,0x0F) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x66,0x00) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x67,0x00) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE2_SEL,0x68,0x00) != OK)
	{
		return FAIL;
	}
    return OK;
}

/**
 * @brief do basic configuration to initialize
 * @param[in] <none>
 * @return result
 */
uint32_t PAN3031_init(void)
{ 
	if(PAN3031_write_spec_page_reg(PAGE0_SEL,0x06,0x01)  != OK)
	{
		return FAIL;
	}
   
	if(PAN3031_write_spec_page_reg(PAGE0_SEL,0x40,0x50)  != OK)
	{
		return FAIL;
	}

	if(PAN3031_write_spec_page_reg(PAGE0_SEL,0x3e,0x2c)  != OK)
	{
		return FAIL;
	}
    
	if(PAN3031_write_spec_page_reg(PAGE0_SEL,0x3c,0xff)  != OK)
	{
		return FAIL;
	}	
	
	if(PAN3031_write_spec_page_reg(PAGE1_SEL,0x0e,0x44)  != OK)
	{
		return FAIL;
	}    

	if(PAN3031_write_spec_page_reg(PAGE1_SEL,0x0f,0x0A)  != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE1_SEL,0x1e,0x00)  != OK)
	{
		return FAIL;
	}

	if(PAN3031_write_spec_page_reg(PAGE1_SEL,0x11,0xA1)  != OK)
	{
		return FAIL;
	}  
	
	if(PAN3031_write_spec_page_reg(PAGE1_SEL,0x15,0x38)  != OK)
	{
		return FAIL;
	} 
	
	if(PAN3031_write_spec_page_reg(PAGE1_SEL,0x2f,0x0c)  != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE3_SEL,0x06,0x26)  != OK)
	{
		return FAIL;
	}

	if(PAN3031_write_spec_page_reg(PAGE3_SEL,0x10,0x80) != OK)
	{
		return FAIL;
	}

	if(PAN3031_write_spec_page_reg(PAGE3_SEL,0x11,0x0d)  != OK)
	{
		return FAIL;
	}

	if(PAN3031_write_spec_page_reg(PAGE3_SEL,0x12,0x16)  != OK) 
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE3_SEL,0x18,0xff) != OK)
	{
		return FAIL;
	}

	return OK;
}

/**
 * @brief change PAN3031 mode from deep sleep to standby3(STB3) 
 * @param[in] <none>
 * @return result
 */
uint32_t PAN3031_deepsleep_wakeup(void)
{
	if(PAN3031_write_reg(REG_OP_MODE,PAN3031_MODE_DEEP_SLEEP) != OK)	
	{
		return FAIL;
	}
	rf_port.delayus(10);

	if(PAN3031_write_reg(REG_OP_MODE,PAN3031_MODE_SLEEP) != OK)	
	{
		return FAIL;
	}
	rf_port.delayus(10);

	if(PAN3031_write_reg(0x03, 0x1b) != OK)	
	{
		return FAIL;
	}	
	
	if(PAN3031_write_reg(0x04, 0x76) != OK)	
	{
		return FAIL;
	}

	if(PAN3031_write_spec_page_reg(PAGE3_SEL,0x26, 0x40)  != OK)
	{
		return FAIL;
	}	

	rf_port.tcxo_init();
	rf_port.delayms(1);

	if(PAN3031_write_reg(REG_OP_MODE,PAN3031_MODE_STB1) != OK)		
	{
		return FAIL;
	}
	rf_port.delayus(10);

	if(PAN3031_write_reg(REG_OP_MODE,PAN3031_MODE_STB2) != OK)
	{
		return FAIL;
	}
	rf_port.delayms(2);

	if(PAN3031_write_reg(REG_OP_MODE,PAN3031_MODE_STB3) != OK)
	{
		return FAIL;
	}	
	else
	{
		rf_port.delayus(10);
		return OK;
	}
}

/**
 * @brief change PAN3031 mode from sleep to standby3(STB3) 
 * @param[in] <none>
 * @return result
 */
uint32_t PAN3031_sleep_wakeup(void)
{
	if(PAN3031_write_reg(REG_OP_MODE,PAN3031_MODE_SLEEP) != OK)	
	{
		return FAIL;
	}
	rf_port.delayus(10);

	if(PAN3031_write_reg(0x03, 0x1b) != OK)	
	{
		return FAIL;
	}	
	
	if(PAN3031_write_reg(0x04, 0x76) != OK)	
	{
		return FAIL;
	}

	if(PAN3031_write_spec_page_reg(PAGE3_SEL,0x26, 0x40)  != OK)
	{
		return FAIL;
	}	

	rf_port.tcxo_init();
	rf_port.delayms(1);	

	if(PAN3031_write_reg(REG_OP_MODE,PAN3031_MODE_STB1) != OK)		
	{
		return FAIL;
	}
	rf_port.delayus(10);

	if(PAN3031_write_reg(REG_OP_MODE,PAN3031_MODE_STB2) != OK)
	{
		return FAIL;
	}
	rf_port.delayms(2);

	if(PAN3031_write_reg(REG_OP_MODE,PAN3031_MODE_STB3) != OK)
	{
		return FAIL;
	}	
	else
	{
		rf_port.delayus(10);
		return OK;
	}
}

/**
 * @brief change PAN3031 mode from standby3(STB3) to deep sleep, PAN3031 should set DCDC_OFF before enter deepsleep
 * @param[in] <none>
 * @return result
 */
uint32_t PAN3031_deepsleep(void)
{
	if(PAN3031_write_reg(REG_OP_MODE,PAN3031_MODE_STB3) != OK)	
	{
		return FAIL;
	}
	rf_port.delayus(10);
	
	if(PAN3031_write_reg(REG_OP_MODE,PAN3031_MODE_STB2) != OK)		
	{
		return FAIL;
	}
	rf_port.delayus(10);
	
	if(PAN3031_write_reg(REG_OP_MODE,PAN3031_MODE_STB1) != OK)
	{
		return FAIL;
	}
	rf_port.delayus(10);
	
	rf_port.tcxo_close();
	
	if(PAN3031_write_reg(0x04, 0x06) != OK)	
	{
		return FAIL;
	}	
	rf_port.delayus(10);

	if(PAN3031_write_reg(REG_OP_MODE,PAN3031_MODE_SLEEP) != OK)	
	{
		return FAIL;
	}	
 	rf_port.delayus(10);

	if(PAN3031_write_reg(REG_OP_MODE,PAN3031_MODE_DEEP_SLEEP) != OK)	
	{
		return FAIL;
	}
	else
	{
		return OK;
	}
}


/**
 * @brief change PAN3031 mode from standby3(STB3) to sleep, PAN3031 should set DCDC_OFF before enter sleep
 * @param[in] <none>
 * @return result
 */
uint32_t PAN3031_sleep(void)
{
	if(PAN3031_write_reg(REG_OP_MODE,PAN3031_MODE_STB3) != OK)	
	{
		return FAIL;
	}
	rf_port.delayus(10);

	if(PAN3031_write_reg(REG_OP_MODE,PAN3031_MODE_STB2) != OK)		
	{
		return FAIL;
	}
	rf_port.delayus(10);

	if(PAN3031_write_reg(REG_OP_MODE,PAN3031_MODE_STB1) != OK)
	{
		return FAIL;
	}
	rf_port.delayus(10);

	rf_port.tcxo_close();
	
	if(PAN3031_write_reg(0x04, 0x16) != OK)	
	{
		return FAIL;
	}	
	rf_port.delayus(10);

	if(PAN3031_write_reg(REG_OP_MODE,PAN3031_MODE_SLEEP) != OK)	
	{
		return FAIL;
	}	
	else
	{
		rf_port.delayus(10);
		return OK;
	}
}

/**
 * @brief set LO frequency 
 * @param[in] <lo> LO frequency 
 *			  LO_400M / LO_800M
 * @return result
 */
uint32_t PAN3031_set_lo_freq(uint32_t lo)
{
	uint32_t reg_val = 0;
	reg_val = PAN3031_read_spec_page_reg(PAGE0_SEL,0x45);
	reg_val &= ~(0x03);

	if(lo == LO_400M)
	{
		reg_val |= 0x02;
	}
	else
	{
		reg_val |= 0x01;
	}

    if(PAN3031_write_spec_page_reg(PAGE0_SEL,0x45, reg_val)  != OK)
	{
		return FAIL;
	}
	return OK;
}

/**
 * @brief set frequence
 * @param[in] <freq>  RF frequency(in Hz) to set
 * @return result
 */
uint32_t PAN3031_set_freq(uint32_t freq)
{
	uint8_t reg_read;
	uint8_t reg_freq;
	float tmp_var = 0.0;
	int integer_part = 0;
	float fractional_part = 0.0;
	int fb,fc;
	uint8_t lowband_sel = 0;

	if ( (freq >= freq_336000000) && (freq <= freq_470000000))
	{
		if(PAN3031_write_spec_page_reg(PAGE0_SEL,0x4a,0x8e)  != OK)
		{
			return FAIL;
		}
		lowband_sel = 1;
		tmp_var = freq * 4 * 1.0 / 16000000;
		PAN3031_set_lo_freq(LO_400M);
	}
	else if ( (freq > freq_470000000) && (freq <= freq_510000000))
	{
		if(PAN3031_write_spec_page_reg(PAGE0_SEL,0x4a,0xae)  != OK)
		{
			return FAIL;
		}
		lowband_sel = 1;
		tmp_var = freq * 4 * 1.0 / 16000000;
		PAN3031_set_lo_freq(LO_400M);
	}
	else if((freq >= freq_800000000) && (freq <= freq_920000000))
	{
		if(PAN3031_write_spec_page_reg(PAGE0_SEL,0x4a,0x8e)  != OK)
		{
			return FAIL;
		}	
        lowband_sel = 0;
		tmp_var = freq * 2 * 1.0 / 16000000;
		PAN3031_set_lo_freq(LO_800M);
	}	
	else
	{
		return FAIL;
	}
	integer_part = (int)tmp_var;
	fb = integer_part - 20;
	fractional_part = tmp_var - integer_part;
	fc = (int)(fractional_part * 1600 / (2 * (1 + lowband_sel)));
	
	if(fc < 0xff)
	{
		fb = fb - 1;
		fc = fc + 400;
	}

	if(PAN3031_write_spec_page_reg(PAGE3_SEL, 0x15, (fb & 0x7F)) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE3_SEL, 0x16,(fc & 0xff)) != OK)
	{
		return FAIL;
	}
	
	if(PAN3031_write_spec_page_reg(PAGE3_SEL, 0x17,((fc >> 8) & 0x0f)) != OK)
	{
		return FAIL;
	}

	reg_read = PAN3031_read_spec_page_reg(PAGE3_SEL, 0x18);
	reg_read &= ~((1 << 2) | (1 << 1));
	reg_read |= (1 << 3) | (lowband_sel << 2) | (lowband_sel << 1);
    
	if(PAN3031_write_spec_page_reg(PAGE3_SEL, 0x18, reg_read) != OK)
	{
		return FAIL;
	}
    
	reg_freq = freq & 0xff;
	if(PAN3031_write_spec_page_reg(PAGE3_SEL, 0x09, reg_freq) != OK)
	{
		return FAIL;
	} 
    
	reg_freq = (freq >> 8) & 0xff;
	if(PAN3031_write_spec_page_reg(PAGE3_SEL, 0x0a, reg_freq) != OK)
	{
		return FAIL;
	}  

	reg_freq = (freq >> 16) & 0xff;
	if(PAN3031_write_spec_page_reg(PAGE3_SEL, 0x0b, reg_freq) != OK)
	{
		return FAIL;
	}      

	reg_freq = (freq >> 24) & 0xff;
	if(PAN3031_write_spec_page_reg(PAGE3_SEL, 0x0c, reg_freq) != OK)
	{
		return FAIL;
	}  
    
	return OK;
}

/**
 * @brief read frequency(in Hz)
 * @param[in] <none>   
 * @return frequency(in Hz)
 */
uint32_t PAN3031_read_freq(void)
{
	uint8_t reg1, reg2, reg3 , reg4;
	uint32_t freq = 0x00;
	
	reg1 = PAN3031_read_spec_page_reg(PAGE3_SEL, 0x09);
	reg2 = PAN3031_read_spec_page_reg(PAGE3_SEL, 0x0a);
	reg3 = PAN3031_read_spec_page_reg(PAGE3_SEL, 0x0b);
	reg4 = PAN3031_read_spec_page_reg(PAGE3_SEL, 0x0c);
	freq = (reg4 << 24) | (reg3 << 16) | (reg2 << 8) | reg1;
	return freq;
}

/**
 * @brief calculate tx time
 * @param[in] <none>
 * @return tx time(ms) 
 */
uint32_t PAN3031_calculate_tx_time(void)
{
	int bw_val;
	float tx_done_time;	
	uint8_t pl = PAN3031_read_spec_page_reg(PAGE1_SEL,REG_PAYLOAD_LEN);
	uint8_t sf = PAN3031_get_sf();
	uint8_t crc = PAN3031_get_crc();
	uint8_t code_rate = PAN3031_get_code_rate();
	uint8_t bw = PAN3031_get_bw();

	float a,b,c,d=0.00;
	
	if(bw == 7)	
	{
		bw_val = 125000;
	}
	if(bw == 8)	
	{
		bw_val = 250000;
	}
	if(bw == 9)	
	{
		bw_val = 500000;
	}
	a = (float)(8 * pl - 4 * sf + 28 + 16 *crc) / (float)(4 * sf);
	b = ceil(a);
	c = code_rate + 4;
	d = ((float)((2<<(sf-1))) / bw_val);
	tx_done_time =(12.25 +8 + b*c)*d*1000 ;

	return tx_done_time + 5; 
}

/**
 * @brief set bandwidth
 * @param[in] <bw_val> value relate to bandwidth
 *		       BW_125K / BW_250K / BW_500K
 * @return result
 */
uint32_t PAN3031_set_bw(uint32_t bw_val)
{
	uint8_t temp_val_1;
	uint8_t temp_val_2;
	temp_val_1 = PAN3031_read_spec_page_reg(PAGE3_SEL, 0x0d);
	temp_val_2 = ((temp_val_1 & 0x0F) | (bw_val << 4)) ;
	if(PAN3031_write_spec_page_reg(PAGE3_SEL, 0x0d, temp_val_2) != OK)
	{
		return FAIL;
	}
	else
	{
		return OK;
	}
}

/**
 * @brief read bandwidth
 * @param[in] <none>   
 * @return bandwidth
 */
uint8_t PAN3031_get_bw(void)
{
	uint8_t tmpreg;
	
	tmpreg = PAN3031_read_spec_page_reg(PAGE3_SEL, 0x0d);
	
	return (tmpreg & 0xff) >> 4;
}

/**
 * @brief set spread factor
 * @param[in] <sf> spread factor to set
 *		      SF_7 / SF_8 / SF_9 
 * @return result
 */
uint32_t PAN3031_set_sf(uint32_t sf_val)
{
	uint8_t temp_val_1;
	uint8_t temp_val_2;

	if(sf_val < 7 || sf_val > 12)
	{
        return FAIL;
	}
	else
	{
		temp_val_1 = PAN3031_read_spec_page_reg(PAGE3_SEL, 0x0e);
		temp_val_2 = ((temp_val_1 & 0x0F) | (sf_val << 4)) ;
		if(PAN3031_write_spec_page_reg(PAGE3_SEL, 0x0e, temp_val_2) != OK)
		{
			return FAIL;
		}
		else
		{
			return OK;
		}
	}
}

/**
 * @brief read Spreading Factor
 * @param[in] <none>   
 * @return Spreading Factor
 */
uint8_t PAN3031_get_sf(void)
{
	uint8_t tmpreg;
	
	tmpreg = PAN3031_read_spec_page_reg(PAGE3_SEL, 0x0e);

	return (tmpreg & 0xff) >> 4;
}

/**
 * @brief set payload CRC
 * @param[in] <crc_val> CRC to set
 *		      CRC_ON / CRC_OFF
 * @return result
 */
uint32_t PAN3031_set_crc(uint32_t crc_val)
{
	uint8_t temp_val_1;
	uint8_t temp_val_2;
	
	temp_val_1 = PAN3031_read_spec_page_reg(PAGE3_SEL, 0x0e);
	temp_val_2 = ((temp_val_1 & 0xF7) | (crc_val << 3)) ;
	if(PAN3031_write_spec_page_reg(PAGE3_SEL, 0x0e, temp_val_2) != OK)
	{
		return FAIL;
	}
	else
	{
		return OK;
	}
}

/**
 * @brief read payload CRC
 * @param[in] <none>   
 * @return CRC status
 */
uint8_t PAN3031_get_crc(void)
{
	uint8_t tmpreg;
	
	tmpreg = PAN3031_read_spec_page_reg(PAGE3_SEL, 0x0e);

	return (tmpreg & 0x08) >> 3;
}

/**
 * @brief set code rate
 * @param[in] <code_rate> code rate to set 
 *			  CODE_RATE_45 / CODE_RATE_46 / CODE_RATE_47 / CODE_RATE_48
 * @return result
 */
uint32_t PAN3031_set_code_rate(uint8_t code_rate)
{
	uint8_t tmpreg = 0;
	
	tmpreg = PAN3031_read_spec_page_reg(PAGE3_SEL, 0x0d);
	tmpreg &= ~(0x7 << 1);
	tmpreg |= (code_rate << 1);
	if(PAN3031_write_spec_page_reg(PAGE3_SEL, 0x0d, tmpreg) != OK)
	{
		return FAIL;
	}
	else
	{
		return OK;
	}
}

/**
 * @brief get code rate
 * @param[in] <none>
 * @return code rate
 */
uint8_t PAN3031_get_code_rate(void)
{
	uint8_t code_rate = 0;
	uint8_t tmpreg = 0;
	
	tmpreg = PAN3031_read_spec_page_reg(PAGE3_SEL, 0x0d);
	code_rate = ((tmpreg & 0x0e) >> 1);

	return code_rate;
}

/**
 * @brief set rf mode
 * @param[in] <mode>    
 *			  PAN3031_MODE_DEEP_SLEEP / PAN3031_MODE_SLEEP
 *            PAN3031_MODE_STB1 / PAN3031_MODE_STB2
 *            PAN3031_MODE_STB3 / PAN3031_MODE_TX / PAN3031_MODE_RX
 * @return result
 */
uint32_t PAN3031_set_mode(uint8_t mode)
{
	if(PAN3031_write_reg(REG_OP_MODE,mode) != OK)
	{
		return FAIL;
	}
	else
	{
		return OK;
	}
}

/**
 * @brief get rf mode
 * @param[in] <none>
 * @return mode 
 *		   PAN3031_MODE_DEEP_SLEEP / PAN3031_MODE_SLEEP
 *         PAN3031_MODE_STB1 / PAN3031_MODE_STB2
 *         PAN3031_MODE_STB3 / PAN3031_MODE_TX / PAN3031_MODE_RX
 */
uint8_t PAN3031_get_mode(void)
{
	return PAN3031_read_reg(REG_OP_MODE);
}

/**
 * @brief set rf Tx mode
 * @param[in] <mode> 
 *			  PAN3031_TX_SINGLE/PAN3031_TX_CONTINOUS
 * @return result
 */
uint32_t PAN3031_set_tx_mode(uint8_t mode)
{
	uint8_t tmp;
	tmp = PAN3031_read_spec_page_reg(PAGE3_SEL, 0x06);
	tmp = tmp & (~(1 << 2));
	tmp = tmp | (mode << 2);

	if(PAN3031_write_spec_page_reg(PAGE3_SEL,0x06,tmp) != OK)
	{
		return FAIL;
	}
	else
	{
		return OK;
	}
}

/**
 * @brief set rf Rx mode
 * @param[in] <mode> 
 *			  PAN3031_RX_SINGLE/PAN3031_RX_SINGLE_TIMEOUT/PAN3031_RX_CONTINOUS
 * @return result
 */
uint32_t PAN3031_set_rx_mode(uint8_t mode)
{
	uint8_t tmp;
	tmp = PAN3031_read_spec_page_reg(PAGE3_SEL, 0x06);
	tmp = tmp & (~(3 << 0));
	tmp = tmp | (mode << 0);
	
	if(PAN3031_write_spec_page_reg(PAGE3_SEL, 0x06, tmp) != OK)
	{
		return FAIL;
	}
	else
	{
		return OK;
	}
}

/**
 * @brief set timeout for Rx. It is useful in PAN3031_RX_SINGLE_TIMEOUT mode
 * @param[in] <timeout> rx single timeout time(in ms)
 * @return result
 */
uint32_t PAN3031_set_timeout(uint32_t timeout)
{
	uint8_t timeout_lsb = 0;
	uint8_t timeout_msb = 0;
    
	if(timeout > 0xffff)
	{
		timeout = 0xffff;
	}
    
	timeout_lsb = timeout & 0xff;
	timeout_msb = (timeout >> 8) & 0xff; 
	
	if(PAN3031_write_spec_page_reg(PAGE3_SEL, 0x07, timeout_lsb) != OK)
	{
		return FAIL;
	}
    
	if(PAN3031_write_spec_page_reg(PAGE3_SEL, 0x08, timeout_msb) != OK)
	{
		return FAIL;
	}    
	else
	{
		return OK;
	}
}

/**
 * @brief get snr value
 * @param[in] <none> 
 * @return snr
 */
float PAN3031_get_snr(void)
{
	float snr_val=0.0;
	uint8_t sig_pow_l, sig_pow_m, sig_pow_h;
	uint8_t noise_pow_l, noise_pow_m, noise_pow_h;
	uint32_t sig_pow_val;
	uint32_t noise_pow_val;
	uint32_t sf_val;
	
	sig_pow_l = PAN3031_read_spec_page_reg(PAGE1_SEL,0x74);
	sig_pow_m = PAN3031_read_spec_page_reg(PAGE1_SEL,0x75);
	sig_pow_h = PAN3031_read_spec_page_reg(PAGE1_SEL,0x76);
	sig_pow_val = ((sig_pow_h << 16) | (sig_pow_m << 8) | sig_pow_l );

	noise_pow_l = PAN3031_read_spec_page_reg(PAGE2_SEL,0x71);
	noise_pow_m = PAN3031_read_spec_page_reg(PAGE2_SEL,0x72);
	noise_pow_h = PAN3031_read_spec_page_reg(PAGE2_SEL,0x73);
	noise_pow_val = ((noise_pow_h << 16) | (noise_pow_m << 8) | noise_pow_l );

	sf_val = (PAN3031_read_spec_page_reg(PAGE1_SEL,0x7c) & 0xf0) >> 4;

	if(noise_pow_val == 0)
	{
		noise_pow_val = 1;
	}
	snr_val = (float)(10 * log10((sig_pow_val / pow(2,sf_val)) / noise_pow_val));

	return snr_val;
}

/**
 * @brief get cascade snr value
 * @param[in] <none> 
 * @return snr
 */
float PAN3031_get_snr_cascade(void)
{
	float snr_val1=0.0;
	uint8_t sig_pow_l, sig_pow_m, sig_pow_h;
	uint8_t noise_pow_l, noise_pow_m, noise_pow_h;
	uint32_t sig_pow_val;
	uint32_t  noise_pow_val;
    
	sig_pow_l = PAN3031_read_spec_page_reg(PAGE1_SEL, 0x74);
	sig_pow_m = PAN3031_read_spec_page_reg(PAGE1_SEL, 0x75);
	sig_pow_h = PAN3031_read_spec_page_reg(PAGE1_SEL, 0x76);
	sig_pow_val = ( (sig_pow_h << 16) | (sig_pow_m << 8) | sig_pow_l );

	noise_pow_l = PAN3031_read_spec_page_reg(PAGE2_SEL,0x71);
	noise_pow_m = PAN3031_read_spec_page_reg(PAGE2_SEL,0x72);
	noise_pow_h = PAN3031_read_spec_page_reg(PAGE2_SEL,0x73);
	noise_pow_val = ((noise_pow_h << 16) | (noise_pow_m << 8) | noise_pow_l );
	
	if(noise_pow_val == 0)
	{
		noise_pow_val = 1;
	}
	snr_val1 = (float)(10 * log10((sig_pow_val)/noise_pow_val));

	return snr_val1;
}

/**
 * @brief get rssi value
 * @param[in] <none> 
 * @return rssi
 */
float PAN3031_get_rssi(void)
{
	float rssi_val;
	int rssi_mix_val;
	int bw_pow_val;

	float snr;
	
	int bw_val = PAN3031_get_bw();

	switch(bw_val)
	{
		case 6 : 
			bw_pow_val = 9;
			break;
		case 7 : 	
			bw_pow_val = 6;
			break;
		case 8:		
			bw_pow_val = 3;
			break;
		case 9:		
			bw_pow_val = 0;
			break;
	}

	snr = PAN3031_get_snr();
	PAN3031_get_snr_cascade();

	if(snr < 6)
	{
		rssi_val = snr - 113 - bw_pow_val;

	}else{
		rssi_mix_val = PAN3031_read_spec_page_reg(PAGE1_SEL,0x7e);
		rssi_val = rssi_mix_val - 256;
	}

	return rssi_val;
}

/**
 * @brief set tx_power
 * @param[in] <tx_power> Reference datasheet for tx_power parameter description
 * @return result
 */
uint32_t PAN3031_set_tx_power(uint8_t tx_power)
{
	uint8_t pa_1st_pwr, pa_2nd_pwr, reg_val;

	pa_1st_pwr = (tx_power >> 4) & 0x07;
	pa_2nd_pwr = tx_power & 0x0f;
	if(pa_1st_pwr < 0x7)
	{
		pa_2nd_pwr = 0x0;
	}

	reg_val = (pa_2nd_pwr << 4) | pa_1st_pwr;
	return PAN3031_write_spec_page_reg(PAGE1_SEL, 0x63, reg_val);
}

/**
 * @brief get tx_power
 * @param[in] <none> 
 * @return tx_power
 */
uint32_t PAN3031_get_tx_power(void)
{
	uint8_t pa_1st_pwr, pa_2nd_pwr, reg_val;

	reg_val = PAN3031_read_spec_page_reg(PAGE1_SEL, 0x63);
    
	pa_1st_pwr = reg_val & 0x07;
	pa_2nd_pwr = (reg_val >> 4) & 0x0f;
    
	return ((pa_1st_pwr << 4) | pa_2nd_pwr);
}

/**
 * @brief set preamble 
 * @param[in] <reg> preamble
 * @return result
 */
uint32_t PAN3031_set_preamble(uint16_t reg)
{
	uint8_t tmp_value;
    
	tmp_value = reg & 0xff;
    
	if(PAN3031_write_spec_page_reg(PAGE3_SEL,0x13, tmp_value)  != OK)
	{
		return FAIL;
	}
    
	tmp_value = (reg >> 8) & 0xff;
	if(PAN3031_write_spec_page_reg(PAGE3_SEL,0x14, tmp_value)  != OK)
	{
		return FAIL;
	}
	return OK;
}

/**
 * @brief set RF GPIO as input
 * @param[in] <gpio_pin>  pin number of GPIO to be enable
 * @return result
 */
uint32_t PAN3031_set_gpio_input(uint8_t gpio_pin)
{
	uint8_t tmpreg = 0;
	
	if(gpio_pin < 8)
	{
		tmpreg = PAN3031_read_spec_page_reg(PAGE0_SEL, 0x63);
		tmpreg |= (1 << gpio_pin);
		if(PAN3031_write_spec_page_reg(PAGE0_SEL, 0x63, tmpreg) != OK)
		{
			return FAIL;
		}
		else
		{
			return OK;
		}        
	}
	else
	{
		tmpreg = PAN3031_read_spec_page_reg(PAGE0_SEL, 0x64);
		tmpreg |= (1 << (gpio_pin - 8));
		if(PAN3031_write_spec_page_reg(PAGE0_SEL, 0x64, tmpreg) != OK)
		{
			return FAIL;
		}
		else
		{
			return OK;
		}
	}
}

/**
 * @brief set RF GPIO as output
 * @param[in] <gpio_pin>  pin number of GPIO to be enable
 * @return result
 */
uint32_t PAN3031_set_gpio_output(uint8_t gpio_pin)
{
	uint8_t tmpreg = 0;
	
    if(gpio_pin < 8)
    {
        tmpreg = PAN3031_read_spec_page_reg(PAGE0_SEL, 0x65);
        tmpreg |= (1 << gpio_pin);
        if(PAN3031_write_spec_page_reg(PAGE0_SEL, 0x65, tmpreg) != OK)
        {
            return FAIL;
        }
        else
        {
            return OK;
        }        
    }
    else
    {
        tmpreg = PAN3031_read_spec_page_reg(PAGE0_SEL, 0x66);
        tmpreg |= (1 << (gpio_pin - 8));
        if(PAN3031_write_spec_page_reg(PAGE0_SEL, 0x66, tmpreg) != OK)
        {
            return FAIL;
        }
        else
        {
            return OK;
        }
    }
}

/**
 * @brief set GPIO output state, SET or RESET 
 * @param[in] <gpio_pin>  pin number of GPIO to be opearted
 *            <state>   0  -  reset, 
 *                      1  -  set
 * @return result
 */
uint32_t PAN3031_set_gpio_state(uint8_t gpio_pin, uint8_t state)
{
	uint8_t tmpreg = 0;
	
    if(gpio_pin < 8)
    {
        tmpreg = PAN3031_read_spec_page_reg(PAGE0_SEL, 0x67);
    
        if(state == 0)
        {
            tmpreg &= ~(1 << gpio_pin);
        }
        else
        {
            tmpreg |= (1 << gpio_pin);
        }
   
        if(PAN3031_write_spec_page_reg(PAGE0_SEL, 0x67, tmpreg) != OK)
        {
            return FAIL;
        }
        else
        {
            return OK;
        }        
    }
    else
    {
        tmpreg = PAN3031_read_spec_page_reg(PAGE0_SEL, 0x68);
    	
        if(state == 0)
        {
            tmpreg &= ~(1 << (gpio_pin - 8));
        }
        else
        {
            tmpreg |= (1 << (gpio_pin - 8));
        }


        if(PAN3031_write_spec_page_reg(PAGE0_SEL, 0x68, tmpreg) != OK)
        {
            return FAIL;
        }
        else
        {
            return OK;
        }
    }
}

/**
 * @brief CAD function enable
 * @param[in] <none> 
 * @return  result
 */
uint32_t PAN3031_cad_en(void)
{   
	PAN3031_set_gpio_output(11);

	if(PAN3031_write_spec_page_reg(PAGE1_SEL, 0x0f, 0x15) != OK)
	{
		return FAIL;
	}
	return OK;
}

/**
 * @brief set rf syncword
 * @param[in] <sync> syncword
 * @return result
 */
uint32_t PAN3031_set_syncword(uint32_t sync)
{
	if(PAN3031_write_spec_page_reg(PAGE3_SEL, 0x0f, sync) != OK)
	{
		return FAIL;
	}
	else
	{
		return OK;
	}
}

/**
 * @brief read rf syncword
 * @param[in] <none>    
 * @return syncword
 */
uint8_t PAN3031_get_syncword(void)
{
	uint8_t tmpreg;
	
	tmpreg = PAN3031_read_spec_page_reg(PAGE3_SEL, 0x0f);

	return tmpreg;
}

/**
 * @brief send one packet
 * @param[in] <buff> buffer contain data to send
 * @param[in] <len> the length of data to send
 * @return result
 */
uint32_t PAN3031_send_packet(uint8_t *buff, uint32_t len)
{
	if(PAN3031_write_spec_page_reg(PAGE1_SEL,REG_PAYLOAD_LEN,len) != OK)
	{
		return FAIL;
	}
	if(PAN3031_write_reg(REG_OP_MODE,PAN3031_MODE_TX) != OK)
	{
		return FAIL;
	}
	else
	{
		PAN3031_write_fifo(REG_FIFO_ACC_ADDR,buff,len);	
		return OK;
	}
}

/**
 * @brief receive a packet in non-block method, it will return 0 when no data got
 * @param[in] <buff> buffer provide for data to receive
 * @return length, it will return 0 when no data got
 */
uint8_t PAN3031_recv_packet(uint8_t *buff)
{
	uint32_t len = 0;

	len = PAN3031_read_spec_page_reg(PAGE1_SEL, 0x7D);
	PAN3031_read_fifo(REG_FIFO_ACC_ADDR,buff,len);
	
	/* clear rx done irq */
	PAN3031_clr_irq();		
    
	return len;
}

/**
 * @brief set early interruption
 * @param[in] <earlyirq_val> PLHD IRQ to set
 *		      PLHD_IRQ_ON / PLHD_IRQ_OFF
 * @return result
 */
uint32_t PAN3031_set_early_irq(uint32_t earlyirq_val)
{
	uint8_t temp_val_1;
	uint8_t temp_val_2;
	
	temp_val_1 = PAN3031_read_spec_page_reg(PAGE1_SEL, 0x2d);
	temp_val_2 = ((temp_val_1 & 0x7f) | (earlyirq_val << 7)) ;

	if(PAN3031_write_spec_page_reg(PAGE1_SEL, 0x2d, temp_val_2) != OK)
	{
		return FAIL;
	}
	else
	{
		return OK;
	}
}

/**
 * @brief read plhd irq status
 * @param[in] <none>   
 * @return plhd irq status
 */
uint8_t PAN3031_get_early_irq(void)
{
	uint8_t tmpreg;
	
	tmpreg = PAN3031_read_spec_page_reg(PAGE1_SEL, 0x2d);

	return tmpreg;
}

/**
 * @brief set plhd
 * @param[in] <addr> PLHD start addr,Range:0..7f
 *		      <len> PLHD len
 *			  PLHD_LEN8 / PLHD_LEN16
 * @return result
 */
uint32_t PAN3031_set_plhd(uint8_t addr,uint8_t len)
{
	uint8_t temp_val_2;
	
	temp_val_2 = ((addr & 0x7f) | (len << 7)) ;

	if(PAN3031_write_spec_page_reg(PAGE1_SEL, 0x2e, temp_val_2) != OK)
	{
		return FAIL;
	}
	else
	{
		return OK;
	}
}

/**
 * @brief read plhd status
 * @param[in] <none>     
 * @return plhd status
 */
uint8_t PAN3031_get_plhd(void)
{
	uint8_t tmpreg;
	
	tmpreg = PAN3031_read_spec_page_reg(PAGE1_SEL, 0x2e);

	return ((tmpreg & 0x80) >> 7);
}

/**
 * @brief set plhd mask
 * @param[in] <plhd_val> plhd mask to set
 *		      PLHD_ON / PLHD_OFF
 * @return result
 */
uint32_t PAN3031_set_plhd_mask(uint32_t plhd_val)
{
	uint8_t temp_val_1;
	uint8_t temp_val_2;
	
	temp_val_1 = PAN3031_read_spec_page_reg(PAGE0_SEL, 0x58);
	temp_val_2 = ((temp_val_1 & 0xef) | (plhd_val << 4)) ;

	if(PAN3031_write_spec_page_reg(PAGE0_SEL, 0x58, temp_val_2) != OK)
	{
		return FAIL;
	}
	else
	{
		return OK;
	}
}

/**
 * @brief read plhd mask
 * @param[in] <none>    
 * @return plhd mask
 */
uint8_t PAN3031_get_plhd_mask(void)
{
	uint8_t tmpreg;
	
	tmpreg = PAN3031_read_spec_page_reg(PAGE0_SEL, 0x58);

	return tmpreg;
}

/**
 * @brief receive 8 bytes plhd data
 * @param[in] <buff> buffer provide for data to receive
 * @return result
 */
uint8_t PAN3031_recv_plhd8(uint8_t *buff)
{
	uint32_t i,len = 8;
	for(i = 0; i < len; i++)
	{
		buff[i] = PAN3031_read_spec_page_reg(PAGE2_SEL, 0x76 + i);
	}
	
	PAN3031_clr_irq();
	return len;
}

/**
 * @brief receive 16 bytes plhd data
 * @param[in] <buff> buffer provide for data to receive
 * @return result
 */
uint8_t PAN3031_recv_plhd16(uint8_t *buff)
{
	uint32_t i,len = 16;	
	for(i = 0; i < len; i++)
	{
		if(i<10)
		{
			buff[i] = PAN3031_read_spec_page_reg(PAGE2_SEL, 0x76 + i);
		}else{
			buff[i] = PAN3031_read_spec_page_reg(PAGE0_SEL, 0x76 + i - 10);
		}	
	}

	PAN3031_clr_irq();
	return len;
}

/**
 * @brief receive a packet in non-block method, it will return 0 when no data got
 * @param[in] <buff> buffer provide for data to receive
 *			  <len> PLHD_LEN8 / PLHD_LEN16
 * @return result
 */
uint32_t PAN3031_plhd_receive(uint8_t *buf,uint8_t len)
{
	if(len == PLHD_LEN8)
	{
		return PAN3031_recv_plhd8(buf);
	}else if (len == PLHD_LEN16)
	{
		return PAN3031_recv_plhd16(buf);
	}
	return FAIL;
}

/**
 * @brief set dcdc mode, The default configuration is DCDC_OFF, PAN3031 should set DCDC_OFF before enter sleep/deepsleep
 * @param[in] <dcdc_val> dcdc switch
 *		      DCDC_ON / DCDC_OFF
 * @return result
 */
uint32_t PAN3031_set_dcdc_mode(uint32_t dcdc_val)
{
	uint8_t temp_val_1;
	uint8_t temp_val_2;
	
	temp_val_1 = PAN3031_read_spec_page_reg(PAGE3_SEL, 0x1e);
	temp_val_2 = ((temp_val_1 & 0xfe) | (dcdc_val << 0)) ;

	if(PAN3031_write_spec_page_reg(PAGE3_SEL, 0x1e, temp_val_2) != OK)
	{
		return FAIL;
	}
	else
	{
		return OK;
	}
}


/**
 * @brief set LDR mode
 * @param[in] <mode> LDR switch
 *		      LDR_ON / LDR_OFF
 * @return result
 */
uint32_t PAN3031_set_ldr(uint32_t mode)
{
	uint8_t temp_val_1;
	uint8_t temp_val_2;
	
	temp_val_1 = PAN3031_read_spec_page_reg(PAGE3_SEL, 0x12);
	temp_val_2 = ((temp_val_1 & 0xF7) | (mode << 3)) ;
	if(PAN3031_write_spec_page_reg(PAGE3_SEL, 0x12, temp_val_2) != OK)
	{
		return FAIL;
	}
	else
	{
		return OK;
	}
}

/**
 * @brief RF IRQ server routine, it should be call at ISR of IRQ pin
 * @param[in] <none>
 * @return result
 */
void PAN3031_irq_handler(void)
{
	double snr,rssi,plhd_len;
	uint16_t size = 0;
	uint8_t irq = PAN3031_get_irq();

	if(irq & REG_IRQ_RX_PLHD_DONE)
	{
		plhd_len = PAN3031_get_plhd();
		size = PAN3031_plhd_receive(plhd_buf,plhd_len);
		rf_rx_plhddone_event( plhd_buf, size );

	}else if(irq & REG_IRQ_RX_DONE)
	{
		snr = PAN3031_get_snr();
		rssi = PAN3031_get_rssi();
		size = PAN3031_recv_packet(RadioRxPayload);
		rf_rx_done_event( RadioRxPayload, size, rssi, snr );

	}
	else if(irq & REG_IRQ_CRC_ERR)
	{
		rf_rx_err_event();
		PAN3031_clr_irq();

	}
	else if(irq & REG_IRQ_RX_TIMEOUT)
	{
		rf_rx_timeout_event();
		PAN3031_clr_irq();

	}
	else if(irq & REG_IRQ_TX_DONE)
	{
		rf_tx_done_event();
		PAN3031_clr_irq();

	}
}


/**
 * @brief set carrier wave test mode
 * @param[in] <none>
 * @return result
 */
uint32_t PAN3031_set_carrier_wave_test_mode(void)
{
	if(PAN3031_write_reg(0x02, 0x00) != OK)
	{
		return FAIL;
	}  
	rf_port.delayms(8);
	
	if(PAN3031_write_reg(0x02, 0x01) != OK)
	{
		return FAIL;
	}    
	rf_port.delayms(8);

	if(PAN3031_write_reg(0x04, 0x16) != OK)
	{
		return FAIL;
	}
	rf_port.delayms(8);

	if(PAN3031_write_reg(0x04, 0x56) != OK)
	{
		return FAIL;
	}
	rf_port.delayms(8);

	if(PAN3031_write_reg(0x04, 0x76) != OK)
	{
		return FAIL;
	}    
	rf_port.delayms(8);

	rf_port.antenna_init();
	rf_port.tcxo_init();
	rf_port.delayms(8);

	if(PAN3031_write_reg(0x04, 0xF6) != OK)
	{
		return FAIL;
	}
	rf_port.delayms(8);

	if(PAN3031_write_reg(0x02, 0x02) != OK)
	{
		return FAIL;
	}  
	rf_port.delayms(8);

	if(PAN3031_write_spec_page_reg(PAGE3_SEL, 0x24, 0x60) != OK)
	{
		return FAIL;
	}
	rf_port.delayms(8);

	if(PAN3031_write_reg(0x02, 0x03) != OK)
	{
		return FAIL;
	} 
	rf_port.delayms(8);

	if(PAN3031_write_spec_page_reg(PAGE3_SEL, 0x24, 0xE0) != OK)
	{
		return FAIL;
	}  
	rf_port.delayms(8);

	if(PAN3031_write_reg(0x02, 0x04) != OK)
	{
		return FAIL;
	} 
	rf_port.delayms(8);

	if(PAN3031_write_spec_page_reg(PAGE3_SEL, 0x24, 0xF0) != OK)
	{
		return FAIL;
	}  
	rf_port.delayms(8);	
	if(PAN3031_write_spec_page_reg(PAGE0_SEL,0x4c,0xbf)  != OK)
	{
		return FAIL;
	}	
	rf_port.delayms(8);
	if(PAN3031_write_spec_page_reg(PAGE0_SEL, 0x4a, 0x92) != OK)//92   8e
	{
		return FAIL;
	} 
	rf_port.delayms(8);

	if(PAN3031_write_spec_page_reg(PAGE3_SEL, 0x18, 0x0f) != OK)
	{
		return FAIL;
	}     
	rf_port.delayms(8);

	if(PAN3031_write_spec_page_reg(PAGE3_SEL, 0x15, 0x58) != OK)
	{
		return FAIL;
	}     
	rf_port.delayms(8);

	if(PAN3031_write_spec_page_reg(PAGE3_SEL, 0x16, 0x64) != OK)
	{
		return FAIL;
	}     
	rf_port.delayms(8);

	if(PAN3031_write_spec_page_reg(PAGE3_SEL, 0x17, 0x00) != OK)
	{
		return FAIL;
	}         
	rf_port.delayms(8);

	if(PAN3031_write_spec_page_reg(PAGE1_SEL, 0x66, 0x7f) != OK)
	{
		return FAIL;
	}     
	rf_port.delayms(8);

	if(PAN3031_write_spec_page_reg(PAGE1_SEL, 0x65, 0xf7) != OK)
	{
		return FAIL;
	}  
	rf_port.delayms(8);

	if(PAN3031_write_spec_page_reg(PAGE0_SEL, 0x17, 0x08) != OK)
	{
		return FAIL;
	}  
	rf_port.delayms(8);

	if(PAN3031_write_spec_page_reg(PAGE0_SEL, 0x18, 0x28) != OK)
	{
		return FAIL;
	}   
	rf_port.delayms(8);  
	
	rf_port.set_tx();

	return OK;
}
