#include "main.h"



int main() {

	Neu::NECore::getInstance()->setFpsLimit(0);
	
	try {
		Neu::NECore::getInstance()->run();
	}
	catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
	}

	return 0;
}

 