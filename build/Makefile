
SRCDIR = ..

CXX = g++
LINK = g++
CXXFLAGS = -g -Wall #-DKISE_DEBUG
LIBS = -lpthread 
INCPATH  = -I$(SRCDIR) -I$(SRCDIR)/include

BackupOBJS = BackupModule.o BackupUtils.o \
			 ClearDirThread.o Database.o DataCache.o \
			 DBIOThread.o ResumeTransfer.o \
			 RotationBackup.o RotationFullBackup.o \
			 timeclass.o stltool.o

OBJS = main.o config.o ConfFile.o \
	   AbstractThread.o Buffer.o jsiomodel.o \
	   Network.o Session.o TaskDispatcher.o \
	   ProxyTaskDispatcher.o Log.o dir.o file.o \
	   ReconnectThread.o Convert.o

CONFIG_BACKUP ?= n
ifeq "$(CONFIG_BACKUP)" 'y'
OBJS += $(BackupOBJS)
LIBS += -lsqlite3
CXXFLAGS += -DCONFIG_BACKUP
endif

ifeq "$(CONFIG_MONITOR)" 'y'
LIBS += -lKiseMonitor 
CXXFLAGS += -DCONFIG_MONITOR
endif

CVT_RULE += -DCONVERT_ZJ
CXXFLAGS += $(CVT_RULE)
OBJS += Convert_zj.o

TARGET =	NetworkProxy

$(TARGET):	$(OBJS)
	$(LINK) $(OBJS) $(LIBS) -o $(TARGET) 
	
clean:
	rm -f $(OBJS) $(TARGET) 


##  compile 
timeclass.o : $(SRCDIR)/Tool/timeclass.cpp $(SRCDIR)/Tool/timeclass.h
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<

stltool.o : $(SRCDIR)/Tool/stltool.cpp $(SRCDIR)/Tool/stltool.h
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<

Convert_zj.o : $(SRCDIR)/proj/Convert_zj.cpp
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<

Convert.o : $(SRCDIR)/proj/Convert.cpp $(SRCDIR)/proj/Convert.h
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<

ReconnectThread.o :  $(SRCDIR)/proj/ReconnectThread.cpp $(SRCDIR)/proj/ReconnectThread.h
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<

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
	
#### Backup module --------------------##

BackupModule.o : $(SRCDIR)/Backup/BackupModule.cpp 
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<

BackupUtils.o : $(SRCDIR)/Backup/BackupUtils.cpp 
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<

ClearDirThread.o : $(SRCDIR)/Backup/ClearDirThread.cpp 
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<

Database.o : $(SRCDIR)/Backup/Database.cpp 
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<

DataCache.o : $(SRCDIR)/Backup/DataCache.cpp 
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<

DBIOThread.o : $(SRCDIR)/Backup/DBIOThread.cpp 
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<

ResumeTransfer.o : $(SRCDIR)/Backup/ResumeTransfer.cpp 
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<

RotationBackup.o : $(SRCDIR)/Backup/RotationBackup.cpp 
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<

RotationFullBackup.o : $(SRCDIR)/Backup/RotationFullBackup.cpp 
	$(CXX) $(INCPATH) $(CXXFLAGS) -o $@ -c $<

### Backup module end -----------------##

