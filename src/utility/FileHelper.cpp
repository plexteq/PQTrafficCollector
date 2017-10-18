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

#include "FileHelper.h"

#include <fstream>
#include <sstream>

#include "../3rdparty/tarball/tarball.h"
#include "TimeDateHelper.h"

void FileHelper::deleteFile(string name)
{
	if (fileExist(name))
		boost::filesystem::remove(path(name));
}

string FileHelper::createTarball(std::vector<boost::filesystem::path>* files,
		char &error)
{
	error = 0;

	stringstream filename;

	if (files->size() > 0)
		filename << (*files)[0].leaf().c_str();

	filename << TimeDateHelper::getCurrentTimestamp() << ".tar";

	string outFile = filename.str();

	/* open file for writing */
	std::fstream out(outFile, std::ios::out);
	if (!out.is_open())
	{
		error = 1;
		return "";
	}

	/* create the tar file */
	lindenb::io::Tar tarball(out);

	for (int i = 0; i < files->size(); i++)
	{
		/* add a file */
		tarball.putFile((*files)[i].string().c_str(),
				(*files)[i].leaf().string().c_str());
	}
	/* write end of the archive (two empty blocks) */
	tarball.finish();

	/* close the file */
	out.close();

	return outFile;
}

bool FileHelper::fileExist(string fileName)
{
	return fileName.length() > 0 && boost::filesystem::exists(path(fileName));
}

void FileHelper::createFolder(string name, char &error)
{
	if (!boost::filesystem::exists(name))
		error = !boost::filesystem::create_directory(name);
	permissions(name, add_perms | owner_write | group_write | others_write);
}

void FileHelper::getAvailableDBList(
		std::vector<boost::filesystem::path>* dbNames, string dir,
		string activeDbFileName)
{
	if (dbNames == NULL)
		return;

	const path dirPath(dir);
	const path activeDBName(activeDbFileName);

	if (!exists(dirPath))
		return;

	directory_iterator endItr; // default construction yields past-the-end
	for (directory_iterator itr(dirPath); itr != endItr; ++itr)
	{
		if (!is_directory(itr->status())
				&& itr->path().compare(activeDBName) != 0)
		{
			dbNames->push_back(itr->path());
		}
	}
}

#ifdef _MSC_VER
string FileHelper::getCurrentExePath()
{
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);

    return std::string(buffer);
}
#endif