# PQTrafficCollector

[Build Status](https://travis-ci.org/plexteq/PQTrafficCollector)

# Usage

Command-line arguments:
- -t\<value> - a number of threads that will process a traffic information
- -q\<value> - a number of queues where will be stored a processed traffic information (should be equal to a threads number)
- -i\<value> - a name of a deice that will be monitores
- -d\<value> - a directory name where the database with collected data will be saved
- -p\<value> - a port number where the server await a connection for transfer a DBs and providing a status

#### The shell script for running on Unix OS

This is an example of a shell script which will run a pqtc with 2 threads and 2 queues, 
eth0 device will be listened, a database will be stored at a /tmp directory and 
all logging information will be redirected to a pqtc.log file.

```bash
#!/bin/bash

nohup ./pqtc -t2 -q2 -ieth0 -d/tmp/ -p13 2>&1 1>pqtc.log &
```

#### Running as a Windows Service

For running PQTrafficCollector as a Windows Service, a <b>config.ini</b> file MUST be present in the PQTrafficCollector executable 
folder.

```bash
-t2 -q2 -i\\Device\\NPF_{********-****-****-****-************} -d/tmp/ -p13
```

Asteriscs inside figure brackets after <b>\\Device\\NPF_...</b> need to be changed to a current device transport name. To know a 
current device transport name enter in the Terminal the next command and look at the "Transport name" column for the row of a needed 
network adapter.

```bash
getmac /fo csv /v
```

For installing PQTrafficCollector as a Windows Service run PQTrafficCollector executable whith <b>/install</b> argument. After that,  PQTrafficCollector service will appear in the Service Manager and "Start/Stop" buttons will be accesible for it. 
For uninstalling PQTrafficCollector as a Windows Service run PQTrafficCollector executable whith <b>/remove</b> argument.

# Building

### Additional dependencies

PQTrafficCollector uses following libraries:
* libpthread
* libsqlite3 
* libpcap 
* libapr-1 
* libaprutil-1 
* libexpat 
* libboost_system 
* libboost_filesystem

#### MSBuild scripts example

This is an example of a batch-script for building a Release executable of PQTrafficCollector.

```batch
@Echo OFF
Echo "Building project file using batch file"
SET PATH=C:\Program Files (x86)\MSBuild\14.0\Bin\
Echo Start Time - %Time%
msbuild PQTrafficCollector.sln /p:Configuration=Release
Echo End Time - %Time%
Set /p Wait=Build Process Completed...
```

#### Makefile

This is an example of a Makefile for building a Release executable of PQTrafficCollector.

```cmake
SOURCES=
	PQTrafficCollector.cpp pqtc.cpp
pqtc: pqtc.cpp
	g++ -O3 -std=c++0x -L/opt/sqlite/lib/ -I/opt/sqlite/include/ -lpthread -lsqlite3 \
	-lpcap -lapr-1 -laprutil-1 -lboost_system -lboost_filesystem $(SOURCES) -o pqtc
	
clean:
	rm -f pqtc
install:
	mkdir -p /var/db/pqtc
	cp -f ext/pqtc /etc/init.d/
	cp -f pqtc /usr/bin/
	cp -f ext/gettop.pl /usr/bin/
	test -f /usr/bin/pqtc && cp -f ext/check_ifutil.pl /etc/nagios/nvpn/
	chkconfig pqtc on
	service pqtc restart

reinstall:
	service pqtc stop
	cp -f pqtc /usr/bin/
	cp -f ext/pqtc /etc/init.d/
	cp -f ext/gettop.pl /usr/bin/
	cp -f ext/check_ifutil.pl /etc/nagios/nvpn/
	service pqtc start
```
