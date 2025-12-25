#include "Defines.hpp"

#include "Visualizer.hpp"

#ifdef DV_RELEASE
#include "Windows.h"
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main()
#endif
{
	if (!Visualizer::Initialize()) { return -1; }

	Visualizer::Shutdown();
	return 0;
}
