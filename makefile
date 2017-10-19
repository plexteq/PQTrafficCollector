remoteurl=http://www.sqlite.org/2014
sqlite=sqlite-autoconf-3080500

CXX=g++
OPTIMIZATION=-g -O0
STD_MODE=-std=c++0x
LDFLAGS=-Wall -Wl,-rpath,/usr/local/lib/gcc5/

OBJ_FOLDER=objs/
SOURCES=$(shell find src/ -name '*.cpp' -not -path "src/service/win32") 
OBJECTS=$(patsubst src/%.cpp, $(OBJ_FOLDER)%.o, $(SOURCES))
LIBS=-lpthread -lsqlite3 -lpcap -lapr-1 \
-laprutil-1 -lexpat -lboost_system -lboost_filesystem -ljansson -llog4cpp
INCLUDES=
LINKER=
EXECUTABLE=pqtc

DIRS:=$(patsubst src/%, $(OBJ_FOLDER)/%, $(shell find src/ -type d))
OBJS_DIRS:=$(shell mkdir -p $(DIRS))

all:$(OBJECTS)	
	${CXX} ${OPTIMIZATION} ${STD_MODE} \
	${INCLUDES} ${LINKER} ${LDFLAGS} \
	-o ${EXECUTABLE} ${OBJECTS} ${LIBS}
	
clean:
	rm -rf ${EXECUTABLE} $(OBJ_FOLDER)
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


$(OBJECTS): $(OBJ_FOLDER)%.o: src/%.cpp
	$(CXX) $(OPTIMIZATION) $(STD_MODE) -c $< -o $@
