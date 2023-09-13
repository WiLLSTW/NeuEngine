#pragma once
#include <random>
//Inspired By The Cherno

class Random {
public:
	static void Init() {
		s_Random.seed(std::random_device()());
	};

	static float Float() {
		return (float)s_Distribution(s_Random) / (float)std::numeric_limits<uint32_t>::max();
	}

	static uint32_t Int() {
		return s_Distribution(s_Random);
	}

private:
	static std::mt19937 s_Random;
	static std::uniform_int_distribution<std::mt19937::result_type>s_Distribution;

};