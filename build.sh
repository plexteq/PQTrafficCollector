#!/bin/bash 

# jansson donwload URL
JANSSON_URL=https://github.com/templarbit/jansson/archive/master.zip

# operating system type
OS=$(. /etc/os-release; echo "$ID $ID_LIKE")

# shared library location after build is completed
SO="./pqtc"

# colors definition
red() { tput setaf 1; }
green() { tput setaf 2; }
res() { tput sgr0; }

echo
echo "* Current OS group is: $OS"

echo
echo "`green`* Setting up build dependencies `res`"
if [[ $OS == *"debian"* ]]; then
        apt-get update

	sudo apt-get install wget \
			libtool \
	                autoconf \
	                automake \
			libexpat-dev \
	                autoheader

	# installing boost
	echo "`green`* Installing boost`res`"
	sudo apt-get install libboost-all-dev
	echo

	# installing boost
	echo "`green`* Installing pcap`res`"
	sudo apt-get install libpcap-dev
	echo

	# installing sqlite3
	echo "`green`* Installing sqlite3`res`"
	sudo apt-get install sqlite3 libsqlite3-dev
	echo

	# installing apr
	echo "`green`* Installing apr and apr-util`res`"
	sudo apt-get install libapr1-dev libaprutil1-dev
	echo


	# installing apr
	echo "`green`* Installing log4cpp`res`"
	sudo apt-get install liblog4cpp5-dev
	echo
                        
elif [[ $OS == *"rhel"* ]]; then
	rpm -Uvh http://dl.fedoraproject.org/pub/epel/7/x86_64/e/epel-release-7-10.noarch.rpm
	yum install httpd-devel \
                    perl \
                    pcre-devel \
                    zlib \
                    zlib-devel \
                    GeoIP \
                    GeoIP-devel \
                    boost-devel \
                    wget \
                    libtool \
                    autoconf \
                    automake \
                    autoheader \
                    unzip \
		    libpcap-devel \
		    sqlite \
		    libexpat-devel\
		    libapr1-devel  \
		    libaprutil1-devel \
		    liblog4cpp5-devel
else
        echo "`red`* Error`res`: current OS is not supported"
        exit 1
fi

# cleaning up
echo
echo "`green`* Cleaning up from previous run `res`"
rm -Rf vendor/jansson

# getting jansson
echo
echo "`green`* Retrieving jansson sources `res`"
mkdir -p vendor/jansson
wget $JANSSON_URL -O ./vendor/jansson_src.zip
( \
  cd vendor/; \
  unzip ./jansson_src.zip; \
  mv ./jansson-master/* ./jansson/; \
  rm -Rf ./jansson-master jansson_src.zip \
)

# building jansson
echo
echo "`green`* Building jansson`res`"
( \
  cd ./vendor/; \
  cd jansson/; \
  autoreconf -i; \
  ./configure; \
  make; \
  make install \
)

# building hptc
echo
echo "`green`* Building hptc`res`"
( \
  cd ../../; \ 
  make 
)

echo
if [[ -e "$SO" ]]; then
   echo "`green`* pqtc has been built successfully. Location: $SO `res`"
   file $SO
else
   echo "`red`* pqtc has not been built. Check build errors above `res`"
   exit 2
fi

echo
