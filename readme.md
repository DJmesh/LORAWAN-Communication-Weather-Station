# Estação Meteorológica LoRaWAN

Este repositório contém o código-fonte e as instruções para montar uma estação meteorológica baseada em LoRaWAN. O objetivo é coletar dados climáticos, como temperatura, umidade, velocidade e direção do vento, e quantidade de chuva, e enviá-los para um gateway LoRa. Em seguida, esses dados são transmitidos para um servidor MQTT, permitindo o monitoramento remoto das condições meteorológicas.

## Componentes Utilizados

- Microcontrolador compatível com Arduino (por exemplo, ESP32)
- Módulo LoRa SX1276
- Sensor de temperatura e umidade DHT11
- Sensor de velocidade do vento (anemômetro)
- Sensor de direção do vento
- Pluviômetro (sensor de chuva)
- Sensor ultrassônico para medição de distância

## Bibliotecas Necessárias

Para executar o código, você precisará instalar as seguintes bibliotecas no ambiente de desenvolvimento Arduino:

- `SPI.h` - Comunicação SPI para módulo LoRa.
- `lmic.h` - Implementação LoRaWAN para Arduino.
- `hal/hal.h` - Abstração de hardware para o LMIC.
- `DHT.h` - Leitura de dados do sensor DHT11.
- `Adafruit_Sensor.h` - Dependência da biblioteca DHT.

## Configuração do Hardware

Aqui estão as definições de pinagem para conectar o módulo LoRa e os sensores ao microcontrolador:

```cpp
#define GANHO_LORA_DBM          20 // dBm
#define RADIO_RESET_PORT 14
#define RADIO_MOSI_PORT 27
#define RADIO_MISO_PORT 19
#define RADIO_SCLK_PORT 5
#define RADIO_NSS_PORT 18     /* CS */
#define RADIO_DIO0_PORT 26
#define RADIO_DIO1_PORT 33
#define RADIO_DIO2_PORT 32

const int windSpeedPin = 12;
const int windDirectionPin = 36;
const int rainSensorPin = 17;
const int DHTPIN = 16; // Pino para o sensor DHT11
const int trigPin = 4;  // Pino de trigger do sensor ultrassônico
const int echoPin = 2;  // Pino de echo do sensor ultrassônico
```
## Configuração do Software

### Inicialização

Na função `setup()`, realizamos a configuração inicial do microcontrolador, dos sensores e da comunicação LoRaWAN. Isso inclui iniciar a comunicação serial para debug, configurar os pinos utilizados pelos sensores como entrada ou saída conforme necessário, e inicializar a comunicação SPI com o módulo LoRa.

### Loop Principal

A função `loop()` mantém o microcontrolador processando os eventos do LMIC, que gerencia a comunicação LoRaWAN. Isso assegura que o dispositivo responda aos comandos da rede LoRaWAN, como solicitações de join, confirmações de recebimento de dados e agendamento de envio de dados.

### Medição dos Sensores e Envio de Dados

Os dados são coletados através de funções específicas para cada tipo de medição:

- `temperatura()`: Retorna a temperatura atual medida pelo sensor DHT11.
- `umidade()`: Retorna a umidade relativa do ar, medida pelo mesmo sensor DHT11.
- `velocidade()`: Calcula a velocidade do vento com base nos pulsos recebidos do anemômetro.
- `chuva()`: Calcula a quantidade de chuva detectada pelo pluviômetro, também baseado em pulsos.
- `distancia()`: Utiliza o sensor ultrassônico para medir a distância, que pode ser usada para detectar o nível da água ou outro parâmetro relevante.

Esses dados são enviados periodicamente via LoRa, alternando entre o envio de dados climáticos e dados de lâmina d'água. A função `do_send()` é responsável por preparar os dados para envio e agendar a próxima transmissão com o LMIC.

## Objetivo

Este projeto permite o monitoramento das condições meteorológicas em tempo real de forma remota, sendo uma ferramenta valiosa para áreas como agricultura de precisão, estudos ambientais e monitoramento climático em áreas remotas. Através da utilização da tecnologia LoRaWAN, é possível obter uma ampla cobertura de área com baixo consumo de energia, ideal para instalações em locais remotos ou de difícil acesso.
