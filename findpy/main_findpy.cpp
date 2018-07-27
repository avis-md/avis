#include <signal.h>
#include <Python.h>

void _dieded(int i) {
	exit(1);
}

int main(int argc, char **argv) {
	signal(SIGABRT, &_dieded);
	Py_Initialize();
	return 0;
}