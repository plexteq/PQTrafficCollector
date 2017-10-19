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

#ifndef SRC_UTILITY_JSONHELPER_H_
#define SRC_UTILITY_JSONHELPER_H_

#include <vector>
#include <jansson.h>
#include <boost/filesystem.hpp>

using namespace boost::filesystem;

class JSONHelper
{
	typedef json_t JSON_OBJ_T;
public:

	/* Create a JSON object.
	 *
	 * Parameters:
	 *
	 * Return value:
	 * 		void*: pointer to the new JSON object
	 * */
	static void* createObject();

	/* Add integer value to the JSON structure
	 *
	 * Parameters:
	 * root  - [In/Out] a pointer to the JSON object
	 * name  - [In] a constant string of the element key
	 * value - [In] an integer value of the element value
	 *
	 * Return value:
	 * 		void (the JSON object 'root' will contain a new element)
	 *
	 * */
	static void addInteger(void* root, const char* name, int value);

	/* Add array of files paths to the JSON structure
	 *
	 * Parameters:
	 * root - [In/Out] a pointer to the JSON object
	 * name - [In] a constant string of the array key
	 * arr 	- [In] a vector of boost::filesystem::path with databases file names
	 * 			   which short names will be added to the JSON array
	 *
	 * Return value:
	 * 		void (the JSON object 'root' will contain a new element)
	 *
	 * */
	static void addArrayPaths(void* root, const char* name,
			std::vector<path> &arr);

	/* Return a string readable form of the JSON object
	 *
	 * Parameters:
	 * root - [In] a pointer to the JSON object
	 *
	 * Return value:
	 * 		char*: a string readable form of the JSON object or NULL
	 *
	 * */
	static char* serialize(void* root);

	/* Release a memory used by the JSON object
	 *
	 * Parameters:
	 * root - [In/Out] a pointer to the JSON object
	 *
	 * Return value: void
	 *
	 * */
	static void free(void* root);
};

#endif /* SRC_UTILITY_JSONHELPER_H_ */
