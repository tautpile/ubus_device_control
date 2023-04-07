struct info_struct {
	char ttyusb[50];
	int pid;
	int vid;
};

int serial_send_data(int on_off, int pin_number, char* portnr);
int ubus_init(int argc, char **argv);
void device_info_port(struct info_struct **s);

