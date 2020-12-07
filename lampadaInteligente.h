/*
   Arquivo: lampada_inteligente.h
   Autores: Carolina Imianosky e Natasha Pereira
   Função do arquivo:
   Criado em 14 de outubro de 2020
   Modificado em 27 de outubro de 2020
*/


#ifndef LAMPADA_INTELIGENTE_H
#define LAMPADA_INTELIGENTE_H


#include <BlynkSimpleEsp32.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <SPI.h>
#include <esp_task_wdt.h>
#include <SoftwareSerial.h>
#include<time.h> 


#define BLYNK_PRINT Serial


#define INACTIVE_TIME 20000       // tempo de inatividade no pir


#define LEDPIN 2                // LED
#define PIRPIN 35               // PIR
#define LDRPIN 34               // LDR


#undef PIR_DEBUG
#undef LDR_DEBUG
#undef SCHEDULED_TIME_DEBUG


BlynkTimer timer;

const char* ntpServer         = "pool.ntp.org";       // servidor NTP
const long  gmtOffsetSec      = -10800;               // definição fuso horario
const int   daylightOffsetSec = 0;                   
struct tm   h;                                        // struct de horário atual
int         hourNow, minNow, secNow, wDayNow;         // variaveis separadas p cada valor
char message[]    = "";

char auth[] = "KjRjRY08vK7yx-8ZGu26c7vg6uqHyjZM";     // autenticacao BlynkApp
char ssid[] = "Carol";                                // autenticacao wifi
char pass[] = "di020927";                                    


int           lightVal;                               // valor no LDR
int           virtualButton;                                     // valor do botao virtual on/off
int           LEDState = 0;                           // estado atual do LED

bool          scheduledTime   = false;                // possui/nao possui horário programado
bool          buttonPressed   = false;                // botao foi/nao foi pressionado


int           startHour = -1,                         // hora, min e seg programados para
              startMin  = -1,                         // desligar e para ligar o led
              stopHour  = -1, 
              stopMin   = -1, 
              startSec  = -1, 
              stopSec   = -1;


unsigned long buttonActivateTime;                     // momento de ativação do botao
unsigned long pirActivateTime = 0;                    // momento de ativacao do PIR
unsigned long changeTime      = 0;                    // momento de mudança automatica do LED


#endif /*LAMPADA_INTELIGENTE_H*/
