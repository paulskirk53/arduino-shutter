#pragma once
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <AccelStepper.h>

void open_shutter();
void close_shutter();
void flap_close_process();
void flap_open_process();
void initialise_relays() ;
void PowerOn();
void PowerOff();