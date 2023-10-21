#ifndef __ATOMIC_OP_H__
#define __ATOMIC_OP_H__
//----------------------------------------------------------------------------------------------------------------------
#ifdef FREERTOS
// include task.h from FreeRTOS implementation
#include "FreeRTOS.h"
#include "task.h" 
#define a_store(x, v) { taskENTER_CRITICAL(); x = (v); taskEXIT_CRITICAL(); }
#define a_load(x) ({ taskENTER_CRITICAL(); typeof(x) _x = (x); taskEXIT_CRITICAL(); _x; })
#else
#define a_store(x, v) { x = (v); }
#define a_load(x) ({ typeof(x) _x = (x); _x; })
#endif
//----------------------------------------------------------------------------------------------------------------------
#endif /* __ATOMIC_OP_H__ */
