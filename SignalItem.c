/*
htop - SignalItem.c
(C) 2004-2006 Hisham H. Muhammad
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include "SignalItem.h"
#include "String.h"
#include "Object.h"
#include "RichString.h"
#include <string.h>
#include <signal.h>

#include "debug.h"

/*{

typedef struct Signal_ {
   Object super;
   char* name;
   int number;
} Signal;

}*/

#ifdef DEBUG
char* SIGNAL_CLASS = "Signal";
#else
#define SIGNAL_CLASS NULL
#endif

static void Signal_delete(Object* cast) {
   Signal* this = (Signal*)cast;
   assert (this != NULL);
   free(this->name);
   free(this);
}

static void Signal_display(Object* cast, RichString* out) {
   Signal* this = (Signal*)cast;
   assert (this != NULL);
   
   char buffer[31];
   snprintf(buffer, 30, "%2d %s", this->number, this->name);
   RichString_write(out, CRT_colors[DEFAULT_COLOR], buffer);
}

static Signal* Signal_new(int number) {
   Signal* this = malloc(sizeof(Signal));
   Object_setClass(this, SIGNAL_CLASS);
   ((Object*)this)->display = Signal_display;
   ((Object*)this)->delete = Signal_delete;
   if (number)
   {
       const char * name = sys_signame[number];
       char * c;
       this->name = malloc(strlen(name) + 4);
       sprintf(this->name, "SIG%s", name);
       for (c = this->name + 3; *c; ++c)
           *c = toupper(*c);
   }
   else
   {
       this->name = strdup("Cancel");
   }
   this->number = number;
   return this;
}

int Signal_getSignalCount() {
   return NSIG;
}

Signal** Signal_getSignalTable() {
   Signal** signals = malloc(sizeof(Signal*) * NSIG);
   unsigned int sig;
   signals[0] = Signal_new(0);
   for (sig = 1; sig < NSIG; ++sig)
       signals[sig] = Signal_new(sig);
   return signals;
}
