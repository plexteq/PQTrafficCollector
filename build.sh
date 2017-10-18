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
	apt-get install wget \
                        libboost-all-dev \
                        libcurl4-gnutls-dev \
                        libcunit1-dev \
                        libpcre3 \
                        libpcre3-dev \
                        libtool \
                        autoconf \
                        automake \
                        autoheader \
                        unzip
    sudo apt-get install libpcap-dev \
			sqlite3 \
			libsqlite3-dev \
			libexpat-dev \
			libapr1-dev  \
			libaprutil1-dev
                        
elif [[ $OS == *"rhel"* ]]; then
	rpm -Uvh http://dl.fedoraproject.org/pub/epel/7/x86_64/e/epel-release-7-10.noarch.rpm
	yum install httpd-devel \
                    pcre \
                    perl \
                    pcre-devel \
                    zlib \
                    zlib-devel \
                    GeoIP \
                    GeoIP-devel \
                    boost-devel \
                    CUnit-devel \
                    curl-devel \
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
		    libaprutil1-devel
else
        echo "`red`* Error`res`: current OS is not supported"
        exit 1
fi

# cleaning up
echo
echo "`green`* Cleaning up from previous run `res`"
rm -Rf vendor/{jansson,nginx}

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
  cd vendor/jansson/; \
  autoreconf -i; \
  ./configure; \
  make  \
  make install \
)

# building hptc
echo
echo "`green`* Building hptc`res`"
( \
  cd ../../ \ 
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
