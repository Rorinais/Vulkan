#include "../base.hpp"
#include "../core/application/app.hpp"


int main() {
#ifdef _WIN32
	_putenv_s("VK_LAYER_PATH", "layers");
#endif 

	App app;
	app.run();

	return 0;
}