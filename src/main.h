#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <syslog.h>
#include <libubox/blobmsg_json.h>
#include <libubus.h>
#include <libserialport.h>
#include <sys/types.h>
#include "ubus.h"