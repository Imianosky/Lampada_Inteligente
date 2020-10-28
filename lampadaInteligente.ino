/*
   Arquivo: lampada_inteligente.ino
   Autores: Carolina Imianosky e Natasha Pereira
   Função do arquivo:
   Criado em 14 de outubro de 2020
   Modificado em 27 de outubro de 2020
*/


#include "lampada_inteligente.h"

bool pirActivated = false;  // PIR foi/nao foi ativado


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


/*
  @brief: controle do input timer no BlynkApp
*/
BLYNK_WRITE(V1) {

  TimeInputParam t(param); // armazena o valor inserido no app

  // Caso nao tenha funcionamento programado:
  if (t.hasStartTime() == 0 && t.hasStopTime() == 0) {
    scheduledTime = false;
  }

  // Caso tenha acionamento programado:
  if (t.hasStartTime()) {
    startHour = t.getStartHour();
    startMin  = t.getStartMinute();
    startSec  = t.getStartSecond();
    scheduledTime = true;
  }


  // Caso tenha desligamento programado:
  if (t.hasStopTime())
  {
    stopHour = t.getStopHour();
    stopMin  = t.getStopMinute();
    stopSec  = t.getStopSecond();
    scheduledTime = true;
  }


  // para cada dia da semana:
  for (int i = 1; i <= 7; i++) {
    if (t.isWeekdaySelected(i)) {
      week[i] = 1;
    }
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
}


void loop() {

  Blynk.run();
  setLED();
  delay(5);

}


void setLED() {

  localTime();

#ifdef SCHEDULED_TIME_DEBUG
  Serial.println(scheduledTime);
#endif

  if (scheduledTime == true) {

    // desativa a interrupcao do PIR
    detachInterrupt(PIRPIN);

    // caso esteja no horario programado p ligar
    if ((startHour == hourNow) && (startMin == minNow)
        && (week[wDayNow])     && (startSec == secNow)
        && (LEDState == 0)) {

      LEDState = 1;
      digitalWrite(LEDPIN, LEDState);    // acende o LED
      Blynk.virtualWrite(V0, LEDState);  // ativa o botao virtual
      virtualButton = LEDState;   // ativa a variavel do botao virtual
    }

    // caso esteja no horario programado p ligar
    if ((stopHour == hourNow) && (stopMin == minNow)
        && (week[wDayNow]) && (stopSec == secNow)
        && (LEDState == 1)) {

      LEDState = 0;
      digitalWrite(LEDPIN, LEDState);   // apaga o LED
      Blynk.virtualWrite(V0, LEDState); // desativa o botao virtual
      virtualButton = LEDState;   // ativa a variavel do botao virtual
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

    // caso o led esteja ativo e tenha alta luminosidade
    if (LEDState == 1 && lightVal > 2800) {

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
      }
    }

    // caso o led esteja apagado, tenha baixa luminosidade e o pir esteja ativo
    if (LEDState == 0 && lightVal < 2000 && pirActivated == true) {

      /* Desativa a variavel p evitar que entre na verificacao com a
          mesma ativacao do PIR */
      pirActivated = false;

      LEDState = 1;
      Blynk.virtualWrite(V0, LEDState);     // botao virtual = ON
      virtualButton = 1;
      digitalWrite(LEDPIN, LEDState);       // acende o LED
    }

    // seta o led de acordo com o botao do app
    LEDState = virtualButton;
    digitalWrite(LEDPIN, LEDState);
  }
}
