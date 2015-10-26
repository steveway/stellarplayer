#ifndef __DEBUGHLPR_H__
#define __DEBUGHLPR_H__

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "driverlib/interrupt.h"
#include "utils/uartstdio.h"
#include "s3m32.h"


#define CHECKMEMORY_E {}

#define HALT_HERE {IntMasterDisable(); UARTprintf("validation failed \n"); while(1){}}

#endif
