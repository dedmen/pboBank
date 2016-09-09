
#include "global.h"


#include <csignal>
void signalHandler(int signum) {
	GLOBAL.signal(signum);
}
#ifdef _WIN32_WINNT
#include <windows.h>
#endif
int main(int, char* []) {

#ifdef _WIN32_WINNT
	HWND console = GetConsoleWindow();
	RECT r;
	GetWindowRect(console, &r); //stores the console's current dimensions
	COORD      c;
	c.X = 1600;
	c.Y = 8000;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), c);
	MoveWindow(console, r.left, r.top, 1600, 800, TRUE); // 800 width, 100 height
#endif

	//signal(SIGINT, signalHandler);
	//signal(SIGTERM, signalHandler);
	GLOBAL.init();
	GLOBAL.run();
}
