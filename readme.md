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


# LoRaWAN Weather Station

This repository hosts the source code and instructions for building a LoRaWAN-based weather station. It aims to collect climatic data like temperature, humidity, wind speed and direction, and rainfall, then sends it to a LoRa gateway. This data is forwarded to an MQTT server for remote weather monitoring.

## Components Used

- Arduino-compatible microcontroller (e.g., ESP32)
- LoRa SX1276 module
- DHT11 temperature and humidity sensor
- Anemometer (wind speed sensor)
- Wind direction sensor
- Rain gauge (rain sensor)
- Ultrasonic sensor for distance measurement

## Required Libraries

To run the code, you will need to install the following libraries in the Arduino development environment:

- `SPI.h` - SPI communication for the LoRa module.
- `lmic.h` - LoRaWAN implementation for Arduino.
- `hal/hal.h` - Hardware abstraction for LMIC.
- `DHT.h` - Data reading from the DHT11 sensor.
- `Adafruit_Sensor.h` - Dependency for the DHT library.

## Hardware Setup

Here are the pin definitions to connect the LoRa module and sensors to the microcontroller:

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
const int DHTPIN = 16; // Pin for the DHT11 sensor
const int trigPin = 4;  // Trigger pin for the ultrasonic sensor
const int echoPin = 2;  // Echo pin for the ultrasonic sensor
```

## Software Setup

### Initialization
In the setup() function, we perform the initial setup of the microcontroller, sensors, and LoRaWAN communication. This includes starting serial communication for debugging, configuring the sensor pins as input or output as needed, and initializing SPI communication with the LoRa module.

### Main Loop
The loop() function keeps the microcontroller processing LMIC events, managing LoRaWAN communication. This ensures that the device responds to LoRaWAN network commands, such as join requests, data receipt confirmations, and data sending scheduling.

### Sensor Measurement and Data Sending
Data is collected through specific functions for each type of measurement:

- `temperatura()`: Returns the current temperature measured by the DHT11 sensor.
- `umidade()`: Returns the relative humidity of the air, measured by the same DHT11 sensor.
- `velocidade()` Calculates the wind speed based on pulses received from the anemometer.
- `chuva()`: Calculates the amount of rain detected by the rain gauge, also based on pulses.
- `distancia()`: Uses the ultrasonic sensor to measure distance, which can be used to detect water level or another relevant parameter.
This data is sent periodically via LoRa, alternating between sending climatic data and water blade data. The `do_send()` function is responsible for preparing the data for sending and scheduling the next transmission with LMIC.

### Objective
This project allows for real-time monitoring of weather conditions remotely, being a valuable tool for areas such as precision agriculture, environmental studies, and climate monitoring in remote areas. Through the use of LoRaWAN technology, it is possible to achieve wide area coverage with low energy consumption, ideal for installations in remote or difficult-to-access locations.


### Spanish Version

La traducción al español superará el límite de 350 caracteres si se incluyen todas las secciones detalladas anteriormente. En su lugar, aquí hay una descripción general simplificada que se ajusta a la limitación de caracteres:

# Estación Meteorológica con LoRaWAN

Este repositorio alberga el código fuente e instrucciones para construir una estación meteorológica basada en LoRaWAN, diseñada para recoger datos climáticos como temperatura, humedad, velocidad y dirección del viento, y precipitaciones, para enviarlos a un gateway LoRa. Posteriormente, estos datos se transmiten a un servidor MQTT, permitiendo el monitoreo remoto de las condiciones meteorológicas.

## Componentes Usados

- Microcontrolador compatible con Arduino (ej., ESP32)
- Módulo LoRa SX1276
- Sensor DHT11 de temperatura y humedad
- Anemómetro (sensor de velocidad del viento)
- Sensor de dirección del viento
- Pluviómetro (sensor de lluvia)
- Sensor ultrasónico para medición de distancia

## Librerías Necesarias

Para ejecutar el código, necesitarás instalar las siguientes librerías en el entorno de desarrollo de Arduino:

- `SPI.h` - Comunicación SPI para el módulo LoRa.
- `lmic.h` - Implementación de LoRaWAN para Arduino.
- `hal/hal.h` - Abstracción de hardware para LMIC.
- `DHT.h` - Lectura de datos del sensor DHT11.
- `Adafruit_Sensor.h` - Dependencia para la librería DHT.

## Configuración del Hardware

Definiciones de pinado para conectar el módulo LoRa y los sensores al microcontrolador:

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
const int DHTPIN = 16; // Pin para el sensor DHT11
const int trigPin = 4;  // Pin de disparo para el sensor ultrasónico
const int echoPin = 2;  // Pin de eco para el sensor ultrasónico
```

## Configuración del Software

### Inicialización

En la función setup(), realizamos la configuración inicial del microcontrolador, los sensores y la comunicación LoRaWAN.

### Bucle Principal

La función loop() mantiene el microcontrolador procesando los eventos de LMIC, que gestiona la comunicación LoRaWAN.

###  Medición de Sensores y Envío de Datos

Los datos se recopilan a través de funciones específicas para cada tipo de medición:

temperatura(): Devuelve la temperatura actual medida por el sensor DHT11.
umidade(): Devuelve la humedad relativa del aire, medida por el mismo sensor DHT11.
velocidade(): Calcula la velocidad del viento basada en los pulsos recibidos del anemómetro.
chuva(): Calcula la cantidad de lluvia detectada por el pluviómetro, también basado en pulsos.
distancia(): Utiliza el sensor ultrasónico para medir la distancia, que puede ser usado para detectar el nivel del agua u otro parámetro relevante.
Estos datos se envían periódicamente vía LoRa, alternando entre el envío de datos climáticos y datos de lámina de agua. La función do_send() es responsable de preparar los datos para el envío y programar la próxima transmisión con LMIC.

###  Objetivo

Este proyecto permite el monitoreo en tiempo real de las condiciones meteorológicas de forma remota, siendo una herramienta valiosa para la agricultura de precisión, estudios ambientales y el monitoreo del clima en áreas remotas.