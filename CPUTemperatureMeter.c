/*
htop
(C) 2004-2006 Hisham H. Muhammad
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include "CPUTemperatureMeter.h"
#include "Meter.h"

#include <time.h>

#include "debug.h"
#include "smc.h"

#define KELVIN     0
#define CELSIUS    1
#define FAHRENHEIT 2

int CPUTemperatureMeter_attributes[] = {
   CPU_TEMP
};

static void CPUTemperatureMeter_setValues(Meter* this, char* buffer, int size) {
   double temp;

   temp = SMCGetTemperature(SMC_KEY_CPU_TEMP); 
   this->values[0] = temp;
}

static const char * scale_markers = "KCF";

static const double scales[][2] = {
    {1.0,  273.15}, /* K */
    {1.0,    0.0},  /* C */
    {1.8,   32.0}   /* F */
};

double celcius_to_scale(double temp, int scale) {
    return temp * scales[scale][0] + scales[scale][1];
}

static void CPUTemperatureMeter_display(Object* cast, RichString* out) {
   Meter* this = (Meter*)cast;
   char buffer[20];
   int scale = CELSIUS; /* change this to your favourite scale */
   double hot_threshold = 70.0; /* *C */
   double warm_threshold = 59.0; /* *C */ 
   double raw_temp, converted_temp;

   RichString_init(out);
   
   raw_temp = this->values[0];

   if (raw_temp > -100 && raw_temp < 900) {
      converted_temp = celcius_to_scale(raw_temp, scale);
      snprintf(buffer, 20, "%.1f %c", converted_temp, scale_markers[scale]);
       if (raw_temp > hot_threshold) {
         RichString_append(out, CRT_colors[CPU_TEMP_HOT], buffer);
       } else if (raw_temp > warm_threshold) {
         RichString_append(out, CRT_colors[CPU_TEMP_WARM], buffer);
       } else {
         RichString_append(out, CRT_colors[CPU_TEMP_NORMAL], buffer);
       }
   } else {
      snprintf(buffer, 20, "-.- %c", scale_markers[scale]);
      RichString_append(out, CRT_colors[CPU_TEMP_NORMAL], buffer);
   }
}

MeterType CPUTemperatureMeter = {
   .setValues = CPUTemperatureMeter_setValues,
   .display = CPUTemperatureMeter_display,
   .mode = TEXT_METERMODE,
   .total = 100.0,
   .items = 1,
   .attributes = CPUTemperatureMeter_attributes,
   .name = "CPUTemp",
   .uiName = "CPU Temp",
   .caption = "CPU Temp: ",
};
