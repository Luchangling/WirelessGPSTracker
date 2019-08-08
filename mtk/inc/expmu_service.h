
#ifndef __EXPMU_SERVICE_H__
#define __EXPMU_SERVICE_H__

#include "gm_type.h"
#include "error_code.h"


extern GM_ERRCODE expmu_service_creat(void);

extern GM_ERRCODE expmu_service_timer_proc(void);

extern GM_ERRCODE expmu_service_destory(void);

extern u8 does_the_device_need_into_lpm(void);

extern u16 expmu_service_get_report_interval(void);

extern bool expmu_service_is_working(void);

#define TRACK_MODE_NEED_INTO_SLEEP_INTERVAL 6

#endif

