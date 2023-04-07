BIN:=port_control
SRCS:=$(wildcard *.c)
OBJS:=$(SRCS:.c=.o)
LDFLAGS:=-lserialport -lblobmsg_json -lubox -lubus 

CFLAGS:=-Wall

.PHONY: all clean

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) 

clean:
	$(RM) $(BIN) $(OBJS)
