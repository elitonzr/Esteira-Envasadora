unsigned long startTimeBomba;
unsigned long currentTimeBomba;
    
    if (digitalRead(Esteira) == DESLIGA && RecipienteCheio == false && RecipientePosicionado) {
      if (!BombaEstado && digitalRead(Bomba) == DESLIGA) {
        startTimeBomba = millis();
        Serial.print("\tEnvasamento iniciado\t");  // Imprime o texto no monitor serial.
        Serial.print("tempo: ");                   // Imprime o texto no monitor serial.
        Serial.print(TempoEnvase / 1000);          // Imprime o texto no monitor serial.
        Serial.println("s");
      } else {
        if (millis() - startTimeBomba <= TempoEnvase) {
          // Serial.print("TempoEnvase: ");                      // Imprime o texto no monitor serial.
          // Serial.print(TempoEnvase);                          // Imprime o texto no monitor serial.
          // Serial.print("\tstartTimeBomba: ");                 // Imprime o texto no monitor serial.
          // Serial.print(startTimeBomba);                       // Imprime o texto no monitor serial.
          // Serial.print("\tcurrentTimeBomba: ");               // Imprime o texto no monitor serial.
          // Serial.print(millis());                     // Imprime o texto no monitor serial.
          Serial.print("Tempo decorrido: ");                   // Imprime o texto no monitor serial.
          Serial.println((millis() - startTimeBomba) / 1000);  // Imprime o texto no monitor serial.
        } else {
          BombaEstado = false;
          digitalWrite(Bomba, DESLIGA);
          Serial.print("Recipiente cheio! ");  // Imprime o texto no monitor serial.
          RecipienteCheio = true;              // Confirar recepiente cheio.
          Contador++;                          // Increenta contador.
          Serial.print("Total = ");            // Imprime o texto no monitor serial.
          Serial.println(Contador);            // Imprime o texto no monitor serial.
          delay(1000);                         // Aguarda 1s para iniciar movimento da esteira.
        }
      }
      BombaEstado = true;
      digitalWrite(Bomba, LIGA);  // Liga bomba de envase.

    } else {
      digitalWrite(Bomba, DESLIGA);
      BombaEstado = false;
    }