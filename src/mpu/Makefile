TARGET = parser
OBJS = stream.o mp4.o test_parser.o
CFLAGS = -I../../../
LDFLAGS = -lm

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

%.o:%.c
	$(CC) -g $(CFLAGS) -o $@ -c $<

%.o:%.cc
	$(CC) -g $(CFLAGS) -o $@ -c $<
	
clean:
	@rm -f $(OBJS)

distclean: clean
	@rm -f $(TARGET)
