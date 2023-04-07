#include "main.h"

int main(int argc, char **argv)
{
	int ret;
	openlog("tuya_iot_sender", LOG_PID|LOG_CONS, LOG_USER);
	
	ret = ubus_init(argc, argv);
	if(ret != 0)
		syslog(LOG_ERR, "Ubus init error");

	closelog();

	return 0;
}