#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <esp_camera.h>
#include <esp_system.h>
#include <esp_event.h>
#include <WiFiClientSecure.h>
#include "certificados.h"
#include <MQTT.h>


unsigned long ultimoEnvioDistancia = 0;
const unsigned long intervaloDistancia = 500; // envia a cada 1 segundo


unsigned long ultimoEnvio = 0;
const long intervalo = 100;
String ultimaMensagem = "";


int estado = 0;
int ult_estado = 0;


// Pinos de comunicação com o Arduino
#define RXD2 47
#define TXD2 21


unsigned long ultimaFoto = 0;
const unsigned long intervaloFoto = 300;


// CONFIG DO PACOTE DE IMAGEM
#define CHUNK_DATA_SIZE 200


#define LED_PIN 2
#define LIGHT_SENSOR_PIN 1


const int PINO_TRIG = 41;
const int PINO_ECHO = 42;


const char *WIFI_SSID = "Projeto";
const char *WIFI_PASS = "2022-11-07";


WiFiClientSecure conexaoSegura;
MQTTClient mqtt(1000);


typedef struct {
  uint16_t packetId;
  uint16_t totalPackets;
  uint16_t dataLen;
  uint32_t frameLen;
  uint8_t data[CHUNK_DATA_SIZE];
} ImagePacket;


// MAC do receptor ESP-NOW
uint8_t receiverMAC[] = { 0x94, 0x51, 0xDC, 0x33, 0xFB, 0x20 };


bool fotoEnviada = false;
bool quero_enviar = false;
camera_fb_t *fb = nullptr;


// CONFIG DA CÂMERA
camera_config_t config = {
  .pin_pwdn = -1, .pin_reset = -1, .pin_xclk = 15, .pin_sscb_sda = 4, .pin_sscb_scl = 5,
  .pin_d7 = 16, .pin_d6 = 17, .pin_d5 = 18, .pin_d4 = 12, .pin_d3 = 10, .pin_d2 = 8, .pin_d1 = 9, .pin_d0 = 11,
  .pin_vsync = 6, .pin_href = 7, .pin_pclk = 13,
  .xclk_freq_hz = 20000000, .ledc_timer = LEDC_TIMER_0, .ledc_channel = LEDC_CHANNEL_0,
  .pixel_format = PIXFORMAT_JPEG, .frame_size = FRAMESIZE_SVGA, .jpeg_quality = 10, .fb_count = 2,
  .grab_mode = CAMERA_GRAB_LATEST
};


void reconectarWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) delay(1000);
    Serial.println("WiFi OK");
  }
}


void reconectarMQTT() {
  if (!mqtt.connected()) {
    while (!mqtt.connected()) {
      mqtt.connect("henrique_hex2", "aula", "zowmad-tavQez");
      delay(1000);
    }
    mqtt.subscribe("fotos");
    mqtt.subscribe("dados_vel_ang");   ///// <---- NOVO!
  }
}

// CALLBACK DO MQTT (PARA VELOCIDADE E ÂNGULO)

void recebeuDadosVelAng(String &topic, String &payload) {
  if (topic != "dados_vel_ang") return;


 
  Serial.println("===== MQTT RECEBIDO =====");
  Serial.print("TOPIC: ");
  Serial.println(topic);
  Serial.print("PAYLOAD: ");
  Serial.println(payload);
  Serial.println("=========================");


  payload.trim();
  float angulo = 0;
  int intensidade = 0;


  int idxA = payload.indexOf("A:");
  int idxI = payload.indexOf("I:");


  if (idxA != -1) angulo = payload.substring(idxA + 2).toFloat();
  if (idxI != -1) intensidade = payload.substring(idxI + 2).toInt();




  String texto = String(angulo,2) +";"+ String(intensidade);
  Serial2.println(texto);


}




void onEspNowReceive(const esp_now_recv_info *info, const uint8_t *data, int len) {
  Serial.print("Recebi ESP-NOW: ");
  Serial.write(data, len);
  Serial.println();
}


void enviarDistanciaMQTT(float distancia) {
  if (!mqtt.connected()) reconectarMQTT();
  char msg[20];
  snprintf(msg, sizeof(msg), "%.2f", distancia);
  mqtt.publish("distancia", msg);
}


void enviarLanternaMQTT(int estado) {
  if (!mqtt.connected()) reconectarMQTT();
  char msg[10];
  snprintf(msg, sizeof(msg), "%d", estado);
  mqtt.publish("lanterna", msg);
}


void setup() {
  Serial.begin(115200);
  Serial.println("Init start");

  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial2 OK");


  WiFi.mode(WIFI_STA);
  Serial.println("WiFi mode OK");

  reconectarWiFi();
  
  conexaoSegura.setCACert(certificado1);


  // MQTT
  mqtt.begin("mqtt.janks.dev.br", 8883, conexaoSegura);
  mqtt.onMessage(recebeuDadosVelAng);   ///// <---- NOVO!
  reconectarMQTT();


  // Inicializa ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ESP-NOW");
    while (true);
  }
  esp_now_register_recv_cb(onEspNowReceive);


  // Inicializa câmera
  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Erro câmera");
    while (true);
  }
  Serial.println("cam e esp now OK");

  pinMode(LED_PIN, OUTPUT);
  pinMode(PINO_TRIG, OUTPUT);
  pinMode(PINO_ECHO, INPUT);


  analogSetAttenuation(ADC_11db);
}


void loop() {
  reconectarMQTT();
  mqtt.loop();


  unsigned long agora = millis();


  // ----- FOTO -----
  if (agora - ultimaFoto >= intervaloFoto) {
    ultimaFoto = agora;
    fotoEnviada = false;
  }


  if (!fotoEnviada) {
    fb = esp_camera_fb_get();
    if (fb) {
      mqtt.publish("fotos", (const char *)fb->buf, fb->len);
      fotoEnviada = true;
      esp_camera_fb_return(fb);
    }
  }


  // ----- SENSOR DE LUZ -----
  int analogValue = analogRead(LIGHT_SENSOR_PIN);
  if (analogValue < 1000) { digitalWrite(LED_PIN, HIGH); estado = 1; }
  else { digitalWrite(LED_PIN, LOW); estado = 0; }


  // ----- SENSOR DE DISTÂNCIA -----
  digitalWrite(PINO_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PINO_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PINO_TRIG, LOW);


  long duracao = pulseIn(PINO_ECHO, HIGH);
  float distancia = (duracao * 0.0343) / 2;


  unsigned long agora_dist = millis();
  if (agora_dist - ultimoEnvioDistancia >= intervaloDistancia) {
    ultimoEnvioDistancia = agora_dist;


    if (estado != ult_estado) enviarLanternaMQTT(estado);
    enviarDistanciaMQTT(distancia);
  }
  ult_estado = estado;
}
