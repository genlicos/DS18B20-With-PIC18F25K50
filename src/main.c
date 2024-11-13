#include "bibis\ds18b20.h"
#include "bibis\lcd_i2c.h"

// --- Variáveis Globais ---
char txt[17];                                                                   // Buffer para mensagens do LCD (16 caracteres + null)

void main() {                                                                   // Função principal
    unsigned char sensores;                                                     // Quantidade de sensores encontrados

    ANSELA = 0;                                                                 // Configura PORTA como digital
    ANSELB = 0;                                                                 // Configura PORTB como digital
    ONE_WIRE_DIR = 1;                                                           // Configura pino OneWire como entrada

    I2C1_Init(100000);                                                          //Inicializa I2C
    I2C_LCD_Init();                                                             //Inicializa Display LCD I2C


    I2C_Lcd_Out(1, 1, "Procurando");                                            // Mensagem inicial
    I2C_Lcd_Out(2, 1, "Sensores...");                                           // de busca

    sensores = Find_Sensors();                                                  // Busca sensores no barramento

    if (sensores == 0) {                                                        // Se não encontrou nenhum sensor
        I2C_Lcd_Cmd(_LCD_CLEAR);                                                // Limpa LCD
        I2C_Lcd_Out(1, 1, "ERRO: Nenhum");                                      // Mostra mensagem
        I2C_Lcd_Out(2, 1, "Sensor!");                                           // de erro
        while(1);                                                               // Trava programa
    }

    Configurar_Resolucao();                                                     // Configura resolução dos sensores

    if (sensores == 1) {                                                        // Se encontrou apenas um sensor
        I2C_Lcd_Cmd(_LCD_CLEAR);                                                // Limpa LCD
        I2C_Lcd_Out(1, 1, "Apenas 1 Sensor");                                   // Mostra mensagem
        I2C_Lcd_Out(2, 1, "Encontrado!");                                       // de aviso
        Delay_ms(1000);                                                         // Aguarda 1 segundo
    }

    while(1) {                                                                  // Loop principal
        temperatura1 = sensores >= 1 ? Ler_Temperatura(sensor1_rom) : 0;        // Lê sensor 1 se existir
        temperatura2 = sensores == 2 ? Ler_Temperatura(sensor2_rom) : 0;        // Lê sensor 2 se existir

        I2C_Lcd_Cmd(_LCD_CLEAR);                                                // Limpa LCD

        sprintf(txt, "S1:%3.1f", temperatura1);                                 // Formata temperatura 1
        I2C_Lcd_Out(1, 1, txt);                                                 // Mostra no LCD
        I2C_Lcd_Chr_CP(223);                                                    // Símbolo de grau
        I2C_Lcd_Chr_CP('C');                                                    // Celsius

        sprintf(txt, "S2:%3.1f", temperatura2);                                 // Formata temperatura 2
        I2C_Lcd_Out(2, 1, txt);                                                 // Mostra no LCD
        I2C_Lcd_Chr_CP(223);                                                    // Símbolo de grau
        I2C_Lcd_Chr_CP('C');                                                    // Celsius

        Delay_ms(1000);                                                         // Aguarda 1 segundo
    }
}