#include <GFButton.h>
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>

#define FUNDO TFT_BLACK
const int CX = 50; // NOVO CENTRO X (170 + 5)
const int CY = 300; // Centro Y
const int R_EXT = 50; 
const int R_INT = 30; 
const int R_EXT_45 = 35; 
const int R_INT_45 = 21; 
const int COMPRIMENTO_INDICADOR = 25; // Comprimento da linha indicadora


// --- CONSTANTES E VARIÁVEIS PARA O GRÁFICO INTENSIDADE VS TEMPO ---
const int G_X_ORIGIN = 45; // Coordenada X do canto inferior esquerdo do gráfico (Origem)
const int G_Y_ORIGIN = 110; // Coordenada Y do canto inferior esquerdo do gráfico (Origem)
const int G_WIDTH = 125;    // Largura do gráfico (Eixo X - Tempo)
const int G_HEIGHT = 80;    // Altura do gráfico (Eixo Y - Intensidade)

// Variáveis de estado do gráfico
const int G_PLOT_X_INICIO = G_X_ORIGIN + 5; // Onde a reta azul começa (ex: 50)
const int G_PLOT_Y_BASE = 105;   // Onde a reta azul começa na base (ex: 105)
const int G_PLOT_Y_TETO = 35; // Onde a reta azul termina no topo (ex: 35)

// Variáveis de estado do gráfico (Usam as novas constantes)
int G_plot_x = G_PLOT_X_INICIO;      // X atual, começando no ponto calibrado
int G_plot_y_prev = G_PLOT_Y_BASE;   // Y anterior, começando no ponto calibrado

const int Y_LIMITE_INFERIOR = G_Y_ORIGIN - 1;            // 1px acima da base (Eixo X)
const int Y_LIMITE_SUPERIOR = G_Y_ORIGIN - G_HEIGHT + 10; // 1px abaixo do topo (Teto)
const int X_LIMITE_ESQUERDO = G_X_ORIGIN + 1;            // 1px à direita da parede (Eixo Y)


float G_plot_y_filtrado = G_PLOT_Y_BASE;
const float FATOR_SUAVIZACAO = 0.15;
const int MAX_STEP = 5;





float angulo_atual = 180.0; // Posição inicial (Reta 1)
float angulo_alvo = 180.0;
unsigned long tempo_anterior = 0;
const long INTERVALO_MOVIMENTO_MS = 10;

int X_anterior = CX - R_INT; 
int Y_anterior = CY;


const int CX_DIR = 180; // Centro X (mesmo do velocímetro)
const int CY_DIR = 300;  // Centro Y (abaixo de 85, para ficar acima das setas)
const int R_DIR = 30;   // Raio do ponteiro de direção

float angulo_dir_atual = 90.0; // Posição inicial no centro (90 graus)
float angulo_dir_alvo = 90.0;
unsigned long tempo_dir_anterior = 0;

// Variáveis para apagar o ponteiro de direção
int X_dir_anterior = CX_DIR; 
int Y_dir_anterior = CY_DIR - R_DIR;

const int RAIO_EXT = R_DIR + 1; // 31
const int RAIO_INT = R_DIR;     // 30


int intensidade = 0;

MCUFRIEND_kbv tela;



GFButton botaoA(A13);
GFButton botaoB(30);
GFButton botaoC(34);
GFButton botaoD(A15);


int eixoX = A9;
int eixoY = A10;



int X=44;
int Y=0;


float angulo = 90;
int alvoPWM = 0;

bool camOn = false;
bool lightOn= false;



void desenharPonteiro(float angulo_graus, int cor) {
    // Converte graus para radianos
    float rad = angulo_graus * 0.0174533; // (PI / 180)

    // Calcula a nova coordenada final (X_novo, Y_novo) usando R_INT
    int X_novo = CX + (int)(R_INT * cos(rad));
    int Y_novo = CY - (int)(R_INT * sin(rad)); 

    // Desenha a reta de 2px de espessura
    tela.drawLine(CX, CY, X_novo, Y_novo, cor);
    tela.drawLine(CX + 1, CY, X_novo + 1, Y_novo, cor);

    // Salva a nova posição para uso futuro (apagar)
    X_anterior = X_novo; 
    Y_anterior = Y_novo;
}

void desenharPonteiroDirecao(float angulo_graus, int cor) {
    // R_DIR, CX_DIR, CY_DIR são usados aqui
    float rad = angulo_graus * 0.0174533; 

    // Calcula a nova coordenada final (X_novo, Y_novo)
    int X_novo = CX_DIR + (int)(R_DIR * cos(rad));
    int Y_novo = CY_DIR - (int)(R_DIR * sin(rad)); 

    // 1. Apaga a reta na posição anterior (o que garante o efeito de "movimento")
    tela.drawLine(CX_DIR, CY_DIR, X_dir_anterior, Y_dir_anterior, FUNDO);
    tela.drawLine(CX_DIR + 1, CY_DIR, X_dir_anterior + 1, Y_dir_anterior, FUNDO);

    // 2. Desenha a nova reta
    tela.drawLine(CX_DIR, CY_DIR, X_novo, Y_novo, cor);
    tela.drawLine(CX_DIR + 1, CY_DIR, X_novo + 1, Y_novo, cor);

    // 3. Salva a nova posição para o próximo ciclo de apagar
    X_dir_anterior = X_novo; 
    Y_dir_anterior = Y_novo;
}



//

void setup() {
    Serial.begin(115200);
    Serial2.begin(115200);
 
    botaoA.setPressHandler(botaoAPressionado);
    botaoB.setPressHandler(botaoBPressionado);
    botaoC.setPressHandler(botaoCPressionado);
    botaoD.setPressHandler(botaoDPressionado);




    pinMode(eixoX, INPUT);
    pinMode(eixoY, INPUT);

    


  tela.begin( tela.readID() );
  tela.fillScreen(TFT_BLACK);




  // 8. Base do Barco 
  tela.fillTriangle(150, 175, 210, 175, 160, 190, TFT_WHITE);
  tela.drawTriangle(150, 175, 210, 175, 160, 190, TFT_WHITE);
  tela.fillTriangle(160, 190, 200, 190, 210, 175, TFT_WHITE);
  tela.drawTriangle(160, 190, 200, 190, 210, 175, TFT_WHITE);

  // 9. Mastro 

  tela.fillRect(179, 156, 2, 19, TFT_WHITE); 
  tela.drawRect(179, 156, 2, 19, TFT_WHITE);

  // 10. Vela Esquerda (Y topo 236 -> 156, Y base 252 -> 172)
  tela.fillTriangle(150, 172, 177, 156, 177, 172, TFT_WHITE);
  tela.drawTriangle(150, 172, 177, 156, 177, 172, TFT_WHITE);

  // 11. Vela Direita (Y topo 236 -> 156, Y base 252 -> 172)
  tela.fillTriangle(182, 156, 182, 172, 210, 172, TFT_WHITE);
  tela.drawTriangle(182, 156, 182, 172, 210, 172, TFT_WHITE);


  // 0. Limpa a área
tela.fillRect(CX - R_EXT, CY - R_EXT, R_EXT * 2, R_EXT, FUNDO); 

// === SEGMENTO VERDE (180 a 135 graus) ===
// A base (180 graus) está em (CX-R_EXT, CY) -> (125, 180)
tela.fillTriangle(CX-R_EXT, CY, CX-R_EXT_45, CY-R_EXT_45, CX-R_INT, CY, TFT_GREEN);
tela.fillTriangle(CX-R_EXT_45, CY-R_EXT_45, CX-R_INT_45, CY-R_INT_45, CX-R_INT, CY, TFT_GREEN);

// === SEGMENTO AMARELO (135 a 90 graus) ===
tela.fillTriangle(CX-R_EXT_45, CY-R_EXT_45, CX, CY-R_EXT, CX-R_INT_45, CY-R_INT_45, TFT_YELLOW);
tela.fillTriangle(CX, CY-R_EXT, CX, CY-R_INT, CX-R_INT_45, CY-R_INT_45, TFT_YELLOW);

// === SEGMENTO LARANJA (90 a 45 graus) ===
tela.fillTriangle(CX, CY-R_EXT, CX+R_EXT_45, CY-R_EXT_45, CX, CY-R_INT, TFT_ORANGE);
tela.fillTriangle(CX+R_EXT_45, CY-R_EXT_45, CX+R_INT_45, CY-R_INT_45, CX, CY-R_INT, TFT_ORANGE);

// === SEGMENTO VERMELHO (45 a 0 graus) ===
tela.fillTriangle(CX+R_EXT_45, CY-R_EXT_45, CX+R_EXT, CY, CX+R_INT_45, CY-R_INT_45, TFT_RED);
tela.fillTriangle(CX+R_EXT, CY, CX+R_INT, CY, CX+R_INT_45, CY-R_INT_45, TFT_RED);

// Círculo central e linha de base
tela.fillCircle(CX, CY, R_INT, FUNDO); 
tela.drawLine(CX - R_EXT, CY, CX + R_EXT, CY, FUNDO);

  // ==========================================================
  // === TEXTOS DE STATUS ===
  // ==========================================================
  tela.setTextColor(TFT_BLUE); // Define a cor Azul
  tela.setTextSize(2);        // Define o tamanho da fonte

  // 12. LUZES: OFF 
  tela.setCursor(15, 140); 
  tela.print("LIGHT: OFF"); 

  // 13. CAMERA: OFF
  tela.setCursor(15, 160); 
  tela.print("CAM: OFF");
  // ==========================================================]

  desenharPonteiro(angulo_atual, TFT_WHITE);

// === INDICADOR DE DIREÇÃO (SEMI-CÍRCULO BRANCO) ===

// 1. Desenha o Círculo Externo (270, 300)
tela.drawCircle(CX_DIR, CY_DIR, RAIO_EXT, TFT_WHITE);

// 2. Desenha o Círculo Interno
tela.drawCircle(CX_DIR, CY_DIR, RAIO_INT, TFT_WHITE);


// 3. Apaga a metade INFERIOR do círculo com um retângulo de cor de fundo (FUNDO/TFT_BLACK)
// O retângulo começa em CY_DIR (y=300) e vai para BAIXO.
tela.fillRect(
    CX_DIR - RAIO_EXT - 1, // X inicial (para cobrir a borda)
    CY_DIR,                // Y inicial: Começa no centro Y (300)
    (RAIO_EXT * 2) + 2,    // Largura total
    RAIO_EXT + 2,          // Altura (cobre a metade inferior)
    FUNDO
);

// Desenha o ponteiro inicial (90 graus)
desenharPonteiroDirecao(angulo_dir_atual, TFT_WHITE);


// 1. Desenha o Eixo X (Tempo)
// Da origem até a largura máxima. (2px de espessura)
tela.drawLine(G_X_ORIGIN, G_Y_ORIGIN, G_X_ORIGIN + G_WIDTH, G_Y_ORIGIN, TFT_WHITE); 
tela.drawLine(G_X_ORIGIN, G_Y_ORIGIN - 1, G_X_ORIGIN + G_WIDTH, G_Y_ORIGIN - 1, TFT_WHITE);

// 2. Desenha o Eixo Y (Intensidade)
// Da origem até a altura máxima. (2px de espessura)
tela.drawLine(G_X_ORIGIN, G_Y_ORIGIN, G_X_ORIGIN, G_Y_ORIGIN - G_HEIGHT, TFT_WHITE); 
tela.drawLine(G_X_ORIGIN + 1, G_Y_ORIGIN, G_X_ORIGIN + 1, G_Y_ORIGIN - G_HEIGHT, TFT_WHITE);

// 3. Desenha o contorno do gráfico
tela.drawRect(G_X_ORIGIN, G_Y_ORIGIN - G_HEIGHT, G_WIDTH + 2, G_HEIGHT + 2, TFT_WHITE);

tela.setCursor(15, 220);
tela.setTextColor(TFT_WHITE); 
tela.setTextSize(1);
tela.print("VELOCIDADE");

tela.setCursor(160, 220);
tela.setTextColor(TFT_WHITE); 
tela.setTextSize(1);
tela.print("DIRECAO");

}




void loop() {
  botaoA.process();
  botaoB.process();
  botaoC.process();
  botaoD.process();
  int leituraY = analogRead(eixoY);
  int leituraX = analogRead(eixoX);
  X = map(leituraX, 0,1023,0,90);
  Y = map(leituraY, 0,1023,-90,90);
  
  float yInv = leituraY;
  float xInv = 1023 - leituraX;

  if (leituraY > 750) {
      angulo = 90.0 - atan2(xInv - 511.5, yInv - 511.5) * 180.0 / PI;
      

      intensidade = map(yInv, 710, 1023, 1, 25);
      intensidade = constrain(intensidade, 1, 25);

      Serial2.print("A: "+String(angulo));
      Serial2.println(" I: "+ String(intensidade));
      
  }

  else{
    intensidade = 0;
    if(leituraX>950){
      angulo = 180;
      Serial2.print("An: "+ String(angulo));
      Serial2.println(" I: 0");
    }
    else if (leituraX<150){
      angulo = 0;
      Serial2.print("An: "+ String(angulo));
      Serial2.println(" I: 0");
    }
  }
  
      if (intensidade >= 24) { angulo_alvo = 0.0; }       // Reta 9
    else if (intensidade >= 21) { angulo_alvo = 22.5; }  // Reta 8
    else if (intensidade >= 18) { angulo_alvo = 45.0; }  // Reta 7
    else if (intensidade >= 15) { angulo_alvo = 67.5; }  // Reta 6
    else if (intensidade >= 12) { angulo_alvo = 90.0; }  // Reta 5
    else if (intensidade >= 9) { angulo_alvo = 112.5; } // Reta 4
    else if (intensidade >= 6) { angulo_alvo = 135.0; } // Reta 3
    else if (intensidade >= 3) { angulo_alvo = 157.5; } // Reta 2
    else { angulo_alvo = 180.0; }                     // Reta 1 (0 a 2)
    
    // --- 2. LÓGICA DE MOVIMENTO FLUIDO (NON-BLOCKING TIMING) ---

    unsigned long tempo_atual = millis();
    
    if (tempo_atual - tempo_anterior >= INTERVALO_MOVIMENTO_MS) {
        
        // --- CÁLCULO DA ACELERAÇÃO DINÂMICA (SUBSTITUI o passo_angulo = 3.0) ---
        
        // 1. Calcula a diferença total de ângulo (o quanto falta mover)
        float diferenca = angulo_alvo - angulo_atual;

        // 2. Define o novo passo: 10% da diferença restante.
        // O valor absoluto (abs) garante que o passo seja sempre positivo.
        float fator_aceleracao = 0.15; // 15% da distância restante (Você pode ajustar 0.1 a 0.5)
        float passo_angulo = abs(diferenca) * fator_aceleracao; 
        
        // 3. Garante que o passo mínimo seja respeitado para evitar travamento em 0
        if (passo_angulo < 1.0) { // Garante que a reta se move no mínimo 1.0 grau
            passo_angulo = 1.0; 
        }
        // -----------------------------------------------------------------------

        if (angulo_atual != angulo_alvo) {

            // 1. APAGA O PONTEIRO ANTERIOR
            desenharPonteiro(angulo_atual, FUNDO); 

            // 2. MOVE O ÂNGULO ATUAL
            if (angulo_atual < angulo_alvo) {
                // Diminui o ângulo (movimento horário). Passo é positivo.
                angulo_atual += passo_angulo;
                if (angulo_atual > angulo_alvo) angulo_atual = angulo_alvo;
            } else if (angulo_atual > angulo_alvo) {
                // Aumenta o ângulo (movimento anti-horário). Passo é subtraído.
                angulo_atual -= passo_angulo;
                if (angulo_atual < angulo_alvo) angulo_atual = angulo_alvo;
            }

            // 3. DESENHA O NOVO PONTEIRO
            desenharPonteiro(angulo_atual, TFT_WHITE);
        }

        tempo_anterior = tempo_atual;
    }

    angulo_dir_alvo = angulo; 

    unsigned long tempo_dir_atual = millis();

    if (tempo_dir_atual - tempo_dir_anterior >= INTERVALO_MOVIMENTO_MS) {
        
        // 1. Calcula a diferença e o passo de aceleração dinâmica (20% da distância)
        float diferenca_dir = angulo_dir_alvo - angulo_dir_atual;
        float fator_aceleracao_dir = 0.20; 
        float passo_angulo_dir = abs(diferenca_dir) * fator_aceleracao_dir; 
        
        // 2. Garante movimento mínimo de 1.0 grau
        if (passo_angulo_dir < 1.0) { 
            passo_angulo_dir = 1.0; 
        }

        if (angulo_dir_atual != angulo_dir_alvo) {
            
            // 3. MOVE O ÂNGULO ATUAL
            if (angulo_dir_atual < angulo_dir_alvo) {
                angulo_dir_atual += passo_angulo_dir;
                if (angulo_dir_atual > angulo_dir_alvo) angulo_dir_atual = angulo_dir_alvo;
            } else if (angulo_dir_atual > angulo_dir_alvo) {
                angulo_dir_atual -= passo_angulo_dir;
                if (angulo_dir_atual < angulo_dir_alvo) angulo_dir_atual = angulo_dir_alvo;
            }

            // 4. DESENHA O NOVO PONTEIRO (Apagar o antigo está embutido)
            desenharPonteiroDirecao(angulo_dir_atual, TFT_WHITE);
        }

        tempo_dir_anterior = tempo_dir_atual;
    }


    if (Serial2.available()) {
        String texto = Serial2.readStringUntil('\n');
        texto.trim();
        texto.replace("  ", " "); 
        texto.toLowerCase();

      if(texto == "light on"){
        if(lightOn == true){
        mudaLuz();
        }
      }
      if(texto == "light off"){
        if(lightOn == false){
        mudaLuz();
        }
      }
      
      if(texto == "cam off"){
        if(camOn == false){
        mudaCam();
        }
      }
      if(texto == "cam on"){
        if(camOn == true){
        mudaCam();
        }
      }
      
  }



const long INTERVALO_PLOTAGEM_MS = 100; // Avança 1 pixel no eixo X a cada 100ms
static unsigned long tempo_plot_anterior = 0; // Temporizador dedicado para o gráfico
const int X_PLOT_INICIO = G_PLOT_X_INICIO;
const int X_PLOT_FIM = G_X_ORIGIN + G_WIDTH - 2;

unsigned long tempo_atual_plot = millis();

if (tempo_atual_plot - tempo_plot_anterior >= INTERVALO_PLOTAGEM_MS) {
    
    int intensidade_mapeada = constrain(intensidade, 0, 25); 
    int Y_ALVO = map(
        intensidade_mapeada, 
        0, 25, 
        G_PLOT_Y_BASE,
        G_PLOT_Y_TETO
    ); // mapeia a intensidade 1-25, para base-teto (preciso mudar se mudar a intensidade)
    
    int step_size = Y_ALVO - G_plot_y_filtrado; // filtrado = piso... alvo - piso = tamanho
    // max step anda no máximo até 5 de uma vez
    
    if (abs(step_size) > MAX_STEP) { // se o módulo do tamanho for maior que o passo máximo
        if (step_size > 0) { 
            G_plot_y_filtrado += MAX_STEP;
        } else {
            G_plot_y_filtrado -= MAX_STEP;
        } // modifica o "piso", ou seja, anda no máximo 5
    } else {
        G_plot_y_filtrado = Y_ALVO; // se não, anda oque foi lido
    }
    
    int G_plot_y_atual = (int)G_plot_y_filtrado; // piso "atual" = piso somado
    
    if (G_plot_x >= X_PLOT_FIM) { //se a parede esquerda passar ou for igual ao fim, reseta pintando de preto por cima e volta pro inicio que ela deveria estar
        
        // Limpa a área com um retângulo preto
        tela.fillRect(G_X_ORIGIN + 1, G_Y_ORIGIN - G_HEIGHT + 1, G_WIDTH - 2, G_HEIGHT - 2, FUNDO); 
        
        G_plot_x = X_PLOT_INICIO;
    }
    
    if (G_plot_x > X_PLOT_INICIO) { // se a parede esquerda for maior que o inicio, desenha diagonalmente
      
        // Desenha 5 linhas adjacentes para engrossar a reta.
        
        // Y (Centro) 
        tela.drawLine(G_plot_x - 1, G_plot_y_prev, G_plot_x, G_plot_y_atual, TFT_BLUE); // desenha na parede esquerda -1 até parede esquerda. piso até piso somado "atual"
        
        // Y-1 e Y-2 (Acima)
        tela.drawLine(G_plot_x - 1, G_plot_y_prev - 1, G_plot_x, G_plot_y_atual - 1, TFT_BLUE);
        tela.drawLine(G_plot_x - 1, G_plot_y_prev - 2, G_plot_x, G_plot_y_atual - 2, TFT_BLUE);
        
        // Y+1 e Y+2 (Abaixo)
        tela.drawLine(G_plot_x - 1, G_plot_y_prev + 1, G_plot_x, G_plot_y_atual + 1, TFT_BLUE);
        tela.drawLine(G_plot_x - 1, G_plot_y_prev + 2, G_plot_x, G_plot_y_atual + 2, TFT_BLUE);
        
    } else {
        // Desenho do primeiro ponto 5x5 no ponto de início.
        tela.fillRect(G_plot_x - 2, G_plot_y_prev - 2, 5, 5, TFT_BLUE); 
    }

    G_plot_x++; // avança parede
    G_plot_y_prev = G_plot_y_atual; // piso vira o piso somado "atual"
    tempo_plot_anterior = tempo_atual_plot;
}


}




void botaoAPressionado(GFButton& botaoA){
  Serial2.println("A");
  mudaCam();

}
void botaoBPressionado(GFButton& botaoB){
  Serial2.println("B");
}
void botaoCPressionado(GFButton& botaoC){
  Serial2.println("C");
}
void botaoDPressionado(GFButton& botaoD){
  Serial2.println("D");
  mudaLuz();
  

}

void mudaLuz() {
    if (lightOn) {
        
        tela.setTextColor(TFT_BLACK);
        tela.setTextSize(2);
        tela.setCursor(15, 140); 
        tela.print("LIGHT: OFF");

        
        tela.setTextColor(TFT_BLUE);
        tela.setCursor(15, 140); 
        tela.print("LIGHT: ON");

        lightOn = false;
    } else {
        tela.setTextColor(TFT_BLACK);
        tela.setTextSize(2);
        tela.setCursor(15, 140); 
        tela.print("LIGHT: ON");

        tela.setTextColor(TFT_BLUE);
        tela.setCursor(15, 140); 
        tela.print("LIGHT: OFF");

        lightOn = true;
    }
}

void mudaCam(){
  if (camOn) {
    
    tela.setTextColor(TFT_BLACK);
    tela.setTextSize(2);
    tela.setCursor(15,160); 
    tela.print("CAM: OFF"); 

    
    tela.setTextColor(TFT_BLUE);
    tela.setCursor(15,160); 
    tela.print("CAM: ON");
    camOn=false;
  } 
  else {
      
      tela.setTextColor(TFT_BLACK);
      tela.setTextSize(2);
      tela.setCursor(15,160); 
      tela.print("CAM: ON"); 

     
      tela.setTextColor(TFT_BLUE);
      tela.setCursor(15,160); 
      tela.print("CAM: OFF"); 
      camOn = true;
  }
}




