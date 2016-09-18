/*
 * ADataProvider.h
 *
 *  Created on: May 13, 2014
 *      Author: root
 */

#ifndef ADATAPROVIDER_H_
#define ADATAPROVIDER_H_
#include "common.h"
#include "ADataConnectionProvider.h"

class ADataProvider
{
	public:
		ADataProvider() {};
		virtual ~ADataProvider() {};
		virtual void insertRecord(hstat_t *packet) = 0;
		virtual ADataConnectionProvider* getProvider() = 0;
};

#endif /* ADATAPROVIDER_H_ */
