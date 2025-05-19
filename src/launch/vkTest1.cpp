#include "../base.hpp"
#include "../core/application/application.hpp"


int main() {
#ifdef _WIN32
	_putenv_s("VK_LAYER_PATH", "layers");
#endif 

	Application app;
	app.run();

	return 0;
}