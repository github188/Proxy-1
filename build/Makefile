
SRCDIR = ..

CXX = g++
LINK = g++
CXXFLAGS = -g -Wall -DKISE_DEBUG
LIBS = -lpthread 
INCPATH  = -I$(SRCDIR) -I$(SRCDIR)/include

OBJS = main.o config.o ConfFile.o \
	   AbstractThread.o Buffer.o jsiomodel.o \
	   Network.o Session.o TaskDispatcher.o \
	   ProxyTaskDispatcher.o Log.o dir.o file.o

TARGET =	NetworkProxy

$(TARGET):	$(OBJS)
	$(LINK) $(OBJS) $(LIBS) -o $(TARGET) 
	
clean:
	rm -f $(OBJS) $(TARGET) 


##  compile 
config.o : $(SRCDIR)/config/config.cpp $(SRCDIR)/config/config.h $(SRCDIR)/config/ConfFile.h
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<

ConfFile.o : $(SRCDIR)/config/ConfFile.cpp $(SRCDIR)/config/ConfFile.h
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<

main.o: $(SRCDIR)/proj/main.cpp
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<

ProxyTaskDispatcher.o: $(SRCDIR)/proj/ProxyTaskDispatcher.cpp $(SRCDIR)/proj/ProxyTaskDispatcher.h
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<

# ======== old =====
AbstractThread.o: $(SRCDIR)/Thread/AbstractThread.cpp $(SRCDIR)/Thread/AbstractThread.h
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<
	
Buffer.o: $(SRCDIR)/Network/Buffer.cpp $(SRCDIR)/Network/Buffer.h
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<

jsiomodel.o: $(SRCDIR)/Network/jsiomodel.cpp $(SRCDIR)/Network/jsiomodel.h
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<

Network.o: $(SRCDIR)/Network/Network.cpp $(SRCDIR)/Network/Network.h
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<

Session.o: $(SRCDIR)/Network/Session.cpp $(SRCDIR)/Network/Session.h 
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<

Log.o: $(SRCDIR)/Log/Log.cpp $(SRCDIR)/Log/Log.h
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<

TaskDispatcher.o: $(SRCDIR)/Network/TaskDispatcher.cpp $(SRCDIR)/Network/TaskDispatcher.h 
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<

dir.o: $(SRCDIR)/Tool/dir.cpp $(SRCDIR)/Tool/dir.h
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<

file.o: $(SRCDIR)/Tool/file.cpp $(SRCDIR)/Tool/file.h
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<
	
package.o: package.cpp package.h
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<
	
MsgQueue.o: MsgQueue.cpp MsgQueue.h
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<
	
	
tools.o: Tool/tools.cpp Tool/tools.h
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<
	
	
CoreDump.o: CoreDump/CoreDump.cpp CoreDump/CoreDump.h
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<
	
	
Heartbeat.o: Heartbeat.cpp Heartbeat.h
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<
