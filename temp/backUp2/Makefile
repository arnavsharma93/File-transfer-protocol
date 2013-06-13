CC=gcc
CFLAGS = -g  
# uncomment this for SunOS
 LIBS = -lssl -lcrypto 

all: app

app: app.o 
	$(CC) -o app app.o $(LIBS)


app.o: app.c port.h


clean:
	rm -f app app.o
