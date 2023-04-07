#include "main.h"

static int pin_on(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg);

static int device_get(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg);

enum {
	PORT_NAME,
        PIN_NUMBER,
	PIN_STATE,
	__COUNTER_MAX
};

static const struct blobmsg_policy pin_policy[] = {
	[PIN_NUMBER] = { .name = "pin", .type = BLOBMSG_TYPE_INT32 },
	[PORT_NAME] = {.name = "port", .type = BLOBMSG_TYPE_STRING },
	[PIN_STATE] = {.name = "state", .type = BLOBMSG_TYPE_INT32}
};

static const struct ubus_method devicectrl_methods[] = {
        UBUS_METHOD_NOARG("device_get", device_get),
        UBUS_METHOD("pin", pin_on, pin_policy),
};

static struct ubus_object_type devicectrl_object_type =
	UBUS_OBJECT_TYPE("devicectrl", devicectrl_methods);

static struct ubus_object devicectrl_object = {
	.name = "devicectrl",
	.type = &devicectrl_object_type,
	.methods = devicectrl_methods,
	.n_methods = ARRAY_SIZE(devicectrl_methods),
};

static int pin_on(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	int pin_number, on_off = 1;
	int pin_numbers[9] = {0, 2, 4, 5, 12, 13, 14, 15, 16};
	int i = 0, a = 1;
	int msg_state = 0, msg_port = 0, msg_pin = 0; 
	int error_num = 1;

	char *port_name, *ret;
	char buffer[128];
	const char *name = port_name;

	struct blob_attr *tb[__COUNTER_MAX];
	struct blob_buf b = {};
	struct info_struct *s;

	void *table;
	void *array;
	
	blob_buf_init(&b, 0);
	blobmsg_parse(pin_policy, __COUNTER_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[PIN_NUMBER])
		return UBUS_STATUS_INVALID_ARGUMENT;

	device_info_port(&s);

	on_off = blobmsg_get_u32(tb[PIN_STATE]);
	pin_number = blobmsg_get_u32(tb[PIN_NUMBER]);
	port_name = blobmsg_get_string(tb[PORT_NAME]);

 	for(i = 0; i < 9; i++) {
		if(pin_numbers[i] == pin_number)
			msg_pin = 1;
	}

	if(on_off != 0 && on_off != 1) {
        	msg_state = 1;
	}

	array = blobmsg_open_array(&b, "messages");
	
	for(i = 0; i < 5; i++) {
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%s", s[i].ttyusb);
		a = strcmp(buffer, port_name);
		if (a == 0) {
			msg_port = 1;
		}
	}

	if(msg_port == 0) {
		table = blobmsg_open_table(&b, name);
		sprintf(buffer, "invalid port:%s", port_name);
		blobmsg_add_string(&b, "msg", buffer);
		blobmsg_close_table(&b, table);
	}

	if(msg_state == 1) {
		
		table = blobmsg_open_table(&b, name);
		sprintf(buffer, "Invalid device pin state: %d", on_off);
		blobmsg_add_string(&b, "msg", buffer);
		blobmsg_close_table(&b, table);
	}

	if(msg_pin == 0) {
		table = blobmsg_open_table(&b, name);
		sprintf(buffer, "invalid pin number:%d",pin_number);
		blobmsg_add_string(&b, "msg", buffer);
		blobmsg_close_table(&b, table);
	} else if ((msg_port != 0) && (msg_pin != 0) && (msg_state != 1 )) {
		ret = serial_send_data(on_off, pin_number, port_name);
		if(ret != 0)
			blobmsg_add_string(&b, "error", "serial send error");

		table = blobmsg_open_table(&b, name);
		blobmsg_add_string(&b, "msg", "success");
		blobmsg_close_table(&b, table);
		error_num = 0;
	} 

	if(error_num == 1) {
		table = blobmsg_open_table(&b, name);
		blobmsg_add_u32(&b, "error", 1);
		blobmsg_close_table(&b, table);
	} else {
		table = blobmsg_open_table(&b, name);
		blobmsg_add_u32(&b, "error", 0);
		blobmsg_close_table(&b, table);
	}

	blobmsg_close_array(&b, array);
	free(s);
	ubus_send_reply(ctx, req, b.head);
	blob_buf_free(&b);
	return 0;
}

void device_info_port(struct info_struct **s)
{
	int i = 0, n = 0;
	int sizear = 1;

	struct sp_port **port_list;

	enum sp_return result = sp_list_ports(&port_list);
	if (result != SP_OK) 
		syslog(LOG_ERR, "sp_list_ports() failed!\n");

	*s = (struct info_struct*)malloc(sizeof(struct info_struct));

	for(i = 0; port_list[i] != NULL; i++) {
		int usb_vid = 0, usb_pid = 0;
		struct sp_port *port = port_list[i];
		char *port_name = sp_get_port_name(port);
		
		if ((port_name[8] == 'U') && (port_name[9] == 'S')) {
			if(n > 0) {
				sizear++;
				*s = (struct info_struct*)realloc(*s, sizeof(struct info_struct) * sizear);
			}
			strcpy((*s)[n].ttyusb, port_name);
			sp_get_port_usb_vid_pid(port, &usb_vid, &usb_pid);
			(*s)[n].pid = usb_pid;
			(*s)[n].vid = usb_vid;
			n++;
		}
	}
	sp_free_port_list(port_list);
}

static int device_get(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	int i = 0;
	struct info_struct *s;
	struct blob_buf b = {0};
	const char *name = "table";
	char buffer[20];
	void *table;
	void *array;

	device_info_port(&s);
	blob_buf_init(&b, 0);
	array = blobmsg_open_array(&b, "device_list");

	if(strncmp((s)[0].ttyusb, "/dev/", 5) != 0) {
		table = blobmsg_open_table(&b, name);
		blobmsg_add_string(&b, "error", "No devices connected");
		blobmsg_close_table(&b, table);
	} else { 
		for(i = 0; ((s)[i].pid) == 60000; i++) {
			table = blobmsg_open_table(&b, name);
			blobmsg_add_string(&b, "port", (s)[i].ttyusb);
			sprintf(buffer, "%04X", s[i].pid);
			blobmsg_add_string(&b, "pid", buffer);
			memset(buffer, 0, sizeof(buffer));
			sprintf(buffer, "%04X", s[i].vid);
			blobmsg_add_string(&b, "vid", buffer);
			memset(buffer, 0, sizeof(buffer));
			blobmsg_close_table(&b, table);
		}
	}

	blobmsg_close_array(&b, array);
	free(s);
	ubus_send_reply(ctx, req, b.head);
	blob_buf_free(&b);
	return 0;
}

int serial_send_data(int on_off, int pin_number, char* portnr)
{
	int fd, len;
        char buff[100];
	char text[255];
	char *port_place = portnr;
	struct termios options; 

	fd = open(port_place, O_RDWR | O_NDELAY | O_NOCTTY);
	if (fd < 0) {
		syslog(LOG_ERR, "Error opening serial");
	}

        options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;

        tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &options);
    
	if(on_off == 1) {
		snprintf(buff, 100, "{\"action\":\"on\", \"pin\": %d}", pin_number);
	} else {
		snprintf(buff, 100, "{\"action\":\"off\", \"pin\": %d}", pin_number);
	}

        strcpy(text, buff);
	len = strlen(text);
	len = write(fd, text, len);

	sleep(2);
	memset(text, 0, 255);
	len = read(fd, text, 255);
	close(fd);
	return 0;
}

int ubus_init(int argc, char **argv)
{
	struct ubus_context *ctx;
	uloop_init();

	ctx = ubus_connect(NULL);
	if (!ctx) {
		syslog(LOG_ERR, "Failed to connect to ubus\n");
		return -1;
	}

	ubus_add_uloop(ctx);
	ubus_add_object(ctx, &devicectrl_object);
	uloop_run();
	ubus_free(ctx);
	uloop_done();
	return 0;
}