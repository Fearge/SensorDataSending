#pragma once
#include "HX71708_ADC.h"

void resetAllADCsSync(HX71708_ADC* adcs[], int count);

bool all_ready(HX71708_ADC* adcs[], int count);

void readAllADCsSync(HX71708_ADC* adcs[], long werte[], int count);

void readAllGramsSync(HX71708_ADC* adcs[], float grams[], int count);

void tareAll(HX71708_ADC* adcs[], int count);
