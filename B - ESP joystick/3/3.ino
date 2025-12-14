
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <MQTT.h>
#include "certificados.h"


// ============================
// CONFIG WIFI + MQTT
// ============================
const char* WIFI_SSID = "Projeto";
const char* WIFI_PASS = "2022-11-07";


WiFiClientSecure conexaoSegura;
MQTTClient mqtt(1000);


#define RXD2_PIN 16
#define TXD2_PIN 17


// ============================
// FUNÇÕES DE RECONEXÃO
// ============================
void reconectarWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Conectando ao WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASS);


    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(1000);
    }


    Serial.println(" conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  }
}


void reconectarMQTT() {
  if (!mqtt.connected()) {
    Serial.print("Reconectando MQTT...");


    while (!mqtt.connected()) {
      mqtt.connect("esp_cerebro", "aula", "zowmad-tavQez");
      Serial.print(".");
      delay(1000);
    }


    Serial.println(" conectado!");
    mqtt.subscribe("controle");
  }
}


// ============================
// ENVIO PARA MQTT (novo)
// ============================
void enviarControle(String msg) {
  if (!mqtt.connected()) {
    reconectarMQTT();
  }


  if (mqtt.publish("dados_vel_ang", msg)) {
    Serial.print("[MQTT enviado] ");
    Serial.println(msg);
  } else {
    Serial.println("[ERRO] Falha ao enviar via MQTT");
  }
}




String montarControle(float angulo, float modulo) {
    char buffer[40];
    snprintf(buffer, sizeof(buffer), "(%.2f;%.2f)", angulo, modulo);
    return String(buffer);
}




// ============================
// SETUP
// ============================
void setup() {
  Serial.begin(115200);
  delay(100);


  Serial.println("\n=== ESP-CÉREBRO MQTT ===");


  Serial2.begin(115200, SERIAL_8N1, RXD2_PIN, TXD2_PIN);


  WiFi.mode(WIFI_STA);
  reconectarWiFi();


  conexaoSegura.setCACert(certificado1);
  mqtt.begin("mqtt.janks.dev.br", 8883, conexaoSegura);
  mqtt.setTimeout(5000);


  reconectarMQTT();


  Serial.println("Pronto! Aguardando dados...");
}


// ============================
// LOOP
// ============================
void loop() {
  reconectarWiFi();
  reconectarMQTT();
  mqtt.loop();


  // ========= Recebe comandos do Arduino via Serial2
  if (Serial2.available()) {
    String texto = Serial2.readStringUntil('\n');
    texto.trim();


    if (texto.length() > 0) {
      Serial.print("[Serial2] ");
      Serial.println(texto);


      enviarControle(texto);
    }
  }


  delay(40);
}
