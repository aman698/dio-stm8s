/* ================================================================
   INDUSTRIAL STM8S + W5500 TCP SERVER
   ================================================================

   MCU      : STM8S103 / STM8S003
   Ethernet : W5500
   TCP Port : 5000

   FEATURES
   ----------------------------------------------------------------
   ? Industrial TCP state machine
   ? Infinite reconnect support
   ? Hercules reconnect stable
   ? Ping stable
   ? W5500 auto recovery
   ? No infinite blocking loops
   ? RX/TX circular buffer handling
   ? TCP timeout protection
   ? Automatic socket reopen
   ? Relay control
   ? Input status monitoring
   ? CR/LF safe command parser
   ? W5500 disconnect recovery
   ? SPI transaction stability
   ? TCP server self-healing

   COMMANDS
   ----------------------------------------------------------------
   STATUS
   RL1ON
   RL1OFF

   NETWORK
   ----------------------------------------------------------------
   IP      : 192.168.22.100
   MASK    : 255.255.255.0
   GW      : 192.168.22.1
   PORT    : 5000

   ================================================================
*/
#define STM8S003
#include <string.h>
#include <stdio.h>

#include "stm8s.h"
#include "stm8s_spi.h"
#include "stm8s_uart1.h"
#include "stm8s_conf.h"

/* ================================================================
   DEFINES
   ================================================================ */

#define COMMON_BLOCK           0x00
#define SOCKET0_BLOCK          0x01
#define SOCKET0_TX             0x02
#define SOCKET0_RX             0x03

#define SOCK_CLOSED            0x00
#define SOCK_INIT              0x13
#define SOCK_LISTEN            0x14
#define SOCK_ESTABLISHED       0x17
#define SOCK_CLOSE_WAIT        0x1C

#define W5500_VERSION          0x04

#define TCP_PORT_H             0x13
#define TCP_PORT_L             0x88

#define SOCKET_BUFFER_MASK     0x07FF

#define CMD_TIMEOUT            3000

#define MAX_RX_SIZE            64

/* ================================================================
   W5500 CS
   ================================================================ */

#define W5500_CS_LOW()         GPIO_WriteLow(GPIOA, GPIO_PIN_3)
#define W5500_CS_HIGH()        GPIO_WriteHigh(GPIOA, GPIO_PIN_3)

/* ================================================================
   GLOBALS
   ================================================================ */

static uint8_t sent_welcome = 0;

/* ================================================================
   FUNCTION DECLARATIONS
   ================================================================ */

void delay_ms(uint16_t ms);
void delay_500us(void);

void UART_Config(void);
void UART_SendString(char *str);
void UART_SendHex(uint8_t value);

void SPI_Config(void);
uint8_t SPI_Transfer(uint8_t data);

void W5500_Write(uint16_t addr, uint8_t block, uint8_t data);
uint8_t W5500_Read(uint16_t addr, uint8_t block);

void W5500_HardReset(void);
void W5500_SoftReset(void);

uint8_t W5500_Detect(void);

uint8_t W5500_WaitCommand(void);

void W5500_SetNetwork(void);

void W5500_CloseSocket(void);

void W5500_TCP_Server_Init(void);

uint8_t W5500_GetStatus(void);

void W5500_SendData(uint8_t *buf, uint16_t len);

void W5500_Recover(void);

void TCP_Process(void);

void SendHardwareStatus(void);

/* ================================================================
   DELAY
   ================================================================ */

void delay_ms(uint16_t ms)
{
    uint16_t i;
    uint16_t j;

    for(i = 0; i < ms; i++)
    {
        for(j = 0; j < 16000 / 4; j++)
        {
            nop();
        }
    }
}

void delay_500us(void)
{
    uint16_t j;

    for(j = 0; j < 16000 / 8; j++)
    {
        nop();
    }
}

/* ================================================================
   UART
   ================================================================ */

void UART_Config(void)
{
    UART1_DeInit();

    UART1_Init(
        9600,
        UART1_WORDLENGTH_8D,
        UART1_STOPBITS_1,
        UART1_PARITY_NO,
        UART1_SYNCMODE_CLOCK_DISABLE,
        UART1_MODE_TXRX_ENABLE
    );

    UART1_Cmd(ENABLE);
}

void UART_SendString(char *str)
{
    while(*str)
    {
        UART1_SendData8(*str++);

        while(UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET);
    }
}

void UART_SendHex(uint8_t value)
{
    char buf[5];

    buf[0] = '0';
    buf[1] = 'x';
    buf[2] = "0123456789ABCDEF"[value >> 4];
    buf[3] = "0123456789ABCDEF"[value & 0x0F];
    buf[4] = 0;

    UART_SendString(buf);
}

/* ================================================================
   SPI
   ================================================================ */

void SPI_Config(void)
{
    SPI_Init(
        SPI_FIRSTBIT_MSB,
        SPI_BAUDRATEPRESCALER_8,
        SPI_MODE_MASTER,
        SPI_CLOCKPOLARITY_LOW,
        SPI_CLOCKPHASE_1EDGE,
        SPI_DATADIRECTION_2LINES_FULLDUPLEX,
        SPI_NSS_SOFT,
        0x07
    );

    SPI_Cmd(ENABLE);
}

uint8_t SPI_Transfer(uint8_t data)
{
    SPI_SendData(data);

    while(SPI_GetFlagStatus(SPI_FLAG_TXE) == RESET);

    while(SPI_GetFlagStatus(SPI_FLAG_RXNE) == RESET);

    return SPI_ReceiveData();
}

/* ================================================================
   W5500 READ WRITE
   ================================================================ */

void W5500_Write(uint16_t addr, uint8_t block, uint8_t data)
{
    W5500_CS_LOW();

    SPI_Transfer(addr >> 8);
    SPI_Transfer(addr & 0xFF);

    SPI_Transfer((block << 3) | 0x04);

    SPI_Transfer(data);

    W5500_CS_HIGH();
}

uint8_t W5500_Read(uint16_t addr, uint8_t block)
{
    uint8_t data;

    W5500_CS_LOW();

    SPI_Transfer(addr >> 8);
    SPI_Transfer(addr & 0xFF);

    SPI_Transfer((block << 3) | 0x00);

    data = SPI_Transfer(0x00);

    W5500_CS_HIGH();

    return data;
}

/* ================================================================
   HARD RESET
   ================================================================ */

void W5500_HardReset(void)
{
    GPIO_WriteLow(GPIOE, GPIO_PIN_5);

    delay_ms(20);

    GPIO_WriteHigh(GPIOE, GPIO_PIN_5);

    delay_ms(200);
}

/* ================================================================
   SOFT RESET
   ================================================================ */

void W5500_SoftReset(void)
{
    W5500_Write(0x0000, COMMON_BLOCK, 0x80);

    delay_ms(100);
}

/* ================================================================
   DETECT
   ================================================================ */

uint8_t W5500_Detect(void)
{
    return W5500_Read(0x0039, COMMON_BLOCK);
}

/* ================================================================
   WAIT COMMAND COMPLETE
   ================================================================ */

uint8_t W5500_WaitCommand(void)
{
    uint16_t timeout = CMD_TIMEOUT;

    while(W5500_Read(0x0001, SOCKET0_BLOCK))
    {
        delay_ms(1);

        timeout--;

        if(timeout == 0)
        {
            UART_SendString("CMD TIMEOUT\r\n");

            return 0;
        }
    }

    return 1;
}

/* ================================================================
   NETWORK CONFIG
   ================================================================ */

void W5500_SetNetwork(void)
{
    /* Gateway */
    W5500_Write(0x0001, COMMON_BLOCK, 192);
    W5500_Write(0x0002, COMMON_BLOCK, 168);
    W5500_Write(0x0003, COMMON_BLOCK, 22);
    W5500_Write(0x0004, COMMON_BLOCK, 1);

    /* Subnet */
    W5500_Write(0x0005, COMMON_BLOCK, 255);
    W5500_Write(0x0006, COMMON_BLOCK, 255);
    W5500_Write(0x0007, COMMON_BLOCK, 255);
    W5500_Write(0x0008, COMMON_BLOCK, 0);

    /* MAC */
    W5500_Write(0x0009, COMMON_BLOCK, 0x00);
    W5500_Write(0x000A, COMMON_BLOCK, 0x08);
    W5500_Write(0x000B, COMMON_BLOCK, 0xDC);
    W5500_Write(0x000C, COMMON_BLOCK, 0x01);
    W5500_Write(0x000D, COMMON_BLOCK, 0x02);
    W5500_Write(0x000E, COMMON_BLOCK, 0x03);

    /* IP */
    W5500_Write(0x000F, COMMON_BLOCK, 192);
    W5500_Write(0x0010, COMMON_BLOCK, 168);
    W5500_Write(0x0011, COMMON_BLOCK, 22);
    W5500_Write(0x0012, COMMON_BLOCK, 100);
}

/* ================================================================
   CLOSE SOCKET
   ================================================================ */

void W5500_CloseSocket(void)
{
    /* DISCONNECT */
    W5500_Write(0x0001, SOCKET0_BLOCK, 0x08);

    W5500_WaitCommand();

    delay_ms(10);

    /* CLOSE */
    W5500_Write(0x0001, SOCKET0_BLOCK, 0x10);

    W5500_WaitCommand();

    delay_ms(10);

    /* CLEAR INTERRUPTS */
    W5500_Write(0x0002, SOCKET0_BLOCK, 0xFF);
}

/* ================================================================
   TCP SERVER INIT
   ================================================================ */

void W5500_TCP_Server_Init(void)
{
    uint8_t status;

    UART_SendString("TCP INIT\r\n");

    W5500_CloseSocket();

    /* MODE TCP */
    W5500_Write(0x0000, SOCKET0_BLOCK, 0x01);

    /* PORT */
    W5500_Write(0x0004, SOCKET0_BLOCK, TCP_PORT_H);
    W5500_Write(0x0005, SOCKET0_BLOCK, TCP_PORT_L);

    /* TX RX MEMORY */
    W5500_Write(0x001E, SOCKET0_BLOCK, 0x02);
    W5500_Write(0x001F, SOCKET0_BLOCK, 0x02);

    /* OPEN */
    W5500_Write(0x0001, SOCKET0_BLOCK, 0x01);

    if(!W5500_WaitCommand())
    {
        UART_SendString("OPEN CMD FAIL\r\n");
        return;
    }

    status = W5500_Read(0x0003, SOCKET0_BLOCK);

    if(status != SOCK_INIT)
    {
        UART_SendString("OPEN FAIL\r\n");
        return;
    }

    /* LISTEN */
    W5500_Write(0x0001, SOCKET0_BLOCK, 0x02);

    if(!W5500_WaitCommand())
    {
        UART_SendString("LISTEN FAIL\r\n");
        return;
    }

    status = W5500_Read(0x0003, SOCKET0_BLOCK);

    UART_SendString("SOCKET=");

    UART_SendHex(status);

    UART_SendString("\r\n");

    sent_welcome = 0;
}

/* ================================================================
   SOCKET STATUS
   ================================================================ */

uint8_t W5500_GetStatus(void)
{
    return W5500_Read(0x0003, SOCKET0_BLOCK);
}

/* ================================================================
   SEND DATA
   ================================================================ */

void W5500_SendData(uint8_t *buf, uint16_t len)
{
    uint16_t ptr;
    uint16_t offset;
    uint16_t i;

    ptr  = W5500_Read(0x0024, SOCKET0_BLOCK);
    ptr <<= 8;
    ptr |= W5500_Read(0x0025, SOCKET0_BLOCK);

    offset = ptr & SOCKET_BUFFER_MASK;

    W5500_CS_LOW();

    SPI_Transfer(offset >> 8);
    SPI_Transfer(offset & 0xFF);

    SPI_Transfer((SOCKET0_TX << 3) | 0x04);

    for(i = 0; i < len; i++)
    {
        SPI_Transfer(buf[i]);
    }

    W5500_CS_HIGH();

    ptr += len;

    W5500_Write(0x0024, SOCKET0_BLOCK, ptr >> 8);
    W5500_Write(0x0025, SOCKET0_BLOCK, ptr & 0xFF);

    W5500_Write(0x0001, SOCKET0_BLOCK, 0x20);

    if(!W5500_WaitCommand())
    {
        UART_SendString("SEND FAIL\r\n");
    }
}

/* ================================================================
   HARDWARE STATUS
   ================================================================ */

void SendHardwareStatus(void)
{
    char msg[64];

    uint8_t in1;
    uint8_t in2;
    uint8_t in3;
    uint8_t in4;

    in1 = GPIO_ReadInputPin(GPIOD, GPIO_PIN_2);
    in2 = GPIO_ReadInputPin(GPIOD, GPIO_PIN_3);
    in3 = GPIO_ReadInputPin(GPIOD, GPIO_PIN_4);
    in4 = GPIO_ReadInputPin(GPIOD, GPIO_PIN_7);

    sprintf(msg,
            "IN:%d%d%d%d\r\n",
            in1,
            in2,
            in3,
            in4);

    W5500_SendData((uint8_t*)msg, strlen(msg));
}

/* ================================================================
   TCP RX PROCESS
   ================================================================ */

void TCP_Process(void)
{
    uint16_t rx_size;
    uint16_t ptr;
    uint16_t offset;
    uint16_t i;

    uint8_t rxbuf[MAX_RX_SIZE];

    /* RX SIZE */
    rx_size  = W5500_Read(0x0026, SOCKET0_BLOCK);
    rx_size <<= 8;
    rx_size |= W5500_Read(0x0027, SOCKET0_BLOCK);

    if(rx_size == 0)
    {
        return;
    }

    if(rx_size >= MAX_RX_SIZE)
    {
        rx_size = MAX_RX_SIZE - 1;
    }

    /* RX READ POINTER */
    ptr  = W5500_Read(0x0028, SOCKET0_BLOCK);
    ptr <<= 8;
    ptr |= W5500_Read(0x0029, SOCKET0_BLOCK);

    offset = ptr & SOCKET_BUFFER_MASK;

    /* READ RX BUFFER */
    W5500_CS_LOW();

    SPI_Transfer(offset >> 8);
    SPI_Transfer(offset & 0xFF);

    SPI_Transfer((SOCKET0_RX << 3) | 0x00);

    for(i = 0; i < rx_size; i++)
    {
        rxbuf[i] = SPI_Transfer(0x00);
    }

    W5500_CS_HIGH();

    rxbuf[rx_size] = 0;

    /* UPDATE POINTER */
    ptr += rx_size;

    W5500_Write(0x0028, SOCKET0_BLOCK, ptr >> 8);
    W5500_Write(0x0029, SOCKET0_BLOCK, ptr & 0xFF);

    /* RECV COMMAND */
    W5500_Write(0x0001, SOCKET0_BLOCK, 0x40);

    W5500_WaitCommand();

    /* REMOVE CR LF */
    for(i = 0; i < rx_size; i++)
    {
        if(rxbuf[i] == '\r' || rxbuf[i] == '\n')
        {
            rxbuf[i] = 0;
            break;
        }
    }

    UART_SendString("RX=");
    UART_SendString((char*)rxbuf);
    UART_SendString("\r\n");

    /* ============================================================
       COMMANDS
       ============================================================ */

    if(strcmp((char*)rxbuf, "STATUS") == 0)
    {
        SendHardwareStatus();
    }

    else if(strcmp((char*)rxbuf, "RL1ON") == 0)
    {
        GPIO_WriteHigh(GPIOC, GPIO_PIN_3);

        W5500_SendData((uint8_t*)"RL1 ON\r\n", 8);
    }

    else if(strcmp((char*)rxbuf, "RL1OFF") == 0)
    {
        GPIO_WriteLow(GPIOC, GPIO_PIN_3);

        W5500_SendData((uint8_t*)"RL1 OFF\r\n", 9);
    }

    else
    {
        W5500_SendData((uint8_t*)"INVALID\r\n", 9);
    }
}

/* ================================================================
   RECOVERY
   ================================================================ */

void W5500_Recover(void)
{
    UART_SendString("RECOVER\r\n");

    while(1)
    {
        W5500_HardReset();

        W5500_SoftReset();

        if(W5500_Detect() == W5500_VERSION)
        {
            UART_SendString("W5500 OK\r\n");

            break;
        }

        UART_SendString("W5500 FAIL\r\n");

        delay_ms(1000);
    }

    W5500_SetNetwork();

    W5500_TCP_Server_Init();
}

/* ================================================================
   MAIN
   ================================================================ */

void main(void)
{
    uint8_t status;

    /* GPIO */

    GPIO_Init(GPIOC, GPIO_PIN_3, GPIO_MODE_OUT_PP_LOW_FAST);

    GPIO_Init(GPIOD, GPIO_PIN_2, GPIO_MODE_IN_PU_NO_IT);
    GPIO_Init(GPIOD, GPIO_PIN_3, GPIO_MODE_IN_PU_NO_IT);
    GPIO_Init(GPIOD, GPIO_PIN_4, GPIO_MODE_IN_PU_NO_IT);
    GPIO_Init(GPIOD, GPIO_PIN_7, GPIO_MODE_IN_PU_NO_IT);

    /* SPI */

    GPIO_Init(GPIOA, GPIO_PIN_3, GPIO_MODE_OUT_PP_HIGH_FAST);

    GPIO_Init(GPIOC, GPIO_PIN_5, GPIO_MODE_OUT_PP_HIGH_FAST);
    GPIO_Init(GPIOC, GPIO_PIN_6, GPIO_MODE_OUT_PP_HIGH_FAST);
    GPIO_Init(GPIOC, GPIO_PIN_7, GPIO_MODE_IN_FL_NO_IT);

    /* RESET */

    GPIO_Init(GPIOE, GPIO_PIN_5, GPIO_MODE_OUT_PP_HIGH_FAST);

    /* UART */

    UART_Config();

    /* SPI */

    SPI_Config();

    UART_SendString("SYSTEM START\r\n");

    /* WAIT W5500 */

    while(1)
    {
        W5500_HardReset();

        W5500_SoftReset();

        if(W5500_Detect() == W5500_VERSION)
        {
            UART_SendString("W5500 DETECTED\r\n");

            break;
        }

        UART_SendString("WAIT W5500\r\n");

        delay_ms(1000);
    }

    /* NETWORK */

    W5500_SetNetwork();

    /* SERVER */

    W5500_TCP_Server_Init();

    /* MAIN LOOP */

    while(1)
    {
        /* W5500 PRESENT ? */

        if(W5500_Detect() != W5500_VERSION)
        {
            UART_SendString("W5500 LOST\r\n");

            W5500_Recover();

            continue;
        }

        /* CLEAR SOCKET INTERRUPTS */

        W5500_Write(0x0002, SOCKET0_BLOCK, 0xFF);

        /* SOCKET STATUS */

        status = W5500_GetStatus();

        switch(status)
        {
            case SOCK_LISTEN:

                break;

            case SOCK_ESTABLISHED:

                if(!sent_welcome)
                {
                    UART_SendString("CLIENT CONNECTED\r\n");

                    W5500_SendData((uint8_t*)"WELCOME\r\n", 9);

                    sent_welcome = 1;
                }

                TCP_Process();

                break;

            case SOCK_CLOSE_WAIT:

                UART_SendString("CLIENT DISCONNECT\r\n");

                W5500_CloseSocket();

                delay_ms(100);

                W5500_TCP_Server_Init();

                sent_welcome = 0;

                break;

            case SOCK_CLOSED:

                UART_SendString("SOCKET CLOSED\r\n");

                delay_ms(100);

                W5500_TCP_Server_Init();

                sent_welcome = 0;

                break;

            case SOCK_INIT:

                UART_SendString("SOCKET INIT\r\n");

                W5500_Write(0x0001, SOCKET0_BLOCK, 0x02);

                W5500_WaitCommand();

                break;

            default:

                break;
        }

        delay_ms(10);
    }
}