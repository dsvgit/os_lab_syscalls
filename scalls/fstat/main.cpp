#include <sys/stat.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

int main() {
	std::string fpath = "test.txt";
	struct stat finfo;
	FILE * pf = fopen( fpath.c_str(), "r" );

	if( pf == NULL ) {
		std::cout << "Cannot open file.\n";
		fclose( pf );
		exit( 1 );
	}

	fstat( fileno(pf), &finfo );

	std::cout << "fileno: " << fileno(pf) << "\n";

	fclose( pf );

	std::cout << "ID of device containing file\t\t" << finfo.st_dev << "\n";
	std::cout << "inode number\t\t\t\t" << finfo.st_ino << "\n";
	std::cout << "protection\t\t\t\t" << finfo.st_mode << "\n";
	std::cout << "number of hard links\t\t\t" << finfo.st_nlink << "\n";
	std::cout << "user ID of owner\t\t\t" << finfo.st_uid << "\n";
	std::cout << "group ID of owner\t\t\t" << finfo.st_gid << "\n";
	std::cout << "device ID (if special file)\t\t" << finfo.st_rdev << "\n";
	std::cout << "total size, in bytes\t\t\t" << finfo.st_size << "\n";
	std::cout << "blocksize for filesystem I/O\t\t" << finfo.st_blksize << "\n";
	std::cout << "number of 512B blocks allocated\t\t" << finfo.st_blocks << "\n";

	exit( 0 );
}