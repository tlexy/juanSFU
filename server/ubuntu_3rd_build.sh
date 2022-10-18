#!/usr/bin
if [ $# != 1 ] ; then
	echo "USAGE: install or clean???"
	exit 1;
fi

do_clean(){
	cd 3rd
    	rm -rf libsrtp-2.4.2
	rm -rf jsoncpp-1.9.5
	rm -rf libuv-v1.44.1
}

do_install(){
	cd 3rd
	apt install -y libtool pkg-config build-essential cmake libssl-dev
	unzip jsoncpp-1.9.5.zip && cd jsoncpp-1.9.5/cmake && cmake .. && make && make install
	cd ../../
	tar xzvf libuv-v1.44.1.tar.gz && cd libuv-v1.44.1 && ./autogen.sh && ./configure && make && make install
	cd ..
	unzip libsrtp-2.4.2.zip && cd libsrtp-2.4.2 && ./configure && make && make install
}


cmd1="install"
cmd2="clean"

if [ "$1" = "$cmd1" ] ; then
	#scl enable devtoolset-7
	#PKG_CONFIG_PATH
	echo "Your want to install the dependents"
	do_install
	echo "/usr/local/lib" >> /etc/ld.so.conf
	echo "/usr/local/lib64" >> /etc/ld.so.conf
	ldconfig
	exit 1;
elif [ "$1" = "$cmd2" ] ; then
	echo "Your want to clean the dependents"
	do_clean
	echo "clean over."
	exit 1;
fi
