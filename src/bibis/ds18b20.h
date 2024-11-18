#ifndef DS18B20_DUAL_SENSOR_H
#define DS18B20_DUAL_SENSOR_H

// --- Definições do OneWire ---
#define ONE_WIRE_PIN RA5_bit                                                    // Pino de dados do protocolo OneWire
#define ONE_WIRE_DIR TRISA5_bit                                                 // Registrador de direção do pino OneWire

// --- Definição da Resolução ---
#define RESOLUCAO    9                                                          // Define resolução: 9, 10, 11 ou 12 bits

// --- Comandos do DS18B20 ---
#define SEARCH_ROM   0xF0                                                       // Comando para buscar dispositivos no barramento
#define MATCH_ROM    0x55                                                       // Comando para selecionar um dispositivo específico
#define CONVERT_T    0x44                                                       // Comando para iniciar a conversão de temperatura
#define READ_SCRPAD  0xBE                                                       // Comando para ler a memória do dispositivo
#define SKIP_ROM     0xCC                                                       // Comando para endereçar todos os sensores
#define WRITE_SCRPAD 0x4E                                                       // Comando para escrever no scratchpad

// --- Variáveis Globais ---
extern unsigned char sensor1_rom[];                                                   // Armazena o endereço ROM do primeiro sensor
extern unsigned char sensor2_rom[];                                                   // Armazena o endereço ROM do segundo sensor
extern unsigned char ROM_NO[];                                                        // Buffer temporário para processo de busca ROM
extern unsigned char LastDiscrepancy;                                                  // Armazena a última posição de discrepância na busca
extern unsigned char LastDeviceFlag;                                                   // Flag que indica se é o último dispositivo
extern unsigned char LastFamilyDiscrepancy;                                            // Armazena discrepância na família de dispositivos
extern float temperatura1, temperatura2;                                               // Armazena as temperaturas lidas dos sensores

// Prototipos de funcoes
unsigned char OneWire_Reset(void);                                              // Reset do barramento OneWire
void OneWire_Write_Bit(unsigned char);                                          // Escreve um bit
unsigned char OneWire_Read_Bit(void);                                           // Le um bit
void OneWire_Write_Byte(unsigned char);                                         // Escreve um byte
unsigned char OneWire_Read_Byte(void);                                          // Le um byte
unsigned char search_ROM(unsigned char);                                        // Busca dispositivos
unsigned char Find_Sensors(void);                                               // Encontra sensores
float Ler_Temperatura(unsigned char*);                                          // Le temperatura
void Configurar_Resolucao(void);                                                // Configura resolucao

#endif