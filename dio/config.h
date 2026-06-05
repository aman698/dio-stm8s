#pragma once
#ifndef CONFIG_H
#define CONFIG_H
// Made by Aman Lakra
#define PIN_SPI_SCK PB_3
#define PIN_SPI_MISO PB_4
#define PIN_SPI_MOSI PB_5
#define ETHERNET_SS_PIN PB0

/*
   ETHERNET CONFIGS
*/

#define SERVER_PORT 1000
byte SERVER_MAC[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
  //0xDE, 0xAD, 0xBE, 0xEF, 0xBF, 0xAE
};
const int ip_array[4] = {192, 168, 0, 1};

/*
   SERIAL CONFIGS
*/
#define SERIAL_BAUDRATE 115200
#define SerialInterface Serial
#define COM_PORT Serial


/*
   RELAY PINOUTS
*/
#define RELAY1 PC6
#define RELAY2 PA9
#define RELAY3 PA8
#define RELAY4 PB15
#define RELAY5 PB14
#define RELAY6 PB13

/*
   Input Pins Define
*/
#define DI2 PC_9  //PC9
#define DI1 PD0  
#define DI3 PD1
#define DI4 PD2
#define DI5 PD3
#define DI6 PD4
#define DI7 PD_5 //PD5
#define DI8 PD_6 //PD6
#define DI9 PB3
#define DI10 PB4

/*
   HARD RESET PIN
*/
#define HARDRST PC_3

#endif
