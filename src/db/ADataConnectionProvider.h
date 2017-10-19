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

#ifndef ADATACONNECTIONPROVIDER_H_
#define ADATACONNECTIONPROVIDER_H_
#include "../common.h"

#ifdef _MSC_VER
#define pthread_mutex_lock(mutex) WaitForSingleObject(mutex, INFINITE);
#define pthread_mutex_unlock(mutex) ReleaseMutex(mutex);
#define pthread_mutex_destroy(mutex) CloseHandle(mutex);
#endif
class ADataConnectionProvider
{
	private:
		/*
		 * Database connection handle
		 */
		dbh_t* connection;

	protected:
		virtual bool isDatabaseStrucureExists() = 0;
		virtual void createDatabaseStructure() = 0;

	public:
		virtual void openConnection() = 0;
		virtual void closeConnection() = 0;
		virtual void dump() = 0;

		/**
		 * Handles connection mutex
		 *
		 * Any read/write operations on connection
		 * should be placed between lock() and unlock() methods
		 */
		virtual void lock() {
			pthread_mutex_lock(&connection->mutex);
		};

		virtual void unlock() {
			pthread_mutex_unlock(&connection->mutex);
		}

		virtual dbh_t* getConnection() {
			return connection;
		};

		ADataConnectionProvider()
		{
			connection = new dbh_t;
#ifdef _MSC_VER
            connection->mutex = CreateMutex(NULL, false, NULL);
#else
			pthread_mutex_init(&connection->mutex, NULL);
#endif
		}

		virtual ~ADataConnectionProvider()
		{
			if (connection != NULL && connection->handle != NULL)
			{
				pthread_mutex_destroy(&connection->mutex);
				delete connection;
			}
		}
};

#endif /* ADATACONNECTIONPROVIDER_H_ */
