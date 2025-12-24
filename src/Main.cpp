#include "Visualizer.hpp"

int main()
{
	if (!Visualizer::Initialize()) { return -1; }

	Visualizer::Shutdown();
	return 0;
}
