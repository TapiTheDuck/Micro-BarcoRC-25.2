

#include <SoftwareSerial.h>


// Pinos da serial por software
const byte rxPin = 2;  // RX do Arduino (ligar no TX do ESP)
const byte txPin = 3;  // TX do Arduino (ligar no RX do ESP)


SoftwareSerial mySerial(rxPin, txPin);


const int pino_motor_1 = 9;
const int pino_motor_2 = 10;


int velocidade_1 = 0;
int velocidade_2 = 0;


float angulo = 90.0;
int intensidade = 0;


unsigned long ultimaLeitura = 0;
const unsigned long tempoLimite = 2000;  // ms


void setup() {
  Serial.begin(115200);
  mySerial.begin(9600);


  pinMode(pino_motor_1, OUTPUT);
  pinMode(pino_motor_2, OUTPUT);


  analogWrite(pino_motor_1, 0);
  analogWrite(pino_motor_2, 0);


  Serial.println("Arduino UNO Iniciado e aguardando dados...");
}


void loop() {


  if (mySerial.available() > 0) {


    String texto = mySerial.readStringUntil('\n');
    texto.trim();


    if (texto.length() == 0) return;


    ultimaLeitura = millis();


    // --- FORMATO ESPERADO: "angulo;intensidade" ex: "93.09;24" ---
    int idxPontoVirgula = texto.indexOf(';');
    if (idxPontoVirgula == -1) {
      // Não tem ';', formato inválido
      Serial.println("ERRO: comando sem ';'");
      return;
    }


    // Parte antes do ';' = ângulo
    String strAng = texto.substring(0, idxPontoVirgula);
    strAng.trim();


    // Parte depois do ';' = intensidade
    String strInt = texto.substring(idxPontoVirgula + 1);
    strInt.trim();


    // Converte para número
    angulo = strAng.toFloat();   // pode vir 93.09
    intensidade = strInt.toInt(); // 0–25


   
    if (intensidade <= 3) {
      intensidade = 0;
    }


    // Mapeia 0–25 para 0–255 PWM
    int pwm = map(intensidade, 0, 25, 0, 255);


    // LÓGICA DE MOVIMENTO
    if (angulo < 50.0) {
      velocidade_1 = 0;
      velocidade_2 = pwm;
    }
    else if (angulo <= 130.0) {  // entre 50 e 130
      velocidade_1 = pwm;
      velocidade_2 = pwm;
    }
    else { // > 130
      velocidade_1 = pwm;
      velocidade_2 = 0;
    }
    // Atualiza motores
    analogWrite(pino_motor_1, velocidade_1);
    analogWrite(pino_motor_2, velocidade_2);
  }



  // TIMEOUT DE SEGURANÇA

  if (millis() - ultimaLeitura > tempoLimite) {


    if (velocidade_1 > 0 || velocidade_2 > 0 || intensidade > 0) {


      velocidade_1 = 0;
      velocidade_2 = 0;
      intensidade = 0;


      analogWrite(pino_motor_1, 0);
      analogWrite(pino_motor_2, 0);


      Serial.println("--- SINAL PERDIDO/PARADO: MOTORES DESLIGADOS ---");
    }
  }
}


