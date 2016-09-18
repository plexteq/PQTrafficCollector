#ifndef CPCAPLOCALIPADDRESSRESOLVER_H_
#define CPCAPLOCALIPADDRESSRESOLVER_H_

#include <linux/limits.h>
#include <stdint.h>
#include <iostream>
#include <stdlib.h>
#include <pcap.h>
#include <arpa/inet.h>
#include "CBaseIPResolver.h"
#include "common.h"

using namespace std;

class CPCAPLocalIPAddressResolver : public CBaseIPResolver
{
	private:
		void fetchNetworkDevices();
	public:
		CPCAPLocalIPAddressResolver();
		virtual ~CPCAPLocalIPAddressResolver(){};
};

CPCAPLocalIPAddressResolver::CPCAPLocalIPAddressResolver() : CBaseIPResolver() {
	fetchNetworkDevices();
}

void CPCAPLocalIPAddressResolver::fetchNetworkDevices()
{
	pcap_if_t *devices;
	char errbuf[PCAP_ERRBUF_SIZE];
	int status = pcap_findalldevs(&devices, errbuf);

	if (status != 0 || devices == NULL) {
		//TODO implement proper exceptions system
		cout << "Error fetching system interfaces! Exiting" << endl;
		exit(1);
	}

	// retrieving ip addresses and storing them in hash
	for (pcap_if_t *d = devices ; d != NULL; d = d->next) {
		for (pcap_addr_t *addr = d->addresses ; addr != NULL ; addr = addr->next) {
			if (addr->addr == NULL) continue;
			if (addr->addr->sa_family == AF_INET) {
				this->addip(_get_long_ip(&((struct sockaddr_in*)addr->addr)->sin_addr));
			}
		}
	}

	pcap_freealldevs(devices);
}

#endif /* CPCAPLOCALIPADDRESSRESOLVER_H_ */
