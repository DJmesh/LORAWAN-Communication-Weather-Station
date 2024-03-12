#include <Arduino.h>
#include "lmic.h"
#include "hal/hal.h"
#include "SPI.h"
#include "DHT.h"
#include "Adafruit_Sensor.h"

/* Definições do rádio LoRa (SX1276) */
#define GANHO_LORA_DBM          20 //dBm

#define RADIO_RESET_PORT 14
#define RADIO_MOSI_PORT 27
#define RADIO_MISO_PORT 19
#define RADIO_SCLK_PORT 5
#define RADIO_NSS_PORT 18     /* CS*/
#define RADIO_DIO0_PORT 26
#define RADIO_DIO1_PORT 33  // 35 v2
#define RADIO_DIO2_PORT 32  // 34 v2

// Definições dos pinos
const int windSpeedPin = 12;
const int windDirectionPin = 36;
const int rainSensorPin = 17;
const int DHTPIN = 16; // Pino para o sensor DHT11
const int trigPin = 4;  // Pino de trigger do sensor ultrassônico
const int echoPin = 2;  // Pino de echo do sensor ultrassônico
bool trocaDados = false;

// Definição do tipo de sensor DHT
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Variáveis globais
volatile int windPulseCount = 0;
volatile int rainPulseCount = 0;
unsigned long lastMillis = 0;
float windPulseToKmhConversionFactor = 0.4; // Conversão de pulso para km/h para o vento
float rainPulseToMmConversionFactor = 0.2; // Conversão de pulso para mm de chuva

/* Constantes do LMIC */
const lmic_pinmap lmic_pins = {
    .nss = RADIO_NSS_PORT,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = RADIO_RESET_PORT,
    .dio = {RADIO_DIO0_PORT, RADIO_DIO1_PORT, RADIO_DIO2_PORT},
};

/* Constantes do LoraWAN */
/* - Chaves (network e application keys) */
static const PROGMEM u1_t NWKSKEY[16] = {}; // Coloque em hexadecimal o NWKSKEY
static const PROGMEM u1_t APPSKEY[16] = {}; // Coloque em hexadecimal o APPSKEY

/* - Device Address */
static const u4_t DEVADDR = 0; // Coloque o seu DEVADDR

/* - Tempo entre envios de pacotes LoRa */
const unsigned TX_INTERVAL = 1; //1800s = 30 minutos 
static osjob_t sendjob;

void os_getArtEui (u1_t* buf) {}
void os_getDevEui (u1_t* buf) {}
void os_getDevKey (u1_t* buf) {}
void dataProcessing_clima(char **p_data, int *tamanho);
void dataProcessing_lamina(char **p_data, int *tamanho);
void displayInfo(char *p_dados);
void onEvent (ev_t ev);
void do_send(osjob_t* j);
void resetLMIC();
void checkAndTriggerResetIfNeeded();
int distancia(void);
float temperatura(void);
float umidade(void);
int chuva(void);
float velocidade(void);
int getWindDirection(float voltage);
int windDirection();
void checkAndTriggerResetIfNeeded();
void resetLMIC();

// Variáveis para controlar tentativas de envio
static int tentativasEnvio = 0;
static const int MAX_TENTATIVAS = 3; // Máximo de tentativas antes de resetar

void IRAM_ATTR windSpeedISR() {
  windPulseCount++;
}

void IRAM_ATTR rainSensorISR() {
  rainPulseCount++;
}

int getWindDirection(float voltage) {
    struct WindDirection {
        int code;
        float voltage;
    };

    WindDirection directions[] = {
        {1, 3.208},   // Norte
        {5, 2.349},   // Nordeste
        {2, 1.665},   // Leste
        {7, 2.283},   // Sudeste
        {3, 2.637},   // Sul
        {8, 1.928},   // Sudoeste
        {4, 0.891},   // Oeste
        {6, 1.619}    // Noroeste
    };

    float minDifference = 3.3; // Valor alto inicial
    int nearestDirection = -1; // Valor padrão para direção desconhecida

    for (WindDirection direction : directions) {
        float difference = abs(voltage - direction.voltage);
        if (difference < minDifference) {
            minDifference = difference;
            nearestDirection = direction.code;
        }
    }

    return nearestDirection;
}

int measureDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2;
  return distance;
}

void setup() {
  Serial.begin(115200);
  pinMode(windSpeedPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(windSpeedPin), windSpeedISR, RISING);

  pinMode(rainSensorPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(rainSensorPin), rainSensorISR, FALLING);

  // Iniciando o sensor DHT11
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  dht.begin();
 
  uint8_t appskey[sizeof(APPSKEY)];
  uint8_t nwkskey[sizeof(NWKSKEY)];
  int b;

  SPI.begin(RADIO_SCLK_PORT, RADIO_MISO_PORT, RADIO_MOSI_PORT);

  os_init();
  LMIC_reset();

  memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
  memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
  LMIC_setSession (0x13, DEVADDR, nwkskey, appskey);

  for (b = 0; b < 8; ++b) {
    LMIC_disableSubBand(b);
  }

  LMIC_enableChannel(0); // 915.2 MHz
  LMIC_enableChannel(1); // 915.4 MHz
  LMIC_enableChannel(2); // 915.6 MHz
  LMIC_enableChannel(3); // 915.8 MHz
  LMIC_enableChannel(4); // 916.0 MHz
  LMIC_enableChannel(5); // 916.2 MHz
  LMIC_enableChannel(6); // 916.4 MHz
  LMIC_enableChannel(7); // 916.6 MHz

  LMIC_setAdrMode(0);
  LMIC_setLinkCheckMode(0);
  LMIC.dn2Dr = DR_SF12CR;
  LMIC_setDrTxpow(DR_SF12, GANHO_LORA_DBM);
  
  do_send(&sendjob);
}

void loop() {
  os_runloop_once();
}

int chuva(void) {
  float rainAmount = (float)rainPulseCount * rainPulseToMmConversionFactor;
  rainPulseCount = 0; // Reseta o contador depois de calcular a quantidade de chuva
  return (int)rainAmount; // Note que isso ainda trunca qualquer parte fracionária
}


float velocidade(void){
  float windSpeed = (float)windPulseCount * windPulseToKmhConversionFactor;
  windPulseCount = 0;
  return windSpeed;
}

float temperatura(void)
{

  float temperatura;
  for(int i = 0; i <100; i++)
    temperatura +=  dht.readTemperature();
  return temperatura/100;
}

float umidade(void)
{

  float umidade;
  for(int i = 0; i <100; i++)
    umidade += dht.readHumidity();
  return umidade/100;
}
// Função distância 
int distancia(void)
{
  // Clear the trigPin condition
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  // Set the trigPin HIGH for 10 microseconds to send the ultrasonic pulse
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measure the duration of the echo pulse
  long duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance based on the speed of sound (340 m/s or 0.034 cm/µs)
  // and divide by 2 because the sound wave travels to the object and back
  long distance = duration * 0.034 / 2;
  unsigned int distSoma = 0;

  for(int i = 0; i <100; i++)
  {
    distSoma  += distance;
  }
  distance = distSoma/100;

  return distance;
}

int windDirection() {
    int windDirectionValue = analogRead(windDirectionPin);
    float voltage = (windDirectionValue * 3.3) / 4095.0;
    return getWindDirection(voltage);
}

void checkAndTriggerResetIfNeeded() {
    if (tentativasEnvio >= MAX_TENTATIVAS) {
        Serial.println(F("Máximo de tentativas atingido. Resetando..."));
        resetLMIC(); // Ou ESP.restart(); para resetar o microcontrolador
    }
}

void resetLMIC() {
    LMIC_reset();
    // Reinicialize o estado do LMIC aqui, se necessário
    do_send(nullptr); // Reiniciar processo de envio
}

void displayInfo(char *p_dados) {
    Serial.println(p_dados);
}



void onEvent(ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");

    // Primeiro, lidaremos com os casos que compartilham uma ação comum
    switch (ev) {
        case EV_SCAN_TIMEOUT:     Serial.println(F("EV_SCAN_TIMEOUT: Tempo limite da varredura atingido")); break;
        case EV_BEACON_FOUND:     Serial.println(F("EV_BEACON_FOUND: Beacon encontrado")); break;
        case EV_BEACON_MISSED:    Serial.println(F("EV_BEACON_MISSED: Beacon perdido")); break;
        case EV_BEACON_TRACKED:   Serial.println(F("EV_BEACON_TRACKED: Beacon rastreado")); break;
        case EV_JOINING:          Serial.println(F("EV_JOINING: Tentativa de join em andamento")); break;
        case EV_JOINED:           Serial.println(F("EV_JOINED: Dispositivo juntou-se à rede")); break;
        case EV_JOIN_FAILED:      Serial.println(F("EV_JOIN_FAILED: Falha ao juntar-se à rede")); tentativasEnvio++; break;
        case EV_REJOIN_FAILED:    Serial.println(F("EV_REJOIN_FAILED: Falha ao tentar re-juntar-se")); tentativasEnvio++; break;
        case EV_LOST_TSYNC:       Serial.println(F("EV_LOST_TSYNC: Sincronização perdida")); tentativasEnvio++; break;
        case EV_RESET:            Serial.println(F("EV_RESET: Dispositivo resetado")); break;
        case EV_RXCOMPLETE:       Serial.println(F("EV_RXCOMPLETE: Recebimento completo")); break;
        case EV_LINK_DEAD:        Serial.println(F("EV_LINK_DEAD: Link considerado morto")); break;
        case EV_LINK_ALIVE:       Serial.println(F("EV_LINK_ALIVE: Link está ativo")); break;
        case EV_TXSTART:
            Serial.println(F("EV_TXSTART: Início da transmissão"));
            Serial.println(millis());
            Serial.print(F("Frequência: "));
            Serial.println(LMIC.freq);
            break;
        case EV_TXCOMPLETE:
            Serial.println(millis());
            Serial.println(F("EV_TXCOMPLETE: Transmissão completa (incluindo espera pelas janelas de recepção)"));
            tentativasEnvio = 0; // Resetar tentativas após envio completo
            if (LMIC.txrxFlags & TXRX_ACK) Serial.println(F("Ack recebido"));
            if (LMIC.dataLen) {
                Serial.print(F("Recebidos "));
                Serial.print(LMIC.dataLen);
                Serial.println(F(" bytes (payload) do gateway"));
            }
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            trocaDados = !trocaDados; // Alternar entre os dados a serem enviados
            break;
        default:
            Serial.print(F("Evento desconhecido: "));
            Serial.println((unsigned) ev);
            break;
    }

    // Agora, verifique se precisamos redefinir com base nas tentativas de envio
    if (ev == EV_LOST_TSYNC || ev == EV_JOIN_FAILED || ev == EV_REJOIN_FAILED) {
        checkAndTriggerResetIfNeeded();
    }
}


void do_send(osjob_t* j)
{
    static bool enviarClima = true; // Alternar entre clima e lâmina
    char *dados = NULL;
    int tamanho = 0;
    tentativasEnvio = 0;

    // Decidir qual conjunto de dados processar e enviar
    if (enviarClima) {
        dataProcessing_clima(&dados, &tamanho);
    } else {
        dataProcessing_lamina(&dados, &tamanho);
    }
    // A próxima vez que essa função for chamada, ela alternará o tipo de dados
    enviarClima = !enviarClima;

    if (dados != NULL && tamanho > 0) {
        displayInfo(dados); // Mostrar os dados para fins de depuração

        if (!(LMIC.opmode & OP_TXRXPEND)) {
            LMIC_setTxData2(4, (uint8_t *)dados, tamanho, 0);
            Serial.println(F("Pacote na fila de envio."));
        } else {
            Serial.println(F("OP_TXRXPEND, envio pendente"));
        }

        free(dados); // Limpar depois do envio
    } else {
        Serial.println(F("Erro: Dados não preparados para envio."));
    }
}


void dataProcessing_clima(char **p_data, int *tamanho) {
  int chu;
  float temp, umi;
  
  temp = temperatura();
  umi = umidade();
  chu = chuva();

  // Cálculo do tamanho da string e alocação de memória
  *tamanho = snprintf(NULL, 0, "{\"umi\":%.1f,\"temp\":%.1f,\"chu\":%d}", umi, temp, chu);
  *p_data = (char*)malloc((*tamanho + 1) * sizeof(char));

  if (*p_data != NULL) {
    // Formatação da string com os dados dos sensores
    snprintf(*p_data, *tamanho + 1, "{\"umi\":%.1f,\"temp\":%.1f,\"chu\":%d}", umi, temp, chu);
    Serial.println(*p_data);
  } else {
    Serial.println("Erro ao alocar memoria");
    delay(1000);
    ESP.restart();
  }
}

void dataProcessing_lamina(char **p_data, int *tamanho) {
  int distance, directionCode;
  float velo;

  // Medição dos valores dos sensores
  distance = distancia();
  velo = velocidade();
  directionCode = windDirection();

  // Cálculo do tamanho da string e alocação de memória
  *tamanho = snprintf(NULL, 0, "{\"dist\":%d,\"velo\":%.1f,\"dir\":%d}", distance, velo, directionCode); // Corrigido para %d em dir
  *p_data = (char*)malloc((*tamanho + 1) * sizeof(char));

  if (*p_data != NULL) {
    // Formatação da string com os dados dos sensores
    snprintf(*p_data, *tamanho + 1, "{\"dist\":%d,\"velo\":%.1f,\"dir\":%d}", distance, velo, directionCode); // Corrigido para %d em dir
    Serial.println(*p_data);
  } else {
    Serial.println("Erro ao alocar memoria");
    delay(1000);
    ESP.restart();
  }
}
