//Brenno Pereira dos Santos - https://github.com/brenno-net
//Huan Guilherme Ribeiro Miranda Di Giorno - [inserir github]
//Lab SisMic - Projeto final: Alarme residencial com avisos por Bluetooth
//Universidade de Brasilia - UnB | data: 17/02/2025

#include <msp430.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

// Definicoes de valores para o codigo
#define DISTANCE_THRESHOLD 60  // cm - distancia de medicao
#define DISTANCE_TOLERANCE 10  // cm - tolerancia para lidar com variacoes do sensor
#define MEASURE_INTERVAL 2048  // ~250 ms
#define MAX_STRING_LENGTH 100  // tamanho maximo da string
//Definicoes para pinos
#define TRIGGER_PIN BIT1       // P6.1 - trigger
#define ECHO_PIN BIT3          // P1.3 - echo
#define LED_PIN BIT0           // P1.0 -  led vermelho
#define BUTTON_PIN BIT1        // P1.1 - botao S2

char received_string[MAX_STRING_LENGTH];
uint16_t string_index = 0;
uint8_t system_armed = 0;      // Flag para indicar se o sistema esta armado
uint8_t alarm_triggered = 0;   // Flag para indicar alarme ativado
uint32_t distance;             // Declaracao da variavel da distancia

uint8_t edge_flag = 0; // 0 = Aguarda flanco de subida, 1 = aguarda flanco de descida
uint16_t start_time = 0; // Guarda o tempo de inicio da captura

void config_alarm(void)
{
    // Config - UART Bluetooth
    P3SEL |= BIT4 | BIT3;      // Pinos para UART (RXD - P3.3 | TXD - P3.4)
    UCA0CTL1 |= UCSSEL_2;      // Seleciona SMCLK
    UCA0BR0 = 6;               // Divisor de baudrate
    UCA0BR1 = 0;               // Divisor de baudrate
    UCA0MCTL = UCBRS_0 + UCBRF_13 + UCOS16; // Config MCTL
    UCA0CTL1 &= ~UCSWRST;      // Inicializa UART
    UCA0IE = UCRXIE;           // Habilita interrupcao para o recebimento

    // Config do Timer
    TA0CCTL2 = CM_3 | CCIS_0 | SCS | CAP | CCIE; // Captura nas bordas do echo
    TA0CCR0 = MEASURE_INTERVAL;
    TA0CCTL0 = CCIE; // Interrupcao de intervalo de medicao
    TA0CTL = TASSEL__ACLK | ID__4 | MC__CONTINUOUS | TACLR; //Config CTL

    // Config Trigger e LED
    P6DIR |= TRIGGER_PIN;
    P6OUT &= ~TRIGGER_PIN;
    P1DIR |= LED_PIN;
    P1OUT &= ~LED_PIN;

    // Config pino Echo
    P1DIR &= ~ECHO_PIN;
    P1SEL |= ECHO_PIN;

    // Config botao S2
    P1DIR &= ~BUTTON_PIN;      // Configura P1.1 para entrada
    P1REN |= BUTTON_PIN;       // Habilita resistor de pull-up/pull-down
    P1OUT |= BUTTON_PIN;       // Define resistor como pull-up
    P1IE |= BUTTON_PIN;        // Habilita interrupcao no botao
    P1IES |= BUTTON_PIN;       // Configura interrupcao quando botao pressionado
    P1IFG &= ~BUTTON_PIN;      // Limpa flag de interrupcao do botao
}

void send_char(unsigned char c)
{
    while (!(UCA0IFG & UCTXIFG)); // Espera o buffer estar pronto
    UCA0TXBUF = c; // Envia o caractere via Bluetooth
}

void send_string(char *str)
{
    uint16_t i;
    for (i = 0; str[i] != '\0'; i++)
        send_char(str[i]);
}

void delay_ms(unsigned int ms) {
    while (ms--) __delay_cycles(16000);  // Aproximadamente 1 ms
}

void triggerMeasurement() {
    P6OUT |= TRIGGER_PIN; // Envia o sinal de Trigger
    __delay_cycles(10); // Espera por 10 ciclos
    P6OUT &= ~TRIGGER_PIN; // Desliga o Trigger
    delay_ms(15);
}

void send_distance(uint32_t distance) {   //funcao para depuracao da distancia
    char distance_str[20];
    sprintf(distance_str, "Distancia: %lu cm\r\n", distance);
    send_string(distance_str); // Envia a distancia pelo Bluetooth
}

void alarmTriggered() {
    if (!alarm_triggered) { // Envia mensagem apenas uma vez
        send_string("ALARME ATIVADO!!! \r\nAlgo foi DETECTADO...\r\n");
        send_distance(distance); // Envia a distancia pelo Bluetooth
        P1OUT |= LED_PIN; // Acende o LED indicando movimento detectado
        alarm_triggered = 1; // Define o alarme como ativado
    }
}

void resetAlarm() {
    alarm_triggered = 0; // Reset o estado do alarme p/ permitir novas mensagens
    P1OUT &= ~LED_PIN;   // Desliga o LED
}

void disarmSystem() {
    system_armed = 0; // Desarma o sistema
    resetAlarm(); // Reseta o alarme
    send_string("Sistema DESARMADO...\r\n");
}

void main(void) {
    WDTCTL = WDTPW + WDTHOLD; // stop watchdog timer

    config_alarm(); // Chama a funcao de configuracao inicial
    __enable_interrupt(); // Habilita interrupcoes

    send_string("\r\n****[SISTEMA DE ALARME INICIADO]**** \r\n-> Insira senha para armar o sistema...\r\n");

    while (1) {
        triggerMeasurement(); // Mede de distancia
        __low_power_mode_3(); // Modo de baixo consumo ate captura

        // Checagem de distancia do alarme
        if (system_armed && distance < (DISTANCE_THRESHOLD - DISTANCE_TOLERANCE)) {
            alarmTriggered(); // ativa o alarme para distancias < 50 cm
        }
        else if (system_armed && distance > DISTANCE_THRESHOLD && alarm_triggered) {
            resetAlarm(); // Reset alarme
        }
    }
}

#pragma vector= USCI_A0_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    unsigned char c = UCA0RXBUF; // Recebe o caractere via Bluetooth

    if (c == '\r' || c == '\n') // Verifica se o caractere de finalizacao
    {
        if (string_index > 0) // Verifica se tem dados no buffer
        {
            received_string[string_index] = '\0'; // Finaliza a string recebida

            send_string("Senha recebida: ");
            send_string(received_string);
            send_string("\r\n");

            if (strcmp(received_string, "1234") == 0) // Verifica a senha
            {
                if (system_armed) // Se ja estiver armado
                {
                    send_string("Senha correta! Sistema DESARMADO...\r\n");
                    system_armed = 0; // Desarma o sistema
                    resetAlarm(); // Reseta o alarme
                }
                else // Se estiver desarmado
                {
                    send_string("Senha correta! Sistema ARMADO...\r\n");
                    system_armed = 1; // Arma o sistema
                }
            }
            else
            {
                send_string("Senha incorreta! Tente novamente...\r\n");
            }

            // Limpa o buffer para a proxima senha
            string_index = 0;
            memset(received_string, 0, MAX_STRING_LENGTH); // Limpa o buffer
        }
    }
    else
    {
        if (string_index < MAX_STRING_LENGTH - 1) // Evita estouro do buffer
        {
            received_string[string_index++] = c; // Armazena char recebido
        }
    }
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
    // Intervalo de medicao
    __low_power_mode_off_on_exit();
    TA0CCR0 += MEASURE_INTERVAL;
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR(void)
{
    if (TA0IV == TA0IV_TACCR2) // Captura em CCR2
    {
        if (edge_flag == 0) // Flanco de subida
        {
            start_time = TA0CCR2; // Armazena o tempo de inicio
            edge_flag = 1; // Aguarda o proximo flanco (descida)
        }
        else if (edge_flag == 1) // Flanco de descida
        {
            uint16_t end_time = TA0CCR2; // Tempo de finalizacao
            distance = end_time - start_time; // Calcula tempo de eco
            distance *= 34000;
            distance = distance / 16384;

            __low_power_mode_off_on_exit(); // Sai do modo de baixo consumo
            edge_flag = 0; //aguardar o flanco de subida novamente
        }
    }
}

#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
    if (P1IFG & BUTTON_PIN) // checa se a interrupcao foi gerada pelo botao S2
    {
        disarmSystem(); // Desarma o sistema
        P1IFG &= ~BUTTON_PIN; // Limpa a flag do botao
    }
}
