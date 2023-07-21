//
//  uniform.cpp
//  evol
//
//  Created by Josef Cullhed on 2023-07-20.
//

#include "uniform.hpp"
#include <random>

namespace uniform {

    std::random_device dev;
    std::mt19937 twister(dev());

    int uniform_int(int min, int max) {
        std::uniform_int_distribution<std::mt19937::result_type> range(min, max);
        return range(twister);
    }

	double uniform_double(double min, double max) {
		std::uniform_real_distribution<double> range(min, max);
		return range(twister);
	}
}

