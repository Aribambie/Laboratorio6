//*****************************************************************************
// Universidad del Valle de Guatemala
// Programaci�n de Microcontroladores
// Archivo: Laboratorio_y_postlab_06 
// Hardware: ATMEGA328P
// Autor: Adriana Marcela Gonzalez
// Carnet: 22438
//*****************************************************************************

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// Declaraciones de funciones
void initUART9600(void);
void writeTextUART(char* texto);
void Menu(void);

// Variable global para almacenar el �ltimo car�cter recibido por UART
volatile char bufferRX;
// Variable global para almacenar el valor del potenci�metro ADC
unsigned char ADCPot = 0;

// Funci�n para inicializar UART a 9600 baudios
void initUART9600(void){
    // Configuraci�n de RX y TX
    DDRD &= ~(1<<DDD0); // RX como entrada
    DDRD |= (1<<DDD1); // TX como salida
    
    // Configuraci�n del modo fast
    UCSR0A = 0;
    UCSR0A |= (1<<U2X0); // Modo de doble velocidad
    
    // Configuraci�n del registro B
    UCSR0B = 0;
    UCSR0B |= (1<<RXCIE0)|(1 << RXEN0)|(1 << TXEN0); // Habilitar RX, TX e interrupci�n de RX
    
    // Configuraci�n del registro C: frame - 8 bits de datos, no paridad, 1 bit de stop
    UCSR0C = 0;
    UCSR0C |= (1 << UCSZ01)|(1 << UCSZ00);
    
    // Baudrate = 9600
    UBRR0 = 207; // F�rmula para 9600 baudios con F_CPU = 16MHz
}

// Funci�n para inicializar ADC
void initADC(void){
    ADMUX = 0;
    ADMUX |= (1<<REFS0); // Referencia de voltaje AVCC con ajuste externo
    ADMUX &= ~(1<<REFS1);
    ADMUX |= (1<<ADLAR); // Ajuste de resultado a la izquierda
    
    ADCSRA = 0;
    ADCSRA |= (1<<ADEN); // Habilitar ADC
    ADCSRA |= (ADPS2)|(ADPS1)|(ADPS0); // Prescaler de 128 -> Frecuencia de muestreo de 125kHz
    
    DIDR0 |= (1<<ADC5D); // Deshabilitar funci�n digital del pin ADC5
}

// Funci�n para leer el valor del ADC
uint16_t valorADC (uint8_t admux_adc) {
    ADMUX &= 0xF0; // Limpiar bits de canal
    ADMUX |= admux_adc; // Seleccionar canal
    
    ADCSRA |= (1 << ADSC); // Iniciar conversi�n
    
    while (ADCSRA & (1 << ADSC)); // Esperar a que la conversi�n termine
    
    return ADCH; // Devolver valor del ADC
}

// Funci�n para transmitir un car�cter por UART
void transUART (unsigned char valorT) {
    while (!(UCSR0A & (1 << UDRE0))); // Esperar a que el buffer de transmisi�n est� vac�o
    UDR0 = valorT; // Transmitir car�cter
}

// Funci�n para recibir un car�cter por UART
unsigned char recivUART(void) {
    return bufferRX; // Devuelve el �ltimo car�cter recibido
}

// Funci�n principal
int main(void) {
    initADC(); // Inicializar ADC
    initUART9600(); // Inicializar UART
    sei(); // Habilitar interrupciones globales
    DDRB = 0xFF; // Configurar puerto B como salida
    DDRC &= ~(1 << DDC5); // Configurar PC5 (ADC5) como entrada (potenci�metro)
    PORTC |= (1 << PORTC5); // Habilitar resistencia pull-up interna en PC5
    Menu(); // Mostrar men� inicial
    
    while (1) {
        if ( bufferRX == '1') { // Si se selecciona la opci�n 1
            uint16_t ADCPot = valorADC(5); // Leer valor del potenci�metro
            char buffer[10]; // Buffer para convertir valor del ADC a cadena
            writeTextUART("Valor del potenciometro: "); // Mostrar mensaje
            sprintf(buffer, "%d", ADCPot); // Convertir valor a cadena
            writeTextUART(buffer); // Mostrar valor del potenci�metro
            bufferRX = 0; // Reiniciar buffer de recepci�n
            _delay_ms(1000); // Esperar 1 segundo
            Menu(); // Mostrar men� nuevamente
        }
        else if (bufferRX == '2') { // Si se selecciona la opci�n 2
            char received_char = recivUART(); // Leer car�cter recibido por UART
            PORTB = received_char; // Mostrar car�cter en el puerto B
            _delay_ms(1000); // Esperar 1 segundo
            Menu(); // Mostrar men� nuevamente
        }
    }
    return 0; // Retorno est�ndar de finalizaci�n
}

// Funci�n para mostrar el men� en la UART
void Menu(void){
    writeTextUART("*** Seleccione una opcion: ***\n");
    writeTextUART("    1. Leer potenciometro \n ");
    writeTextUART("    2. Enviar ASCII \n ");
}

// Funci�n para transmitir una cadena de texto por UART
void writeTextUART(char* texto){
    uint8_t i;
    for(i=0; texto[i]!='\n'; i++){ // Iterar hasta encontrar el car�cter de nueva l�nea
        while(!(UCSR0A & (1<<UDRE0))); // Esperar a que el buffer de transmisi�n est� vac�o
        UDR0 = texto[i]; // Transmitir car�cter por car�cter
    }
}

// Rutina de interrupci�n para manejar la recepci�n de datos por UART
ISR(USART_RX_vect){
    bufferRX = UDR0; // Almacenar car�cter recibido en el buffer
    while(!(UCSR0A & (1<<UDRE0))); // Esperar a que el buffer de transmisi�n est� vac�o
    UDR0 = bufferRX; // Transmitir el car�cter recibido de vuelta
}
