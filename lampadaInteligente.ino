/*
   Arquivo: lampada_inteligente.ino
   Autores: Carolina Imianosky e Natasha Pereira
   Função do arquivo:
   Criado em 14 de outubro de 2020
   Modificado em 27 de outubro de 2020
*/


#include "lampadaInteligente.h"



bool pirActivated    = false;                // PIR foi/nao foi ativado



/*
   @brief: coleta o horario atual e armazena separadamente em cada variavel
*/
void localTime() {

  // caso consiga obter o valor do servidor:
  if (getLocalTime(&h)) {
    hourNow = h.tm_hour;
    minNow  = h.tm_min;
    secNow  = h.tm_sec;
    wDayNow = h.tm_wday;

  } else {
    Serial.println("Failed to obtain time");
    return;
  }
}


/*
   @brief: detecta movimento atraves do sensor PIR.
*/
void IRAM_ATTR detectsMovement() {

#ifdef PIR_DEBUG
  Serial.println("MOTION DETECTED!!!");
#endif

  pirActivateTime = millis();   // momento de ativacao do PIR
  pirActivated = true;          // variavel de ativacao do PIR
}


/*
   @brief: controle do botao virtual on/off
*/
BLYNK_WRITE(V0) {

  virtualButton = param.asInt();              // virtualButton recebe o valor do botao no app
  buttonActivateTime = millis();   // momento em que o botão foi pressionado
  buttonPressed = true;            // variavel de ativacao do botão

}

void setAppTime() {
  localTime();

  String aux;
  aux = "";
  if (hourNow < 10) {
    aux += "0";
  }
  aux += (hourNow);
  aux += " : ";
  if (minNow < 10)
    aux += "0";
  aux += minNow;
  aux += " : ";
  if (secNow < 10)
    aux += "0";
  aux += secNow;

  Blynk.virtualWrite(V14, aux);

  Blynk.virtualWrite(V21, lightVal);

  if ((millis() - pirActivateTime) < INACTIVE_TIME) {
    Blynk.virtualWrite(V20, 1);
  } else {
    Blynk.virtualWrite(V20, 0);
  }

  if (millis() - changeTime > 5000)
    Blynk.virtualWrite(V15, " ");

}




/*
  @brief: controle do input timer no BlynkApp
*/
BLYNK_WRITE(V1) {

  TimeInputParam t(param); // armazena o valor inserido no app

  // Caso nao tenha funcionamento programado:
  if (t.hasStartTime() == 0 && t.hasStopTime() == 0) {
    scheduledTime = false;
    Blynk.virtualWrite(V2, 0);
  }

  // Caso tenha acionamento programado:
  if (t.hasStartTime()) {
    startHour = t.getStartHour();
    startMin  = t.getStartMinute();
    startSec  = t.getStartSecond();
    scheduledTime = true;
    Blynk.virtualWrite(V2, 255);
  }


  // Caso tenha desligamento programado:
  if (t.hasStopTime())
  {
    stopHour = t.getStopHour();
    stopMin  = t.getStopMinute();
    stopSec  = t.getStopSecond();
    scheduledTime = true;
    Blynk.virtualWrite(V2, 255);
  }
}


void setup() {

  // pinMode do LED e do PIR
  pinMode(LEDPIN, OUTPUT);
  pinMode(PIRPIN, INPUT_PULLUP);

  // baud rate
  Serial.begin(115200);
  delay(10);

  // conexao com wifi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
  Blynk.begin("KjRjRY08vK7yx-8ZGu26c7vg6uqHyjZM", ssid, pass);


  // configuracao do horário atual
  configTime(gmtOffsetSec,
             daylightOffsetSec,
             ntpServer);

  timer.setInterval(1000L, setAppTime);
  timer.setInterval(1000L, setLED);

}


void loop() {

  Blynk.run();
  timer.run();

}


void setLED() {
#ifdef SCHEDULED_TIME_DEBUG
  localTime();
  Serial.println(&h, "%A, %B %d %Y %H:%M:%S");
#endif

  lightVal = 0;   // zera o valor do LDR

  if (scheduledTime == true) {

    // desativa a interrupcao do PIR
    detachInterrupt(PIRPIN);

    localTime();

    // caso esteja no horario programado p ligar
    if ((startHour == hourNow) 
        && (startMin == minNow)
        && (startSec == secNow)
        && (LEDState == 0)) {

      LEDState = 1;
      digitalWrite(LEDPIN, LEDState);    // acende o LED
      Blynk.virtualWrite(V0, LEDState);  // ativa o botao virtual
      virtualButton = LEDState;   // ativa a variavel do botao virtual
      Blynk.virtualWrite(V15, "Acendendo: horário programado");
      changeTime = millis();
    }

    // caso esteja no horario programado p ligar
    if ((stopHour == hourNow) 
        && (stopMin == minNow)
        && (stopSec == secNow)
        && (LEDState == 1)) {

      LEDState = 0;
      digitalWrite(LEDPIN, LEDState);   // apaga o LED
      Blynk.virtualWrite(V0, LEDState); // desativa o botao virtual
      virtualButton = LEDState;   // ativa a variavel do botao virtual
      Blynk.virtualWrite(V15, "Desligando: horário programado");
      changeTime = millis();
    }

    // seta o led de acordo com o botao do app
    LEDState = virtualButton;
    digitalWrite(LEDPIN, LEDState);

  } else {

    // ativa interrupcao caso o PIR seja ativado
    attachInterrupt(digitalPinToInterrupt(PIRPIN), detectsMovement, RISING);

    lightVal = analogRead(LDRPIN);  // lê do ldr

#ifdef LDR_DEBUG
    Serial.println(lightVal);
#endif

    if (lightVal > 2800) {
      // caso o led esteja ativo e tenha alta luminosidade
      if (LEDState == 1) {

        // tempo desde a ultima ativacao do PIR:
        int pirInactivityTime = (millis() - pirActivateTime);
        // tempo desde a ultima ativacao do botao virtual
        int buttonInactivityTime = (millis() - buttonActivateTime);

        // verifica o tempo inativo
        if ((INACTIVE_TIME < pirInactivityTime)
            && (INACTIVE_TIME < buttonInactivityTime)) {

          LEDState = 0;
          Blynk.virtualWrite(V0, LEDState);   // botao virtual = OFF
          virtualButton = 0;
          digitalWrite(LEDPIN, LEDState);     // apaga o LED
          Blynk.virtualWrite(V15, "Desligando: sem movimento e bastante luz");
          changeTime = millis();

        }
      }
    } else if (lightVal < 2000) {

      // caso o led esteja apagado e o pir esteja ativo
      if (LEDState == 0 && pirActivated == true && (millis() - pirActivateTime) < INACTIVE_TIME) {

        /* Desativa a variavel p evitar que entre na verificacao com a
            mesma ativacao do PIR */
        pirActivated = false;
        LEDState = 1;
        Blynk.virtualWrite(V0, LEDState);     // botao virtual = ON
        virtualButton = 1;
        digitalWrite(LEDPIN, LEDState);       // acende o LED
        Blynk.virtualWrite(V15, "Acendendo: movimentação e pouca luz");
        changeTime = millis();
      }
    }

    // seta o led de acordo com o botao do app
    LEDState = virtualButton;
    digitalWrite(LEDPIN, LEDState);
  }
}
