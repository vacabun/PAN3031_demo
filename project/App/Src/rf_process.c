//
// Created by maguo on 2022/4/30.
//
#include "rf_process.h"
#include "radio.h"
#include "main.h"

extern struct RxDoneMsg RxDoneParams;


uint32_t tx_times = 0;
uint8_t temp;
uint8_t n = 0;
void rf_tx_demo(void){

    tx_times++;
    uint8_t tx_test_buf[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int len = sizeof(tx_test_buf) / sizeof(tx_test_buf[0]);
    n++;
    if(n == 10)
        n = 0;
    for (int i = 0; i < n; i++) {
        temp = tx_test_buf[0];
        for (int j = 0; j < len - 1; j++) {
            tx_test_buf[j] = tx_test_buf[j + 1];
        }
        tx_test_buf[len - 1] = temp;
    }

    if (rf_continous_tx_send_data(tx_test_buf, 10) == OK)
    {

        // wait for TX Done. transmit flag will be set when TXDONE IRQ received
        while (RADIO_FLAG_IDLE == rf_get_transmit_flag())
            ;
        rf_set_transmit_flag(RADIO_FLAG_IDLE);
        LedToggle();
        printf("msg index %lu send finish.\r\n",tx_times);
    }
    else
    {
        HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);
        printf("send err.\r\n");
    }
}

void rf_rx_demo(void){
    uint8_t rx_buf[100] = {0};
    if (rf_get_recv_flag() == RADIO_FLAG_RXDONE)
    {
        rf_set_recv_flag(RADIO_FLAG_IDLE);
        double Rssi_dBm = RxDoneParams.Rssi;
        double Snr_value = RxDoneParams.Snr;
        uint16_t rx_len = RxDoneParams.Size;

        for (uint8_t i = 0; i < rx_len; i++)
        {
            rx_buf[i] = RxDoneParams.Payload[i];
        }
        LedToggle();

        // log
        printf("RSSI:%.03f\r\n", Rssi_dBm);
        printf("SNR:%.03f\r\n", Snr_value);
        printf("RX:{");
        for (uint8_t i = 0; i < rx_len; i++)
        {
            printf(" %x",rx_buf[i]);
        }
        printf("}\r\n");
    }
    if ((rf_get_recv_flag() == RADIO_FLAG_RXTIMEOUT) || (rf_get_recv_flag() == RADIO_FLAG_RXERR))
    {
        rf_set_recv_flag(RADIO_FLAG_IDLE);
    }
}