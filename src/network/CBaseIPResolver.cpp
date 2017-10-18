/**
 * Copyright (c) 2017, Plexteq
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

#include "CBaseIPResolver.h"

#ifdef _MSC_VER 
#define inet_aton(ip_str,pstruct) inet_pton(AF_INET,ip_str,pstruct);
#endif

CBaseIPResolver::CBaseIPResolver()
{
	className = string(__func__);
	addresses = new map<uint32_t, bool>;
}

CBaseIPResolver::~CBaseIPResolver()
{
	addresses->clear();
	delete addresses;
}

bool CBaseIPResolver::isLocalIP(struct in_addr *in)
{
	return isLocalIP((uint32_t)in->s_addr);
}

bool CBaseIPResolver::isLocalIP(uint32_t addr)
{
	return (addresses->find(addr) != addresses->end());
}

bool CBaseIPResolver::isLocalIP(const char *addr)
{
	struct in_addr inaddr;
	inet_aton(addr, &inaddr);
	return isLocalIP(&inaddr);
}

void CBaseIPResolver::dump()
{
	Logger::info(className,
			string("Found ")
					+ boost::lexical_cast<std::string>(addresses->size())
					+ " ip addresses");

	struct in_addr inaddr;
	for (map<uint32_t, bool>::iterator i = addresses->begin();
			i != addresses->end(); ++i)
	{
		inaddr.s_addr = ntohl((*i).first);
		Logger::info(className, string(" ->") + inet_ntoa(inaddr));
	}
}


