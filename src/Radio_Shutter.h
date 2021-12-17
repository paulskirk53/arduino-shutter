#pragma once
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <AccelStepper.h>

void dogkick();
void open_shutter();
void close_shutter();
void flap_close_process();
void flap_open_process();
void initialise_relays() ;
void PowerOn();
void PowerOff();
void Check_if_Raining();
void checkPowerOnTime();