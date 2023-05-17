/*
* Programa : Esteira Envasadora
* Autor : Eliton Roberto Monteiro
* Data: 01/04/2023
* Versão: 1.3v
*/

#include <EEPROM.h>      // incluir a biblioteca EEPROM.
#include "Ultrasonic.h"  // incluir a biblioteca Ultrasonic para funcionar com o sensor ultrassônico HC-SR04, download em https://github.com/makerhero/Ultrasonic.git.

const int Btn_AutoMan = 2;        // Pino digital utilizado pelo botão automático = 1, Manual = 0.
const int Btn_EsteiraAvanca = 3;  // Pino digital utilizado pelo botão avança esteira.
const int echoPin = 4;            // Pino digital utilizado pelo HC-SR04 ECHO(RECEBE).
const int trigPin = 5;            // Pino digital utilizado pelo HC-SR04 TRIG(ENVIA).
const int Rele_Esteira = 6;       // Pino digital utilizado pelo rele da Esteira.
const int Rele_Bomba = 7;         // Pino digital utilizado pelo rele da Bomba.
const int LED_RGB_Verde = 12;     // Pino digital utilizado pelo LED RGB cor verde para sinalizar sistema em manual.
const int LED_RGB_Vermelho = 13;  // Pino digital utilizado pelo LED RGB cor vermelha para sinalizar sistema em automático.

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
  Serial.begin(9600);                                                                  // Inicia comunicação serial.
  Serial.print(F("Sketch:   " __FILE__ "\nCompiled: " __DATE__ " " __TIME__ "\n\n"));  // Só para saber qual programa está rodando no meu Arduino.
  Serial.println("\n\n\nIniciando Setup...");                                          // Imprime o texto no monitor serial.
  pinMode(Rele_Bomba, OUTPUT);                                                         // define o pino do rele liga bomba como saída do Arduíno.
  pinMode(LED_RGB_Vermelho, OUTPUT);                                                   // define o pino do LED RGB cor verde saída do Arduíno.
  pinMode(LED_RGB_Verde, OUTPUT);                                                      // define o pino do LED RGB cor vermelha saída do Arduíno.
  pinMode(Rele_Esteira, OUTPUT);                                                       // define o pino do rele liga esteira como saída do Arduíno.
  pinMode(Btn_AutoMan, INPUT_PULLUP);                                                  // define pino do botão automático como entrada do Arduíno.
  pinMode(Btn_EsteiraAvanca, INPUT_PULLUP);                                            // define pino do botão avança esteira como entrada do Arduíno.
  pinMode(echoPin, INPUT);                                                             // define pino como entrada do HC-SR04.
  pinMode(trigPin, OUTPUT);                                                            // define pino como saída do HC-SR04.
  digitalWrite(Rele_Esteira, DESLIGA);                                                 // Inicia esteira desligada.
  digitalWrite(Rele_Bomba, DESLIGA);                                                   // Inicia bomba desligada.
  TempoEnvaseSalvo = (EEPROM.read(0) * 1000);                                          // Recupera valor do tempo salvo do envase.
  TempoEnvase = TempoEnvaseSalvo;                                                      // Configura valor do tempo de envase que foi salvo na EEPROM.
  // Função if para garantir tempo mínimo de 1s do envase.
  if (TempoEnvase < 1000) {
    TempoEnvase = 1000;
  }
  Serial.print("Tempo de envase configurado para: ");  // Imprime o texto no monitor serial.
  Serial.print(TempoEnvase / 1000);                    // Imprime o texto no monitor serial.
  Serial.println("s");                                 // Imprime o texto no monitor serial.
  Serial.println("Setup finalizado");                  // Imprime o texto no monitor serial.
  Serial.println("Tudo pronto para iniciar");          // Imprime o texto no monitor serial.
}

void loop() {

  // Lê o estado do botão automático ou manual: manual (HIGH) ou automático (LOW), e armazena o estado invertido na variável Est_Automatico.
  if (Est_Automatico == digitalRead(Btn_AutoMan)) {
    Est_Automatico = !digitalRead(Btn_AutoMan);                                                          // O estado do botão automático/manual é armazena na variável Est_Automatico de forma invertida.
    Est_Automatico ? Serial.println("\nSistema em automático") : Serial.println("\nSistema em manual");  // Imprime o texto no monitor serial.
  }

  // Controle dos LEDs para indicar estado automático ou manual.
  if (Est_Automatico) {
    digitalWrite(LED_RGB_Vermelho, HIGH);  // Liga LED RGB cor vermelha que indicar estado automático.
    digitalWrite(LED_RGB_Verde, LOW);      // Desliga LED RGB cor verde que indicar estado manual.
  } else {
    digitalWrite(LED_RGB_Vermelho, LOW);  // desliga LED RGB cor vermelha que indicar estado automático.
    digitalWrite(LED_RGB_Verde, HIGH);    // Liga LED RGB cor verde que indicar estado manual.
  }

  hcsr04();                // Função usada para verificar posição do recipiente.
  StartEsteira();          // Função de controle da esteira.
  StartEnvase();           // Função de controle da bomba de envase.
  ComunicacaoBluetooth();  // Função de controle da comunicação Bluetooth.
  SalvaTempoEnvase();      // Função de controle do tempo de envase que é salvo na EEPROM.

  currentTime = millis();
  // Quando o sistema estiver em manual será mostrado no monitor serial a distância lida pelo sensor hcsr04 a cada 2s.
  if (!Est_Automatico && distancia < 999 && (currentTime - startTime >= period)) {
    Serial.print("Distância ");  // Imprime o texto no monitor serial
    Serial.print(distancia);     // Imprime a distância medida no monitor serial
    Serial.println("cm");        // Imprime o texto no monitor serial
    startTime = currentTime;
  }
}

// Função responsável por calcular a distância
void hcsr04() {
  digitalWrite(trigPin, LOW);   // Seta o pino 7 com um pulso baixo "LOW"
  delayMicroseconds(10);        // Intervalo de 2 microssegundos
  digitalWrite(trigPin, HIGH);  // Seta o pino 8 com um pulso baixo "HIGH"
  delayMicroseconds(10);        // Intervalo de 10 microssegundos
  digitalWrite(trigPin, LOW);   // Seta o pino 7 com um pulso baixo "LOW" novamente
  // Função ranging, faz a conversão do tempo de resposta do echo em centímetros, e armazena na variável "distancia"
  distancia = (ultrasonic.Ranging(CM));  // variável global recebe o valor da distância medida
  if (distancia == 0) {
    distancia = 999;
  }
  result = String(distancia);  // variável global do tipo string recebe a distância(convertido de inteiro para string)
  delay(10);                   // intervalo de 10 milissegundos

  // Aqui definimos a posição do recipiente, nesse caso deve ser menor que 7cm.
  if (distancia <= 7) {
    RecipientePosicionado = true;  // Recipiente posicionado de maneira correta.
  } else {
    RecipientePosicionado = false;  // Recipiente não está na posição esperada.
  }
}

// Função responsável pelo funcionamento da esteira.
void StartEsteira() {
  // Operação da esteira em automático.
  if (Est_Automatico) {
    // Liga a esteira em automático se recipiente estiver cheio ou não estiver na posição de envase.
    if (RecipienteCheio || RecipientePosicionado == false) {
      // Envia mensagem de "Avançando esteira e aguardando recipiente..." após envase.
      if (Contador != ContadorAnterior) {
        ContadorAnterior = Contador;                                           // Variável ContadorAnterior recebe valor de Contador.
        Serial.println("\n\n\nAvançando esteira e aguardando recipiente...");  // Imprime o texto no monitor serial.
      }
      digitalWrite(Rele_Esteira, LIGA);  // Liga esteira em automático, após envase.
      // Confirma recipiente fora de posição após esteirar ligar.
      if (RecipientePosicionado == LOW) {
        RecipienteCheio = false;  // Confirma que próximo recipiente não está cheio após esteirar ligar.
      }

    } else {
      digitalWrite(Rele_Esteira, DESLIGA);  // Desliga esteira em automático.
      delay(1000);                          // Aguarda 1s para confirmar esteira parada.
      hcsr04();                             // Função usada para verificar posição do recipiente.
    }
  } else {
    // Verifica se botão avança esteira esta precionado.
    if (!digitalRead(Btn_EsteiraAvanca)) {
      // Liga esteira em manual.
      digitalWrite(Rele_Esteira, LIGA);
    } else {
      // Desliga esteira em manual.
      digitalWrite(Rele_Esteira, DESLIGA);
    }
  }
}

// Função responsável pelo funcionamento do envase.
void StartEnvase() {

  boolean EST_Esteira = digitalRead(Rele_Esteira);

  if (Est_Automatico) {
    // Função if para iniciar envase, esteira deve estar desligada e recipiente vazio e na posição.
    if (EST_Esteira == DESLIGA && RecipienteCheio == false && RecipientePosicionado) {
      Serial.print("Recipiente posicionado a ");     // Imprime o texto no monitor serial.
      Serial.print(distancia);                       // Imprime a distância medida no monitor serial.
      Serial.println("cm");                          // Imprime o texto no monitor serial.
      Serial.print("Envasamento iniciado tempo: ");  // Imprime o texto no monitor serial.
      Serial.print(TempoEnvase / 1000);              // Imprime o texto no monitor serial.
      Serial.println("s");                           // Imprime o texto no monitor serial.
      digitalWrite(Rele_Bomba, LIGA);                // Liga bomba de envase.
      delay(TempoEnvase);                            // Aguarda tempo de envase
      digitalWrite(Rele_Bomba, DESLIGA);             // Desliga bomba de envase.
      Serial.print("Recipiente cheio! ");            // Imprime o texto no monitor serial.
      RecipienteCheio = true;                        // Confirma recipiente cheio.
      Contador++;                                    // Incrementa contador.
      Serial.print("Total: ");                       // Imprime o texto no monitor serial.
      Serial.println(Contador);                      // Imprime o texto no monitor serial.
      delay(1000);                                   // Aguarda 1s para iniciar movimento da esteira.
    }
  } else {
    digitalWrite(Rele_Bomba, DESLIGA);  // Desliga bomba em manual
  }
}

void SalvaTempoEnvase() {
  // Caso o tempo de envase salvo seja diferente do tempo de envase atual atualizamos o endereço 0 da EEPROM com o novo tempo de envase.
  if (TempoEnvaseSalvo != TempoEnvase) {
    TempoEnvaseSalvo = TempoEnvase;            // Armazena TempoEnvase na variável TempoEnvaseSalvo.
    EEPROM.write(0, TempoEnvaseSalvo / 1000);  // Divide TempoEnvaseSalvo por 1000 e armazena no endereço 0 da EEPROM.
    Serial.print("Tempo de envase salvo: ");   // Imprime o texto no monitor serial.
    Serial.print(TempoEnvaseSalvo / 1000);     // Imprime o texto no monitor serial.
    Serial.print("s \t");                      // Imprime o texto no monitor serial.
    Serial.print("EEPROM 0: ");                // Imprime o texto no monitor serial.
    Serial.print(EEPROM.read(0));              // Imprime o texto no monitor serial.
    Serial.println("s");                       // Imprime o texto no monitor serial.
  }
}

void ComunicacaoBluetooth() {
  // Verifica se foi recebido algo na serial.
  if (Serial.available() > 0) {
    char Recebido = Serial.read();  // Armazena carácter recebido na variável Recebido.
    Serial.print("Recebido: ");     // Imprime o texto no monitor serial.
    Serial.println(Recebido);       // Imprime o texto no monitor serial.

    // Incrementa valor do tempo de envase.
    if (Recebido == '+') {

      if (TempoEnvase < 30000) {
        TempoEnvase = (TempoEnvase + 1000);  // Incrementa 1 segundo no valor do tempo de envase.
      }
      Serial.print("Tempo envase: ");    // Imprime o texto no monitor serial.
      Serial.print(TempoEnvase / 1000);  // Imprime o texto no monitor serial.
      Serial.println("s");               // Imprime o texto no monitor serial.
    }

    // Decrementa valor do tempo de envase.
    if (Recebido == '-') {

      if (TempoEnvase > 1000) {
        TempoEnvase = TempoEnvase - 1000;  // Decrementa 1 segundo no valor do tempo de envase.
      }
      Serial.print("Tempo envase: ");    // Imprime o texto no monitor serial.
      Serial.print(TempoEnvase / 1000);  // Imprime o texto no monitor serial.
      Serial.println("s");               // Imprime o texto no monitor serial.
    }

    // Incrementa valor do contador de envase.
    if (Recebido == 'm') {

      if (Contador < 10000) {
        Contador++;  // Incrementa 1 unidade no valor do contador de envase.
      }
      Serial.print("Números de recipiente cheio = ");  // Imprime o texto no monitor serial.
      Serial.println(Contador);                        // Imprime o texto no monitor serial.
    }

    // Decrementa valor do contador de envase.
    if (Recebido == 'n') {

      if (Contador > 0) {
        Contador--;  // Decrementa 1 unidade no valor do contador de envase.
      }
      Serial.print("Números de recipiente cheio = ");  // Imprime o texto no monitor serial.
      Serial.println(Contador);                        // Imprime o texto no monitor serial.
    }
    // zera valor do contador de envase.
    if (Recebido == 'z') {
      Contador = 0;                                    // zera valor do contador de envase.
      Serial.print("Números de recipiente cheio = ");  // Imprime o texto no monitor serial.
      Serial.println(Contador);                        // Imprime o texto no monitor serial.
    }
  }
}