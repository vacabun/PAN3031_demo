/*******************************************************************************
 * @note Copyright (C) 2020 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file pan3031_port.c
 * @brief
 *
 * @history - V3.0, 2021-07-12
*******************************************************************************/
#include "stm32f0xx.h"
#include "pan3031_port.h"
#include "radio.h"
#include "pan3031.h"
#include "spi.h"


extern SPI_HandleTypeDef hspi2;

extern uint8_t spi_tx_rx(uint8_t tx_data);

rf_port_t rf_port =
	{
		.antenna_init = rf_antenna_init,
		.tcxo_init = rf_tcxo_init,
		.set_tx = rf_antenna_tx,
		.set_rx = rf_antenna_rx,
		.antenna_close = rf_antenna_close,
		.tcxo_close = rf_tcxo_close,
		.spi_readwrite = spi_readwritebyte,
		.spi_cs_high = spi_cs_set_high,
		.spi_cs_low = spi_cs_set_low,
		.delayms = rf_delay_ms,
		.delayus = rf_delay_us,
};

/**
 * @brief spi_readwritebyte
 * @param[in] <tx_data> spi readwritebyte value
 * @return result
 */
uint8_t spi_readwritebyte(uint8_t tx_data)
{
    uint8_t rx_data;
    HAL_SPI_TransmitReceive(&hspi2,&tx_data,&rx_data,1, 1000);
    return rx_data;

//	while (RESET == SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE))
//	{
//	}
//
//	SPI_SendData8(SPI2, tx_data);
//
//	while (RESET == SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE))
//	{
//	}
//
//	return SPI_ReceiveData8(SPI2);
}

/**
 * @brief spi_cs_set_high
 * @param[in] <none>
 * @return none
 */
void spi_cs_set_high(void)
{
	// GPIO_SetBits(RF_NSS_PORT,RF_NSS_IO);
    HAL_GPIO_WritePin(RF_NSS_GPIO_Port,RF_NSS_Pin,GPIO_PIN_SET);
}

/**
 * @brief spi_cs_set_low
 * @param[in] <none>
 * @return none
 */
void spi_cs_set_low(void)
{
	// GPIO_ResetBits(RF_NSS_PORT,RF_NSS_IO);
    HAL_GPIO_WritePin(RF_NSS_GPIO_Port,RF_NSS_Pin,GPIO_PIN_RESET);
}

/**
 * @brief rf_delay_ms
 * @param[in] <time> ms
 * @return none
 */
void rf_delay_ms(uint32_t time)
{
	// Delay_Ms(time);
    HAL_Delay(time);
}

/**
 * @brief rf_delay_us
 * @param[in] <time> us
 * @return none
 */
void rf_delay_us(uint32_t time)
{

    HAL_Delay(time/1000);
}

/**
 * @brief do PAN3031 TX/RX IO to initialize
 * @param[in] <none>
 * @return none
 */
void rf_antenna_init(void)
{
	PAN3031_set_gpio_output(MODULE_GPIO_RX);
	PAN3031_set_gpio_output(MODULE_GPIO_TX);

	PAN3031_set_gpio_state(MODULE_GPIO_RX, 0);
	PAN3031_set_gpio_state(MODULE_GPIO_TX, 0);
}

/**
 * @brief do PAN3031 XTAL IO to initialize
 * @param[in] <none>
 * @return none
 */
void rf_tcxo_init(void)
{
	PAN3031_set_gpio_output(MODULE_GPIO_TCXO);
	PAN3031_set_gpio_state(MODULE_GPIO_TCXO, 1);
}

/**
 * @brief close PAN3031 XTAL IO 
 * @param[in] <none>
 * @return none
 */
void rf_tcxo_close(void)
{
	PAN3031_set_gpio_output(MODULE_GPIO_TCXO);
	PAN3031_set_gpio_state(MODULE_GPIO_TCXO, 0);
}
/**
 * @brief change PAN3031 IO to rx
 * @param[in] <none>
 * @return none
 */
void rf_antenna_rx(void)
{
	PAN3031_set_gpio_state(MODULE_GPIO_TX, 0);
	PAN3031_set_gpio_state(MODULE_GPIO_RX, 1);
}

/**
 * @brief change PAN3031 IO to tx
 * @param[in] <none>
 * @return none
 */
void rf_antenna_tx(void)
{
	PAN3031_set_gpio_state(MODULE_GPIO_RX, 0);
	PAN3031_set_gpio_state(MODULE_GPIO_TX, 1);
}

/**
 * @brief change PAN3031 IO to close
 * @param[in] <none>
 * @return none
 */
void rf_antenna_close(void)
{
	PAN3031_set_gpio_state(MODULE_GPIO_TX, 0);
	PAN3031_set_gpio_state(MODULE_GPIO_RX, 0);
}
