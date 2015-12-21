#include <sys/stat.h>

int main() {
	mkdir("test_dir", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	return 0;
}