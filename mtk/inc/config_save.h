#ifndef __CONFIG_SAVE_H__
#define __CONFIG_SAVE_H__

#include "gm_type.h"
#include "error_code.h"
#include "socket.h"
#include "applied_math.h"


extern GM_ERRCODE config_service_read_from_local(void);

extern GM_ERRCODE config_service_save_to_local(void);

extern void goome_del_param_file(void);


#endif


