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

int CPUTemperatureMeter_attributes[] = {
   CPU_TEMP
};

static void CPUTemperatureMeter_setValues(Meter* this, char* buffer, int size) {
   double temp;

   temp = SMCGetTemperature(SMC_KEY_CPU_TEMP, CELSIUS);
   if (temp > -100 && temp < 900) {
      snprintf(buffer, size, "%.1f °C", temp);
   } else {
      snprintf(buffer, size, "-.- °C");
   }
}

MeterType CPUTemperatureMeter = {
   .setValues = CPUTemperatureMeter_setValues,
   .display = NULL,
   .mode = TEXT_METERMODE,
   .total = 100.0,
   .items = 1,
   .attributes = CPUTemperatureMeter_attributes,
   .name = "CPU Temp",
   .uiName = "CPU Temp",
   .caption = "Temp: ",
};
