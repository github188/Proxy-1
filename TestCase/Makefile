CXXFLAGS =	-g -Wall -fmessage-length=0

OBJS =		package.o \
			client.o
			
#LIBS = `pkg-config --libs libevent libevent_pthreads` -lpthread
LIBS = /usr/local/libevent/lib/libevent.a /usr/local/libevent/lib/libevent_pthreads.a -lpthread -lrt
INCPATH  = `pkg-config --cflags libevent`


CLIENT =	client

$(CLIENT): $(OBJS)
	$(CXX) -o client $(OBJS) $(LIBS)
	
all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) client.o client *.jpeg

client.o: client.cpp
	g++ -c $(INCPATH) $(CXXFLAGS) client.cpp
	
package.o: ../package.cpp ../package.h
	g++ -c $(INCPATH) $(CXXFLAGS) ../package.cpp
	
