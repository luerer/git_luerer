TARGET = client server
OBJS = DList.o Mlist.o getfile.o ini.o mmt.o ringq.o send.o mkdir.o ../mpu/stream.o ../mpu/mp4.o
OBJ1 = client.o mmtp.o
OBJ2 = server.o

CFLAGS = -I../../../ -D_TEST
LDFLAGS = -lm -lpthread

all: $(TARGET)

client: $(OBJS) $(OBJ1)
	$(CC) -o $@ $(OBJS) $(OBJ1) $(LDFLAGS)

server: $(OBJS) $(OBJ2)
	$(CC) -o $@ $(OBJS) $(OBJ2) $(LDFLAGS)

%.o:%.c
	$(CC) -g $(CFLAGS) -o $@ -c $<

%.o:%.cc
	$(CC) -g $(CFLAGS) -o $@ -c $<
	
clean:
	@rm -f $(OBJS) $(OBJ1) $(OBJ2)

distclean: clean
	@rm -f $(TARGET)
