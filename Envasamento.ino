#include <EEPROM.h>      // incluir a biblioteca EEPROM
#include "Ultrasonic.h"  // incluir a biblioteca Ultrasonic

const int Btn_AutoMan = 2;        // Pino digital utilizado pelo botao automatico = 1, Manual = 0.
const int Btn_EsteiraAvanca = 3;  // Pino digital utilizado pelo botao avança esteira.
const int echoPin = 7;            // Pino digital utilizado pelo HC-SR04 ECHO(RECEBE).
const int trigPin = 8;            // Pino digital utilizado pelo HC-SR04 TRIG(ENVIA).
const int Bomba = 10;             // Pino digital utilizado pelo rele da bomba.
const int Esteira = 11;           // Pino digital utilizado pelo rele da Esteira.
const int ledPin = 13;            // Pino digital utilizado pelo LED.

Ultrasonic ultrasonic(trigPin, echoPin);  //Iniciando os pinos do HC-SR04.

// Configuraçao das variáveis
int distancia;
String result;
int TempoEnvase = 0;  // Tempo de envase 10000 ms = 10s
int TempoEnvaseSalvo = 0;
int Contador = 0;
int ContadorAnterior = 1;
int estado = 0;  // variável para leitura do pushbutton
boolean RecipienteCheio = true;
boolean Est_Automatico = false;  // variável para armazenar valores do pushbutton
boolean RecipientePosicionado = false;
boolean LIGA = false;
boolean DESLIGA = true;


void setup() {
  Serial.begin(9600);
  Serial.println("Iniciando Setup...");
  pinMode(Bomba, OUTPUT);                    // define o pino do rele liga bomba como saída do Arduino
  pinMode(ledPin, OUTPUT);                   // define o pino do ledPin saída do Arduino
  pinMode(Esteira, OUTPUT);                  // define o pino do rele liga esteira como saída do Arduino
  pinMode(Btn_AutoMan, INPUT_PULLUP);        // define pino do botão automático como entrada do Arduino:
  pinMode(Btn_EsteiraAvanca, INPUT_PULLUP);  // define pino do botão avança esteira como entrada do Arduino:
  pinMode(echoPin, INPUT);                   // define pino como entrada do HC-SR04
  pinMode(trigPin, OUTPUT);                  // define pino como saida do HC-SR04
  digitalWrite(Esteira, DESLIGA);            // Inicia esteira desligada
  digitalWrite(Bomba, DESLIGA);              // Inicia bomba desligada

  TempoEnvaseSalvo = (EEPROM.read(0) * 1000);  // Recupera valor Tempo envase salvo
  if (TempoEnvaseSalvo < 5000) {
    TempoEnvaseSalvo = 5000;
  }

  TempoEnvase = TempoEnvaseSalvo;  // Setup valor Tempo envase
  Serial.print("Tempo Envase Configurado para: ");
  Serial.print(TempoEnvase / 1000);
  Serial.println("s");
  Serial.println("Setup OK!");
  Serial.println("Tudo pronto Para Iniciar");
}

void loop() {

  hcsr04();  // Faz a chamada do método "hcsr04()"
  // Serial.print("Distancia ");  // Imprime o texto no monitor serial
  // Serial.print(distancia);        // Imprime a distância medida no monitor serial
  // Serial.println("cm");        // Imprime o texto no monitor serial

  if (distancia < 5) {
    RecipientePosicionado = true;
    // Serial.println("Recipiente posicionado");  // Imprime o texto no monitor serial
  } else {
    RecipientePosicionado = false;
  }

  if (Serial.available() > 0) {
    char Recebido = Serial.read();
    //    Serial.print("Recebido: ");
    //    Serial.println(Recebido);

    if (Recebido == '+') {

      if (TempoEnvase < 30000) {
        TempoEnvase = (TempoEnvase + 1000);
      }
      Serial.print("Tempo Envase: ");
      Serial.print(TempoEnvase / 1000);
      Serial.println("s");
    }

    if (Recebido == '-') {

      if (TempoEnvase > 5000) {
        TempoEnvase = TempoEnvase - 1000;
      }
      Serial.print("Tempo Envase: ");
      Serial.print(TempoEnvase / 1000);
      Serial.println("s");
    }

    if (Recebido == 'm') {

      if (Contador < 10000) {
        Contador++;
      }
      Serial.print("Numeros de Recipiente Cheio = ");
      Serial.println(Contador);
    }

    if (Recebido == 'n') {

      if (Contador > 0) {
        Contador--;
      }
      Serial.print("Numeros de Recipiente Cheio = ");
      Serial.println(Contador);
    }

    if (Recebido == 'z') {
      Contador = 0;
      Serial.print("Numeros de Recipiente Cheio = ");
      Serial.println(Contador);
    }
  }

  // lê o estado pushbutton: ligado (HIGH) ou desligado (LOW)
  if (Est_Automatico != digitalRead(Btn_AutoMan)) {
    Est_Automatico = digitalRead(Btn_AutoMan);
    Est_Automatico ? Serial.println("Sistema em Automático") : Serial.println("Sistema em Manual");
  }

  // Funcionamento em automático
  if (Est_Automatico) {
    // liga o led
    digitalWrite(ledPin, HIGH);
    if (RecipienteCheio || RecipientePosicionado == false) {
      if (Contador != ContadorAnterior) {
        ContadorAnterior = Contador;
        Serial.println("Avançando Esteira");
        Serial.println("Aguandando Recipiente...");
      }
      digitalWrite(Esteira, LIGA);
      if (RecipientePosicionado == LOW) {
        RecipienteCheio = false;
      }

    } else {
      digitalWrite(Esteira, DESLIGA);
    }
    if (RecipienteCheio == false && digitalRead(Esteira) == DESLIGA && RecipientePosicionado) {
      delay(1000);  // Aguarda 1s para iniciar envase.
      digitalWrite(Bomba, LIGA);
      Serial.println("Envasamento iniciado! ");
      delay(TempoEnvase);
      digitalWrite(Bomba, DESLIGA);
      Serial.print("Recipiente Cheio! ");
      RecipienteCheio = true;
      Contador++;
      Serial.print("Total = ");
      Serial.println(Contador);
      delay(1000);  // Aguarda 1s para iniciar movimento da esteira.
    }
  } else {
    // desliga o led
    digitalWrite(ledPin, LOW);

    // Verifica se botao avança esteira esta precionado
    if (digitalRead(Btn_EsteiraAvanca)) {
      // Liga esteira em manual
      digitalWrite(Esteira, LIGA);
    } else {
      // Desliga esteira em manual
      digitalWrite(Esteira, DESLIGA);
    }

    // Desliga esteira em manual
    // digitalWrite(Esteira, DESLIGA);

    // Desliga bomba em manual
    digitalWrite(Bomba, DESLIGA);
  }

  if (TempoEnvaseSalvo != TempoEnvase) {
    TempoEnvaseSalvo = TempoEnvase;
    EEPROM.write(0, TempoEnvaseSalvo / 1000);
    Serial.print("Tempo de envase salvo: ");
    Serial.print(TempoEnvaseSalvo / 1000);
    Serial.print("s \t");
    Serial.print("EEPROM: ");
    Serial.print(EEPROM.read(0));
    Serial.println("s");
  }
}

// Método responsável por calcular a distância
void hcsr04() {
  digitalWrite(trigPin, LOW);   // Seta o pino 7 com um pulso baixo "LOW"
  delayMicroseconds(10);        // Intervalo de 2 microssegundos
  digitalWrite(trigPin, HIGH);  // Seta o pino 8 com um pulso baixo "HIGH"
  delayMicroseconds(10);        // Intervalo de 10 microssegundos
  digitalWrite(trigPin, LOW);   // Seta o pino 7 com um pulso baixo "LOW" novamente
  // Função ranging, faz a conversão do tempo de resposta do echo em centimetros, e armazena na variavel "distancia"
  distancia = (ultrasonic.Ranging(CM));  // variável global recebe o valor da distância medida
  result = String(distancia);            // variável global do tipo string recebe a distância(convertido de inteiro para string)
  delay(100);                            // intervalo de 10 milissegundos
}
