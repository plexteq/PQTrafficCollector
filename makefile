remoteurl=http://www.sqlite.org/2014
sqlite=sqlite-autoconf-3080500

CXX=g++
OPTIMIZATION=-g -O0
STD_MODE=-std=c++0x
LDFLAGS=-Wl,-rpath,/usr/local/lib/gcc5/

SOURCES=$(shell find src/ -name '*.cpp' -not -path "src/service/win32") 
LIBS=-lpthread -lsqlite3 -lpcap -lapr-1 \
-laprutil-1 -lexpat -lboost_system -lboost_filesystem -ljansson -llog4cpp
INCLUDES=-I/usr/local/apr/include/ -I/usr/local/apr/lib/include/
LINKER=-L/usr/local/apr/lib/ -L/usr/local/apr/lib/lib/ -L./libs/log4cpp/
EXECUTABLE=pqtc

all: src/pqtc.cpp
	${CXX} ${OPTIMIZATION} ${STD_MODE} \
	${INCLUDES} \
	${LINKER} \
	${LDFLAGS} \
	${SOURCES} \
	${LIBS} \
	-o ${EXECUTABLE}
	
clean:
	rm -f ${EXECUTABLE}
install:
	mkdir -p /var/db/${EXECUTABLE}
	cp -f ext/${EXECUTABLE} /etc/init.d/
	cp -f ${EXECUTABLE} /usr/bin/
	cp -f ext/gettop.pl /usr/bin/
	test -f /usr/bin/${EXECUTABLE} && cp -f ext/check_ifutil.pl /etc/nagios/nvpn/
	chkconfig ${EXECUTABLE} on
	service ${EXECUTABLE} restart

reinstall:
	service ${EXECUTABLE} stop
	cp -f ${EXECUTABLE} /usr/bin/
	cp -f ext/${EXECUTABLE} /etc/init.d/
	cp -f ext/gettop.pl /usr/bin/
	cp -f ext/check_ifutil.pl /etc/nagios/nvpn/
	service ${EXECUTABLE} start
