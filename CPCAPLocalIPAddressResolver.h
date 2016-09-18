/**
 * Copyright (c) 2014, Plexteq                                   
 * All rights reserved.
 *  
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors 
 * may be used to endorse or promote products derived from this software without 
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

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
