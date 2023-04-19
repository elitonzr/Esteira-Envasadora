/*
* Programa : Esteira Envasadora
* Autor : Eliton Roberto Monteiro
* Data: 01/04/2023
* Versão: 1.0v
*/

#include <EEPROM.h>      // incluir a biblioteca EEPROM
#include "Ultrasonic.h"  // incluir a biblioteca Ultrasonic

const int Btn_AutoMan = 2;        // Pino digital utilizado pelo botão automático = 1, Manual = 0.
const int Btn_EsteiraAvanca = 3;  // Pino digital utilizado pelo botão avança esteira.
const int echoPin = 7;            // Pino digital utilizado pelo HC-SR04 ECHO(RECEBE).
const int trigPin = 8;            // Pino digital utilizado pelo HC-SR04 TRIG(ENVIA).
const int Bomba = 10;             // Pino digital utilizado pelo rele da bomba.
const int Esteira = 11;           // Pino digital utilizado pelo rele da Esteira.
const int LED_Automatico = 13;    // Pino digital utilizado pelo LED_Automatico para sinalizar sistema em automático.

Ultrasonic ultrasonic(trigPin, echoPin);  //Iniciando os pinos do HC-SR04.

// Configuração das variáveis
unsigned long startTime;
unsigned long currentTime;
const unsigned long period = 2000;
int TempoEnvase = 0;                    // variável para armazenar tempo de envase.
int TempoEnvaseSalvo = 0;               // variável para armazenar tempo de envase alterado durante execução.
int Contador = 0;                       // variável para armazenar quantidade de recipientes cheios.
int ContadorAnterior = 1;               // variável para armazenar quantidade de recipientes cheios anteriormente.
int estado = 0;                         // variável para leitura do pushbutton
int distancia;                          // variável para armazenar distância lida pelo sensor HC-SR04.
String result;                          // variável para armazenar resultado da leitura do sensor HC-SR04.
boolean RecipienteCheio = true;         // variável para confirmar recipientes cheios.
boolean Est_Automatico = false;         // variável para confirmar estado de automático.
boolean RecipientePosicionado = false;  // variável para indicar recipiente na posição de envase.
boolean LIGA = false;                   // variável para armazenar estado pra ligar reles.
boolean DESLIGA = true;                 // variável para armazenar estado pra desligar reles.
boolean BombaEstado = false;


void setup() {
  Serial.begin(9600);                        // Inicia comunicação serial.
  Serial.println("Iniciando Setup...");      // Imprime o texto no monitor serial.
  pinMode(Bomba, OUTPUT);                    // define o pino do rele liga bomba como saída do Arduino.
  pinMode(LED_Automatico, OUTPUT);           // define o pino do LED_Automatico saída do Arduino.
  pinMode(Esteira, OUTPUT);                  // define o pino do rele liga esteira como saída do Arduino.
  pinMode(Btn_AutoMan, INPUT_PULLUP);        // define pino do botão automático como entrada do Arduino.
  pinMode(Btn_EsteiraAvanca, INPUT_PULLUP);  // define pino do botão avança esteira como entrada do Arduino.
  pinMode(echoPin, INPUT);                   // define pino como entrada do HC-SR04.
  pinMode(trigPin, OUTPUT);                  // define pino como saída do HC-SR04.
  digitalWrite(Esteira, DESLIGA);            // Inicia esteira desligada.
  digitalWrite(Bomba, DESLIGA);              // Inicia bomba desligada.

  TempoEnvaseSalvo = (EEPROM.read(0) * 1000);  // Função if que recupera valor do tempo salvo do envase.

  // Função if para garantir tempo minimo de 5s do envase.
  if (TempoEnvaseSalvo < 5000) {
    TempoEnvaseSalvo = 5000;
  }

  TempoEnvase = TempoEnvaseSalvo;                   // Configura valor do tempo de envase que foi salvo na EEPROM.
  Serial.print("Tempo envase configurado para: ");  // Imprime o texto no monitor serial.
  Serial.print(TempoEnvase / 1000);                 // Imprime o texto no monitor serial.
  Serial.println("s");                              // Imprime o texto no monitor serial.
  Serial.println("Setup OK!");                      // Imprime o texto no monitor serial.
  Serial.println("Tudo pronto para iniciar");       // Imprime o texto no monitor serial.
}  // Fim do setup

void loop() {

  hcsr04();  // Faz a chamada do método "hcsr04()"

  // Aqui definimos a posição do recipiente, nesse caso deve ser menor que 10cm.
  if (distancia <= 10) {
    RecipientePosicionado = true;  // Recipiente posicionado de maneira correta.
  } else {
    RecipientePosicionado = false;  // Recipiente posicionado não está na posição esperada.
  }

  currentTime = millis();
  // Quando o sistema estiver em manual será mostrado a distância lida pelo sensor hcsr04 a cada 2s.
  if (!Est_Automatico && distancia < 999 && (currentTime - startTime >= period)) {
    Serial.print("Distância ");  // Imprime o texto no monitor serial
    Serial.print(distancia);     // Imprime a distância medida no monitor serial
    Serial.println("cm");        // Imprime o texto no monitor serial
    startTime = currentTime;
  }

  // lê o estado do botão automático ou manual: manual (HIGH) ou automático (LOW).
  if (Est_Automatico == digitalRead(Btn_AutoMan)) {
    Est_Automatico = !digitalRead(Btn_AutoMan);                                                      // O estado do botão automático/manual é armazena na variável Est_Automatico.
    Est_Automatico ? Serial.println("Sistema em automático") : Serial.println("Sistema em manual");  // Imprime o texto no monitor serial.
  }

  StartEsteira();                                // Função de controle da esteira.
  StartEnvase();                                 // Função de controle da bomba de envase.
  ComunicacaoSerial();                           // Função de controle da comunicação serial.
  SalvaTempoEnvase();                            // Função de controle do tempo de envase que é salvo na EEPROM.
  digitalWrite(LED_Automatico, Est_Automatico);  // controle do LED para indicar estado automático ou manual
}  // Fim do loop

// Função responsável por calcular a distância
void hcsr04() {
  digitalWrite(trigPin, LOW);   // Seta o pino 7 com um pulso baixo "LOW"
  delayMicroseconds(10);        // Intervalo de 2 microssegundos
  digitalWrite(trigPin, HIGH);  // Seta o pino 8 com um pulso baixo "HIGH"
  delayMicroseconds(10);        // Intervalo de 10 microssegundos
  digitalWrite(trigPin, LOW);   // Seta o pino 7 com um pulso baixo "LOW" novamente
  // Função ranging, faz a conversão do tempo de resposta do echo em centimetros, e armazena na variavel "distancia"
  distancia = (ultrasonic.Ranging(CM));  // variável global recebe o valor da distância medida
  if (distancia == 0) {
    distancia = 999;
  }
  result = String(distancia);  // variável global do tipo string recebe a distância(convertido de inteiro para string)
  delay(10);                   // intervalo de 10 milissegundos
}

// Função responsável
void StartEsteira() {
  if (Est_Automatico) {
    if (RecipienteCheio || RecipientePosicionado == false) {
      if (Contador != ContadorAnterior) {
        ContadorAnterior = Contador;
        Serial.println("Avançando esteira");
        Serial.println("Aguandando recipiente...");
      }
      digitalWrite(Esteira, LIGA);
      if (RecipientePosicionado == LOW) {
        RecipienteCheio = false;
      }

    } else {
      digitalWrite(Esteira, DESLIGA);
    }
  } else {
    // Verifica se botão avança esteira esta precionado
    if (!digitalRead(Btn_EsteiraAvanca)) {
      // Liga esteira em manual
      digitalWrite(Esteira, LIGA);
    } else {
      // Desliga esteira em manual
      digitalWrite(Esteira, DESLIGA);
    }
  }
}

void StartEnvase() {
  if (Est_Automatico) {
    // Função if para iniciar envase, esteira deve estar desligada e recipiente vazio e na posição.
    if (digitalRead(Esteira) == DESLIGA && RecipienteCheio == false && RecipientePosicionado) {
      Serial.print("Recipiente posicionado\t");  // Imprime o texto no monitor serial.
      Serial.print("distancia ");                // Imprime o texto no monitor serial.
      Serial.print(distancia);                   // Imprime a distância medida no monitor serial.
      Serial.println("cm");                      // Imprime o texto no monitor serial.
      Serial.print("Envasamento iniciado\t");    // Imprime o texto no monitor serial.
      Serial.print("tempo: ");                   // Imprime o texto no monitor serial.
      Serial.print(TempoEnvase / 1000);          // Imprime o texto no monitor serial.
      Serial.println("s");                       // Imprime o texto no monitor serial.
      delay(1000);                               // Aguarda 1s para iniciar envase.
      digitalWrite(Bomba, LIGA);                 // Liga bomba de envase.
      delay(TempoEnvase);                        // Aguarda tempo de envase
      digitalWrite(Bomba, DESLIGA);              // Desliga bomba de envase.
      Serial.print("Recipiente cheio! ");        // Imprime o texto no monitor serial.
      RecipienteCheio = true;                    // Confirma recipiente cheio.
      Contador++;                                // Incrementa contador.
      Serial.print("Total = ");                  // Imprime o texto no monitor serial.
      Serial.println(Contador);                  // Imprime o texto no monitor serial.
      delay(1000);                               // Aguarda 1s para iniciar movimento da esteira.
    }
  } else {
    // Desliga bomba em manual
    digitalWrite(Bomba, DESLIGA);
  }
}

void SalvaTempoEnvase() {
  if (TempoEnvaseSalvo != TempoEnvase) {
    TempoEnvaseSalvo = TempoEnvase;
    EEPROM.write(0, TempoEnvaseSalvo / 1000);
    Serial.print("Tempo de envase salvo: ");
    Serial.print(TempoEnvaseSalvo / 1000);
    Serial.print("s \t");
    Serial.print("EEPROM 0: ");
    Serial.print(EEPROM.read(0));
    Serial.println("s");
  }
}

void ComunicacaoSerial() {
  if (Serial.available() > 0) {
    char Recebido = Serial.read();
    //    Serial.print("Recebido: ");
    //    Serial.println(Recebido);

    if (Recebido == '+') {

      if (TempoEnvase < 30000) {
        TempoEnvase = (TempoEnvase + 1000);
      }
      Serial.print("Tempo envase: ");
      Serial.print(TempoEnvase / 1000);
      Serial.println("s");
    }

    if (Recebido == '-') {

      if (TempoEnvase > 5000) {
        TempoEnvase = TempoEnvase - 1000;
      }
      Serial.print("Tempo envase: ");
      Serial.print(TempoEnvase / 1000);
      Serial.println("s");
    }

    if (Recebido == 'm') {

      if (Contador < 10000) {
        Contador++;
      }
      Serial.print("Números de recipiente cheio = ");
      Serial.println(Contador);
    }

    if (Recebido == 'n') {

      if (Contador > 0) {
        Contador--;
      }
      Serial.print("Números de recipiente cheio = ");
      Serial.println(Contador);
    }

    if (Recebido == 'z') {
      Contador = 0;
      Serial.print("Números de recipiente cheio = ");
      Serial.println(Contador);
    }
  }
}