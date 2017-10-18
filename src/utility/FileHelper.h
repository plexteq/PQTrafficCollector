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

#ifndef SRC_UTILITY_FILEHELPER_H_
#define SRC_UTILITY_FILEHELPER_H_

#include <boost/filesystem.hpp>
#include <vector>

using namespace boost::filesystem;
using namespace std;

class FileHelper
{
public:
	/* Generate a vector of boost::filesystem::path values which stores paths
	 * of all files in a database folder except a current database file.
	 *
	 * Parameters:
	 * dbNames 			- [Out] pointer to the exists
	 * 							std::vector<boost::filesystem::path> container.
	 * dir 				- [In] 	a string path to the databases folder
	 * activeDbFileName - [In] 	a string name of a current database file
	 *
	 * Return value:
	 * 		void (dbNames will contain all files path from 'dir' directory)
	 * */
	static void getAvailableDBList(std::vector<path>* dbNames, string dir,
			string activeDbFileName);

	/* Create a .tar archive of all files from 'files' vector, generate a .tar name
	 * with a current timestamp.
	 *
	 * Parameters:
	 * files - [In]  	pointer to the exists std::vector<boost::filesystem::path>
	 * 					container with files names which need to be added to .tar archieve.
	 * error - [Out]	passed by value char variable which will store an error code,
	 * 					if an error occurred
	 *
	 * Return value:
	 * 		a string path to the created .tar file or empty string if an error occurred
	 * */
	static string createTarball(std::vector<path>* files, char &error);

	/* Create a folder with specified 'name' if it not exists.
	 *
	 * Parameters:
	 * name  - [In]  	a pass and name of a new folder
	 * error - [Out]	passed by value char variable which will store an error code,
	 * 					if an error occurred
	 *
	 * Return value: void
	 * */
	static void createFolder(string name, char &error);

	/* Remove a file by 'name' if it exists.
	 *
	 * Parameters:
	 * name  - [In] a pass to the file for deletion
	 *
	 * Return value: void
	 * */
	static void deleteFile(string name);

	/* Check if a file by 'name' exists.
	 *
	 * Parameters:
	 * name  - [In] a pass to the file
	 *
	 * Return value:
	 * 		bool: true - if exists, false - if not exists
	 *
	 * */
	static bool fileExist(string fileName);

#ifdef _MSC_VER
    static string getCurrentExePath();
#endif
};

#endif /* SRC_UTILITY_FILEHELPER_H_ */
