#include <iostream>
#include <unistd.h>

int main() {
	if ( !rmdir( "test_dir" ) ) {
		std::cout << "test_dir removed\n";
	}
	return 0;
}