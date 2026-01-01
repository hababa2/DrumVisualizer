#include "Defines.hpp"

#include "Visualizer.hpp"

//#ifdef DV_RELEASE
//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
//#else
int main()
//#endif
{
	if (!Visualizer::Initialize())
	{
		Visualizer::Shutdown();
		return -1;
	}

	return 0;
}
