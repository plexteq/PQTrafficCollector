remoteurl=http://www.sqlite.org/2014
sqlite=sqlite-autoconf-3080500

pqtc: pqtc.cpp
	g++ -O3 -std=c++0x -L/opt/sqlite/lib/ -I/opt/sqlite/include/ -lpthread -lsqlite3 -lpcap -lapr-1 -laprutil-1 pqtc.cpp -o pqtc
prepare:
	yum -y install apr apr-util apr-devel apr-util-devel sqlite sqlite-devel libpcap libpcap-devel
	(cd /tmp ; rm -f $(sqlite).tar.gz; rm -Rf ./$(sqlite) ; wget $(remoteurl)/$(sqlite).tar.gz ; tar -zxf ./$(sqlite).tar.gz ; cd $(sqlite) ; ./configure --prefix /opt/sqlite ; make ; make install)
	echo "/opt/sqlite/lib/" > /etc/ld.so.conf.d/sqlite.conf && ldconfig && echo "export PATH=\$$PATH:/opt/sqlite/bin" >> ~/.bashrc
	cpan DBD::SQLite
	cpan Math::Round
	
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
