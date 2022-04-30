/*******************************************************************************
 * @note Copyright (C) 2020 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file radio.c
 * @brief
 *
 * @history - V3.0, 2021-07-12
*******************************************************************************/
#include "pan3031.h"
#include "radio.h"
#include "stdlib.h"
#include "stm32f0xx_hal.h"
#include "stdio.h"

/*
 * flag that indicate if a new packet is received.
*/
static int packet_received = RADIO_FLAG_IDLE;

/*
 * flag that indicate if transmision is finished.
*/
static int packet_transmit = RADIO_FLAG_IDLE;

struct RxDoneMsg RxDoneParams;

/**
 * @brief get receive flag 
 * @param[in] <none>
 * @return receive state
 */
uint32_t rf_get_recv_flag(void)
{
	return packet_received;
}

/**
 * @brief set receive flag 
 * @param[in] <status> receive flag state to set
 * @return none
 */
void rf_set_recv_flag(int status)
{
	packet_received = status;
}

/**
 * @brief get transmit flag 
 * @param[in] <none>
 * @return reansmit state
 */
uint32_t rf_get_transmit_flag(void)
{
	return packet_transmit;
}

/**
 * @brief set transmit flag 
 * @param[in] <status> transmit flag state to set
 * @return none
 */
void rf_set_transmit_flag(int status)
{
	packet_transmit = status;
}

/**
 * @brief do basic configuration to initialize
 * @param[in] <none>
 * @return result
 */
uint32_t rf_init(void)
{
	if(PAN3031_deepsleep_wakeup() != OK)
	{

		return FAIL;


	}



	if(PAN3031_init() != OK)
	{
		return FAIL;
	}

	if(rf_set_agc(AGC_ON) != OK)
	{
		return FAIL;
	}

	rf_port.antenna_init();

	return OK;    
}

/**
 * @brief change PAN3031 mode from deep sleep to wakeup(STB3)
 * @param[in] <none>
 * @return result
 */
uint32_t rf_deepsleep_wakeup(void)
{
	if(PAN3031_deepsleep_wakeup() != OK)
	{
		return FAIL;
	} 

	if(PAN3031_init() != OK)
	{
		return FAIL;
	} 

	if(rf_set_agc(AGC_ON) != OK)
	{
		return FAIL;
	} 

	rf_port.antenna_init();

	return OK; 
}

/**
 * @brief change PAN3031 mode from sleep to wakeup(STB3) 
 * @param[in] <none>
 * @return result
 */
uint32_t rf_sleep_wakeup(void)
{
	if(PAN3031_sleep_wakeup() != OK)
	{
		return FAIL;
	}     
	rf_port.antenna_init();
	return OK;
}

/**
 * @brief change PAN3031 mode from standby3(STB3) to deep sleep, PAN3031 should set DCDC_OFF before enter deepsleep
 * @param[in] <none>
 * @return result
 */
uint32_t rf_deepsleep(void)
{
	rf_port.antenna_close();
	return PAN3031_deepsleep();
}

/**
 * @brief change PAN3031 mode from standby3(STB3) to deep sleep, PAN3031 should set DCDC_OFF before enter sleep
 * @param[in] <none>
 * @return result
 */
uint32_t rf_sleep(void)
{
	rf_port.antenna_close();
	return PAN3031_sleep();
}
	
/**
 * @brief calculate tx time
 * @param[in] <none>
 * @return tx time(ms) 
 */
uint32_t rf_get_tx_time(void)
{
	return PAN3031_calculate_tx_time();
}

/**
 * @brief set rf mode
 * @param[in] <mode>    
 *			  PAN3031_MODE_DEEP_SLEEP / PAN3031_MODE_SLEEP
 *            PAN3031_MODE_STB1 / PAN3031_MODE_STB2
 *            PAN3031_MODE_STB3 / PAN3031_MODE_TX / PAN3031_MODE_RX
 * @return result
 */
uint32_t rf_set_mode(uint8_t mode)
{
	return PAN3031_set_mode(mode);
}

/**
 * @brief get rf mode
 * @param[in] <none>
 * @return mode 
 *		   PAN3031_MODE_DEEP_SLEEP / PAN3031_MODE_SLEEP
 *         PAN3031_MODE_STB1 / PAN3031_MODE_STB2
 *         PAN3031_MODE_STB3 / PAN3031_MODE_TX / PAN3031_MODE_RX
 */
uint8_t rf_get_mode(void)
{
	return PAN3031_get_mode();
}

/**
 * @brief set rf Tx mode
 * @param[in] <mode> 
 *			  PAN3031_TX_SINGLE/PAN3031_TX_CONTINOUS
 * @return result
 */
uint32_t rf_set_tx_mode(uint8_t mode)
{
	return PAN3031_set_tx_mode(mode);
}

/**
 * @brief set rf Rx mode
 * @param[in] <mode> 
 *			  PAN3031_RX_SINGLE/PAN3031_RX_SINGLE_TIMEOUT/PAN3031_RX_CONTINOUS
 * @return result
 */
uint32_t rf_set_rx_mode(uint8_t mode)
{
	return PAN3031_set_rx_mode(mode);
}

/**
 * @brief set timeout for Rx. It is useful in PAN3031_RX_SINGLE_TIMEOUT mode
 * @param[in] <timeout> rx single timeout time(in ms)
 * @return result
 */
uint32_t rf_set_rx_single_timeout(uint32_t timeout)
{
	return PAN3031_set_timeout(timeout);
}

/**
 * @brief get snr value
 * @param[in] <none> 
 * @return snr
 */
float rf_get_snr(void)
{
	return PAN3031_get_snr();
}

/**
 * @brief get rssi value
 * @param[in] <none> 
 * @return rssi
 */
float rf_get_rssi(void)
{
	return PAN3031_get_rssi();
}

/**
 * @brief set preamble 
 * @param[in] <reg> preamble
 * @return result
 */
uint32_t rf_set_preamble(uint16_t pream)
{
	return PAN3031_set_preamble(pream);
}

/**
 * @brief CAD function enable
 * @param[in] <none> 
 * @return  result
 */
uint32_t rf_set_cad(void)
{
	return PAN3031_cad_en();
}

/**
 * @brief set rf syncword
 * @param[in] <sync> syncword
 * @return result
 */
uint32_t rf_set_syncword(uint8_t sync)
{
	return PAN3031_set_syncword(sync);
}

/**
 * @brief read rf syncword
 * @param[in] <none>   
 * @return syncword
 */
uint8_t rf_get_syncword(void)
{
	return PAN3031_get_syncword();
}

/**
 * @brief RF IRQ server routine, it should be call at ISR of IRQ pin
 * @param[in] <none>
 * @return result
 */
void rf_irq_handler(void)
{
	PAN3031_irq_handler();
}

/**
 * @brief set rf plhd mode on , rf will use early interruption
 * @param[in] <addr> PLHD start addr,Range:0..7f
		      <len> PLHD len
			  PLHD_LEN8 / PLHD_LEN16
 * @return result
 */
void rf_set_plhd_rx_on(uint8_t addr,uint8_t len)
{
	PAN3031_set_early_irq(PLHD_IRQ_ON);
	PAN3031_set_plhd(addr,len);
	PAN3031_set_plhd_mask(PLHD_ON);
}

/**
 * @brief set rf plhd mode off
 * @param[in] <none>
 * @return result
 */
void rf_set_plhd_rx_off(void)
{
	PAN3031_set_early_irq(PLHD_IRQ_OFF);
	PAN3031_set_plhd_mask(PLHD_OFF);
}

/**
 * @brief receive a packet in non-block method, it will return 0 when no data got
 * @param[in] <buff> buffer provide for data to receive
 * @return length, it will return 0 when no data got
 */
uint32_t rf_receive(uint8_t *buf)
{
	return PAN3031_recv_packet(buf);
}

/**
 * @brief receive a packet in non-block method, it will return 0 when no data got
 * @param[in] <buff> buffer provide for data to receive
			   <len> PLHD_LEN8 / PLHD_LEN16
 * @return result
 */
uint32_t rf_plhd_receive(uint8_t *buf,uint8_t len)
{
	return PAN3031_plhd_receive(buf,len);
}

/**
 * @brief RF PAN3031_irq_handler OnRadioRxPlhdDone callbact,it will use in Plhd Mode
 * @param[in] <payload> recv packet
 * @param[in] <size> the length of recv packet
 * @return none
 */
__weak void rf_rx_plhddone_event( uint8_t *payload, uint16_t size )
{
	RxDoneParams.PlhdSize = size;
	RxDoneParams.PlhdPayload = payload;
	rf_set_recv_flag(RADIO_FLAG_PLHDRXDONE);

	PAN3031_rst();//stop it
}

/**
 * @brief RF PAN3031_irq_handler OnRadioRxDone callbact,it will use in PAN3031_RX_SINGLE/PAN3031_RX_SINGLE_TIMEOUT/PAN3031_RX_CONTINOUS Mode
 * @param[in] <payload> recv packet
 * @param[in] <size> the length of recv packet
 * @param[in] <rssi> the rssi of recv packet
 * @param[in] <snr> the snr of recv packet
 * @return none
 */
__weak void rf_rx_done_event( uint8_t *payload, uint16_t size, double rssi, double snr )
{
	RxDoneParams.Payload = payload;
	RxDoneParams.Size = size;
	RxDoneParams.Rssi = rssi;
	RxDoneParams.Snr = snr;

	rf_set_recv_flag(RADIO_FLAG_RXDONE);
}

/**
 * @brief RF PAN3031_irq_handler OnRadioRxErr callbact,it will use in PAN3031_RX_SINGLE/PAN3031_RX_SINGLE_TIMEOUT/PAN3031_RX_CONTINOUS Mode
 * @param[in] <none> 
 * @return none
 */
__weak void rf_rx_err_event(void)
{
	rf_set_recv_flag(RADIO_FLAG_RXERR);
}

/**
 * @brief RF PAN3031_irq_handler OnRadioRxTimeout callbact,it will use in PAN3031_RX_SINGLE/PAN3031_RX_SINGLE_TIMEOUT/PAN3031_RX_CONTINOUS Mode
 * @param[in] <none> 
 * @return none
 */
__weak void rf_rx_timeout_event(void)
{
	rf_set_recv_flag(RADIO_FLAG_RXTIMEOUT);
}

/**
 * @brief RF PAN3031_irq_handler OnRadioTxDone callbact,it will use in tx Mode
 * @param[in] <none> 
 * @return none
 */
__weak void rf_tx_done_event(void)
{
	rf_set_transmit_flag(RADIO_FLAG_TXDONE);
}

/**
 * @brief rf enter rx continous mode to receive packet
 * @param[in] <none> 
 * @return result
 */
uint32_t rf_enter_continous_rx(void)
{
	if(PAN3031_set_mode(PAN3031_MODE_STB3) != OK)
	{
		return FAIL;
	}

	rf_port.set_rx();

	if(PAN3031_set_rx_mode(PAN3031_RX_CONTINOUS) != OK)
	{
		return FAIL;
	} 

	if(PAN3031_set_mode(PAN3031_MODE_RX) != OK)
	{
		return FAIL;
	} 
	return OK;
}

/**
 * @brief rf enter rx single timeout mode to receive packet
 * @param[in] <timeout> rx single timeout time(in ms)
 * @return result
 */
uint32_t rf_enter_single_timeout_rx(uint32_t timeout)
{
	if(PAN3031_set_mode(PAN3031_MODE_STB3) != OK)
	{
		return FAIL;
	}

	rf_port.set_rx();

	if(PAN3031_set_rx_mode(PAN3031_RX_SINGLE_TIMEOUT) != OK)
	{
		return FAIL;
	} 

	if(PAN3031_set_timeout(timeout) != OK)
	{
		return FAIL;
	}  

	if(PAN3031_set_mode(PAN3031_MODE_RX) != OK)
	{
		return FAIL;
	} 
	return OK;
}

/**
 * @brief rf enter rx single mode to receive packet
 * @param[in] <none> 
 * @return result
 */
uint32_t rf_enter_single_rx(void)
{
	if(PAN3031_set_mode(PAN3031_MODE_STB3) != OK)
	{
		return FAIL;
	}

	rf_port.set_rx();

	if(PAN3031_set_rx_mode(PAN3031_RX_SINGLE) != OK)
	{
		return FAIL;
	} 

	if(PAN3031_set_mode(PAN3031_MODE_RX) != OK)
	{
		return FAIL;
	} 
	return OK;
}

/**
 * @brief rf enter single tx mode and send packet
 * @param[in] <buf> buffer contain data to send
 * @param[in] <size> the length of data to send
 * @param[in] <tx_time> the packet tx time
 * @return result
 */
uint32_t rf_single_tx_data(uint8_t *buf, uint8_t size, uint32_t *tx_time)
{     
	if(PAN3031_set_mode(PAN3031_MODE_STB3) != OK)
	{
		return FAIL;
	}

	rf_port.set_tx();

	if(PAN3031_set_tx_mode(PAN3031_TX_SINGLE) != OK)
	{
		return FAIL;
	}  

	if(PAN3031_send_packet(buf, size) != OK)
	{
		return FAIL;
	}

	*tx_time = rf_get_tx_time();

	return OK;
}

/**
 * @brief rf enter continous tx mode to ready send packet
 * @param[in] <none> 
 * @return result
 */
uint32_t rf_enter_continous_tx(void)
{
	if(PAN3031_set_mode(PAN3031_MODE_STB3) != OK)
	{
		return FAIL;
	}

	rf_port.set_tx();

	if(PAN3031_set_tx_mode(PAN3031_TX_CONTINOUS) != OK)
	{
		return FAIL;
	}	
	
	return OK;
}
	
/**
 * @brief rf continous mode send packet
 * @param[in] <buf> buffer contain data to send
 * @param[in] <size> the length of data to send
 * @return result
 */
uint32_t rf_continous_tx_send_data(uint8_t *buf, uint8_t size)
{   
	if(PAN3031_send_packet(buf, size) != OK)
	{
		return FAIL;
	}

	return OK;
}

/**
 * @brief enable AGC function
 * @param[in] <state>  
 *			  AGC_OFF/AGC_ON
 * @return result
 */
uint32_t rf_set_agc(uint32_t state)
{
	if(PAN3031_agc_enable( state ) != OK)
	{
		return FAIL;
	}
	if(PAN3031_agc_config() != OK)
	{
		return FAIL;
	}
	return OK;
}

/**
 * @brief set rf para
 * @param[in] <para_type> set type, rf_para_type_t para_type
 * @param[in] <para_val> set value
 * @return result
 */
uint32_t rf_set_para(rf_para_type_t para_type, uint32_t para_val)
{
	PAN3031_set_mode(PAN3031_MODE_STB3);
	switch(para_type)
	{
		case RF_PARA_TYPE_FREQ:
			PAN3031_set_freq(para_val);  
			PAN3031_rst();
			break;
		case RF_PARA_TYPE_CR:
			PAN3031_set_code_rate(para_val);
			PAN3031_rst();
			break;
		case RF_PARA_TYPE_BW:
			PAN3031_set_bw(para_val);  
			PAN3031_rst();            
			break;
		case RF_PARA_TYPE_SF:
			PAN3031_set_sf(para_val);  
			PAN3031_rst();
			break;
		case RF_PARA_TYPE_TXPOWER:
			PAN3031_set_tx_power(para_val);
			PAN3031_rst(); 
			break;
		case RF_PARA_TYPE_CRC:
			PAN3031_set_crc(para_val);
			PAN3031_rst(); 
			break;
		default:
			break;    
	}
	return OK;
}

/**
 * @brief get rf para
 * @param[in] <para_type> get typ, rf_para_type_t para_type
 * @param[in] <para_val> get value
 * @return result
 */
uint32_t rf_get_para(rf_para_type_t para_type, uint32_t *para_val)
{
	PAN3031_set_mode(PAN3031_MODE_STB3);
	switch(para_type)
	{
		case RF_PARA_TYPE_FREQ:
			*para_val = PAN3031_read_freq();  
			break;
		case RF_PARA_TYPE_CR:
			*para_val = PAN3031_get_code_rate();
			break;
		case RF_PARA_TYPE_BW:
			*para_val = PAN3031_get_bw();
			break;
		case RF_PARA_TYPE_SF:
			*para_val = PAN3031_get_sf();          
			break;
		case RF_PARA_TYPE_TXPOWER:
			*para_val = PAN3031_get_tx_power();
			break;
		case RF_PARA_TYPE_CRC:
			*para_val = PAN3031_get_crc();          
			break;
		default:
			break;    
	}
	return OK;
}

/**
 * @brief set rf default para
 * @param[in] <none>
 * @return result
 */
void rf_set_default_para(void)
{
	PAN3031_set_mode(PAN3031_MODE_STB3); //参数配置通常 在 standby3 状态下进行
	rf_set_para(RF_PARA_TYPE_FREQ, DEFAULT_FREQ);//频率设置
	rf_set_para(RF_PARA_TYPE_CR, DEFAULT_CR);  //注：空中速率通过 SF、BW、CR三个参数确定   参考资料包里的 PAN3031计算器
	rf_set_para(RF_PARA_TYPE_BW, DEFAULT_BW);
	rf_set_para(RF_PARA_TYPE_SF, DEFAULT_SF);
	rf_set_para(RF_PARA_TYPE_TXPOWER, 0X7F);//发射功率表 参考：PAN3031_SDK用户指南
	rf_set_para(RF_PARA_TYPE_CRC, CRC_ON);//打开硬件CRC
	rf_set_dcdc_mode(DCDC_OFF);//关闭DCDC
	rf_set_ldr(LDR_OFF);
}

/**
 * @brief set dcdc mode, The default configuration is DCDC_OFF, PAN3031 should set DCDC_OFF before enter sleep/deepsleep
 * @param[in] <dcdc_val> dcdc switch
 *		      DCDC_ON / DCDC_OFF
 * @return result
 */
uint32_t rf_set_dcdc_mode(uint32_t dcdc_val)
{
	return PAN3031_set_dcdc_mode(dcdc_val);
}

/**
 * @brief set LDR mode
 * @param[in] <mode> LDR switch
 *		      LDR_ON / LDR_OFF
 * @return result
 */
uint32_t rf_set_ldr(uint32_t mode)
{
	return PAN3031_set_ldr(mode);
}



