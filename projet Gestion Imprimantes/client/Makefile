TARGET=client.out

SOURCES= client.c ../communication/communication.c

OBJECTS=$(SOURCES:.c=.o)

CC=gcc
CXXFLAGS+= -std=c99 -g -Wall 

$(TARGET):  $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ -lpthread

.cpp.o:
	$(CC) $(CXXFLAGS) -c $<

clean:
	rm -f $(TARGET)  $(OBJECTS) *~

tmp_serveur.o: ../communication/communication.h ../utils/utils.h