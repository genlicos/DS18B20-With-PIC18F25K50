#include "ds18b20.h"
// --- Variáveis Globais ---
unsigned char sensor1_rom[8];                                                   // Armazena o endereço ROM do primeiro sensor
unsigned char sensor2_rom[8];                                                   // Armazena o endereço ROM do segundo sensor
unsigned char ROM_NO[8];                                                        // Buffer temporário para processo de busca ROM
unsigned char LastDiscrepancy;                                                  // Armazena a última posição de discrepância na busca
unsigned char LastDeviceFlag;                                                   // Flag que indica se é o último dispositivo
unsigned char LastFamilyDiscrepancy;                                            // Armazena discrepância na família de dispositivos
float temperatura1, temperatura2;                                               // Armazena as temperaturas lidas dos sensores

// --- Funções do OneWire ---
unsigned char OneWire_Reset() {                                                 // Função que realiza o reset do barramento OneWire
    unsigned char presenca;                                                     // Variável para armazenar resposta dos dispositivos

    ONE_WIRE_DIR = 0;                                                           // Configura o pino como saída
    ONE_WIRE_PIN = 0;                                                           // Coloca o pino em nível baixo
    Delay_us(480);                                                              // Mantém em nível baixo por 480us
    ONE_WIRE_DIR = 1;                                                           // Configura o pino como entrada
    Delay_us(70);                                                               // Aguarda 70us pela resposta

    presenca = ONE_WIRE_PIN;                                                    // Lê a resposta dos dispositivos
    Delay_us(410);                                                              // Aguarda fim do slot de tempo

    return !presenca;                                                           // Retorna 1 se detectou dispositivo, 0 se não
}

void OneWire_Write_Bit(unsigned char valor_bit) {                               // Função para escrever um bit no barramento
    ONE_WIRE_DIR = 0;                                                           // Configura o pino como saída
    ONE_WIRE_PIN = 0;                                                           // Inicia o slot de tempo

    if(valor_bit) {                                                             // Se o bit a ser enviado é 1
        Delay_us(6);                                                            // Aguarda 6us
        ONE_WIRE_PIN = 1;                                                       // Libera o barramento
        Delay_us(64);                                                           // Aguarda 64us para completar o slot
    } else {                                                                    // Se o bit a ser enviado é 0
        Delay_us(60);                                                           // Mantém nível baixo por 60us
        ONE_WIRE_PIN = 1;                                                       // Libera o barramento
        Delay_us(10);                                                           // Aguarda 10us para completar o slot
    }
}

unsigned char OneWire_Read_Bit() {                                              // Função para ler um bit do barramento
    unsigned char bit_valor;                                                    // Variável para armazenar o bit lido

    ONE_WIRE_DIR = 0;                                                           // Configura o pino como saída
    ONE_WIRE_PIN = 0;                                                           // Inicia o slot de tempo
    Delay_us(6);                                                                // Aguarda 6us
    ONE_WIRE_DIR = 1;                                                           // Configura como entrada para leitura
    Delay_us(9);                                                                // Aguarda 9us antes da leitura
    bit_valor = ONE_WIRE_PIN;                                                   // Lê o valor do bit
    Delay_us(55);                                                               // Aguarda fim do slot

    return bit_valor;                                                           // Retorna o bit lido
}

void OneWire_Write_Byte(unsigned char valor_byte) {                             // Função para escrever um byte no barramento
    unsigned char i;                                                            // Variável de controle do loop
    for(i = 0; i < 8; i++) {                                                    // Para cada bit do byte
        OneWire_Write_Bit(valor_byte & 0x01);                                   // Envia o bit menos significativo
        valor_byte >>= 1;                                                       // Desloca para o próximo bit
    }
}

unsigned char OneWire_Read_Byte() {                                             // Função para ler um byte do barramento
    unsigned char i, valor = 0;                                                 // Variáveis locais
    for(i = 0; i < 8; i++) {                                                    // Para cada bit do byte
        valor >>= 1;                                                            // Desloca os bits já lidos
        if(OneWire_Read_Bit())                                                  // Lê o próximo bit
            valor |= 0x80;                                                      // Se for 1, seta o bit mais significativo
    }
    return valor;                                                               // Retorna o byte lido
}

unsigned char search_ROM(unsigned char search_direction) {                      // Função de busca de dispositivos no barramento
    unsigned char id_bit_number;                                                // Número do bit sendo processado
    unsigned char last_zero, rom_byte_number;                                   // Controles para busca
    unsigned char id_bit, cmp_id_bit;                                           // Bits lidos do dispositivo
    unsigned char rom_byte_mask;                                                // Máscara para processamento do byte
    unsigned char return_value = 0;                                             // Valor de retorno da função

    id_bit_number = 1;                                                          // Inicia com o primeiro bit
    last_zero = 0;                                                              // Reseta última posição zero
    rom_byte_number = 0;                                                        // Inicia com o primeiro byte
    rom_byte_mask = 1;                                                          // Inicia com o primeiro bit

    if (!LastDeviceFlag) {                                                      // Se não for o último dispositivo
        if (!OneWire_Reset()) {                                                 // Realiza reset no barramento
            LastDiscrepancy = 0;                                                // Reseta variáveis de controle
            LastDeviceFlag = 0;
            LastFamilyDiscrepancy = 0;
            return 0;                                                           // Retorna erro se não houver resposta
        }

        OneWire_Write_Byte(SEARCH_ROM);                                         // Envia comando de busca ROM

        do {                                                                    // Loop de busca ROM
            id_bit = OneWire_Read_Bit();                                        // Lê primeiro bit
            cmp_id_bit = OneWire_Read_Bit();                                    // Lê bit complementar

            if ((id_bit == 1) && (cmp_id_bit == 1)) {                           // Se ambos 1, erro na resposta
                break;
            }
            else {                                                              // Processa bits válidos
                if (id_bit != cmp_id_bit)                                       // Se bits diferentes
                    search_direction = id_bit;                                  // Direção definida pelo bit lido
                else {                                                          // Se bits iguais
                    if (id_bit_number < LastDiscrepancy)                        // Se antes da última discrepância
                        search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0); // Usa valor anterior
                    else                                                        // Se após última discrepância
                        search_direction = (id_bit_number == LastDiscrepancy);  // Usa 1 se for a última
                    if (search_direction == 0) {                                // Se direção é 0
                        last_zero = id_bit_number;                              // Salva posição
                        if (last_zero < 9)                                      // Se na primeira família
                            LastFamilyDiscrepancy = last_zero;                  // Salva discrepância
                    }
                }

                if (search_direction == 1)                                      // Salva bit na ROM de acordo
                    ROM_NO[rom_byte_number] |= rom_byte_mask;                   // com a direção escolhida
                else
                    ROM_NO[rom_byte_number] &= ~rom_byte_mask;

                OneWire_Write_Bit(search_direction);                            // Envia bit de direção

                id_bit_number++;                                                // Incrementa contador de bits
                rom_byte_mask <<= 1;                                            // Desloca máscara

                if (rom_byte_mask == 0) {                                       // Se processou byte completo
                    rom_byte_number++;                                          // Passa para próximo byte
                    rom_byte_mask = 1;                                          // Reinicia máscara
                }
            }
        } while(rom_byte_number < 8);                                           // Até processar 8 bytes (64 bits)

        if (!(id_bit_number < 65)) {                                            // Se completou busca
            LastDiscrepancy = last_zero;                                        // Atualiza última discrepância
            if (LastDiscrepancy == 0)                                           // Se não há mais discrepâncias
                LastDeviceFlag = 1;                                             // Marca como último dispositivo
            return_value = 1;                                                   // Retorna sucesso
        }
    }

    if (!return_value || !ROM_NO[0]) {                                          // Se falha ou ROM inválido
        LastDiscrepancy = 0;                                                    // Reseta variáveis de controle
        LastDeviceFlag = 0;
        LastFamilyDiscrepancy = 0;
        return_value = 0;                                                       // Retorna falha
    }

    return return_value;                                                        // Retorna resultado da busca
}

unsigned char Find_Sensors() {                                                  // Função para encontrar sensores no barramento
    unsigned char i;                                                            // Variável para loop
    unsigned char sensores = 0;                                                 // Contador de sensores encontrados

    LastDiscrepancy = 0;                                                        // Reseta variáveis de busca ROM
    LastDeviceFlag = 0;
    LastFamilyDiscrepancy = 0;

    if (search_ROM(0)) {                                                        // Busca primeiro sensor
        for(i = 0; i < 8; i++) {                                                // Copia endereço ROM
            sensor1_rom[i] = ROM_NO[i];                                         // para o buffer do sensor 1
        }
        sensores++;                                                             // Incrementa contador

        if (search_ROM(0)) {                                                    // Busca segundo sensor
            for(i = 0; i < 8; i++) {                                            // Copia endereço ROM
                sensor2_rom[i] = ROM_NO[i];                                     // para o buffer do sensor 2
            }
            sensores++;                                                         // Incrementa contador
        }
    }

    return sensores;                                                            // Retorna quantidade encontrada
}

// ... (mantém código anterior)

float Ler_Temperatura(unsigned char *rom) {                                     // Função para ler temperatura de um sensor
    unsigned char i;                                                            // Variável para loop
    unsigned int temp;                                                          // Temperatura bruta lida

    OneWire_Reset();                                                            // Reset no barramento
    OneWire_Write_Byte(MATCH_ROM);                                              // Seleciona sensor específico

    for(i = 0; i < 8; i++) {                                                    // Envia endereço ROM
        OneWire_Write_Byte(rom[i]);                                             // do sensor desejado
    }

    OneWire_Write_Byte(CONVERT_T);                                              // Inicia conversão

    // Aguarda tempo de conversão conforme resolução configurada
    switch(RESOLUCAO) {                                                         // Seleciona delay conforme resolução
        case 9:                                                                 // Para 9 bits (0.5°C)
            Delay_ms(94);                                                       // 93.75ms
            break;
        case 10:                                                                // Para 10 bits (0.25°C)
            Delay_ms(188);                                                      // 187.5ms
            break;
        case 11:                                                                // Para 11 bits (0.125°C)
            Delay_ms(375);                                                      // 375ms
            break;
        default:                                                                // Para 12 bits (0.0625°C)
            Delay_ms(750);                                                      // 750ms
            break;
    }

    OneWire_Reset();                                                            // Reset no barramento
    OneWire_Write_Byte(MATCH_ROM);                                              // Seleciona sensor novamente

    for(i = 0; i < 8; i++) {                                                    // Envia endereço ROM
        OneWire_Write_Byte(rom[i]);                                             // do sensor novamente
    }

    OneWire_Write_Byte(READ_SCRPAD);                                            // Solicita leitura da memória
    temp = OneWire_Read_Byte();                                                 // Lê byte menos significativo
    temp |= (unsigned int)OneWire_Read_Byte() << 8;                             // Lê byte mais significativo

    return temp * 0.0625;                                                       // Converte e retorna em graus Celsius
}

// Função para configurar resolução dos termômetros
void Configurar_Resolucao() {                                                   // Configura resolução de todos os sensores
    unsigned char config;                                                       // Byte de configuração

    switch(RESOLUCAO) {                                                         // Seleciona configuração conforme resolução
        case 9:                                                                 // Para 9 bits (0.5°C)
            config = 0x1F;                                                      // Configuração: 0001 1111
            break;
        case 10:                                                                // Para 10 bits (0.25°C)
            config = 0x3F;                                                      // Configuração: 0011 1111
            break;
        case 11:                                                                // Para 11 bits (0.125°C)
            config = 0x5F;                                                      // Configuração: 0101 1111
            break;
        default:                                                                // Para 12 bits (0.0625°C)
            config = 0x7F;                                                      // Configuração: 0111 1111
            break;
    }

    OneWire_Reset();                                                            // Reset no barramento
    OneWire_Write_Byte(SKIP_ROM);                                               // Endereça todos os sensores
    OneWire_Write_Byte(WRITE_SCRPAD);                                           // Comando para escrever no scratchpad
    OneWire_Write_Byte(0);                                                      // TH - não utilizado
    OneWire_Write_Byte(0);                                                      // TL - não utilizado
    OneWire_Write_Byte(config);                                                 // Configuração da resolução
}