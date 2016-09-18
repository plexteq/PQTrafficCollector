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

#ifndef CBASEIPRESOLVER_H_
#define CBASEIPRESOLVER_H_

#include <iostream>
#include <map>
#include <utility>
#include "common.h"

using namespace std;

class CBaseIPResolver
{
	protected:
		std::map<uint32_t, bool> *addresses;
		void addip(uint32_t longip) {
			addresses->insert(std::pair<uint32_t, bool>(longip, true));
		}
	public:
		CBaseIPResolver();
		virtual ~CBaseIPResolver();
		virtual bool isLocalIP(struct in_addr *in);
		virtual bool isLocalIP(uint32_t addr);
		virtual bool isLocalIP(const char *addr);
		virtual void dump();
};

CBaseIPResolver::CBaseIPResolver() {
	addresses = new map<uint32_t, bool>;
}

CBaseIPResolver::~CBaseIPResolver() {
	addresses->clear();
	delete addresses;
}

bool CBaseIPResolver::isLocalIP(struct in_addr *in) {
	return isLocalIP(in->s_addr);
}

bool CBaseIPResolver::isLocalIP(uint32_t addr) {
	return (addresses->find(addr) != addresses->end());
}

bool CBaseIPResolver::isLocalIP(const char *addr) {
	struct in_addr inaddr;
	inet_aton(addr, &inaddr);
	return isLocalIP(&inaddr);
}

void CBaseIPResolver::dump() {
	cout << "Found " << addresses->size() << " ip addresses" << endl;
	struct in_addr inaddr;
	for (map<uint32_t, bool>::iterator i = addresses->begin() ; i != addresses->end() ; ++i) {
		inaddr.s_addr = ntohl((*i).first);
		cout << " -> " << inet_ntoa(inaddr) << endl;
	}
}

#endif /* CBASEIPRESOLVER_H_ */
