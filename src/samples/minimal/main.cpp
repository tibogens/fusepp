/*
Copyright (c) 2014, University of Lausanne, Switzerland 
All rights reserved. 

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are 
met: 

1. Redistributions of source code must retain the above copyright 
notice, this list of conditions and the following disclaimer. 

2. Redistributions in binary form must reproduce the above copyright 
notice, this list of conditions and the following disclaimer in the 
documentation and/or other materials provided with the distribution. 

3. Neither the name of the copyright holder nor the names of its 
contributors may be used to endorse or promote products derived from 
this software without specific prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS 
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A 
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 

*/

#include <fusepp/Application.h>
#include <unistd.h>
#include <sys/types.h>
#include <iostream>


class MinimalFileSystem : public fusepp::FS_getattr, public fusepp::FS_readdir
{
public:
	int getattr(const std::string& path, struct stat* buf)
	{
		std::cout << "getattr(" << path << ")" << std::endl;
		if (path == "/" || path == "/bar")
		{
			buf->st_mode = S_IFDIR | 0755;
			buf->st_uid = getuid();
			buf->st_gid = getgid();
			return 0;
		}
		else if (path == "/foo")
		{
			buf->st_mode = S_IFREG | 0644;
			buf->st_uid = getuid();
			buf->st_gid = getgid();
			return 0;
		}
		return -ENOENT;
	}

	int readdir(const std::string& path, DirectoryFiller& filler)
	{
		std::cout << "readdir(" << path << ")" << std::endl;
		if (path == "/")
		{
			filler.add("foo");
			filler.add("bar");
		}
		return 0;
	}
};


int main(int argc, char* argv[])
{
	fusepp::FileSystemPtr fs(new MinimalFileSystem());
	fusepp::ApplicationPtr app(new fusepp::Application(fs));
	
	return app->run(argc, argv);
}
