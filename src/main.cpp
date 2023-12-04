#include "app.h"

int main(int argc, const char* argv[])
{
	LOG("main", "starting application");

	using namespace vxgi;

	if (application::init(argc, argv, { 1920, 1080 })) {
		application::run();
	}
	application::uninit();

	LOG("main", "exiting application");
	return 0;
}