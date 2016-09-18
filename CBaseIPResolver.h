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
