/*
 * ADataConnectionProvider.h
 *
 *  Created on: May 13, 2014
 *      Author: root
 */

#ifndef ADATACONNECTIONPROVIDER_H_
#define ADATACONNECTIONPROVIDER_H_
#include "common.h"

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
			pthread_mutex_init(&connection->mutex, NULL);
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
