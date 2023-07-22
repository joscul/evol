
#include <iostream>
#include <map>
#include "gp_tree.hpp"
#include "trainer.hpp"
#include "uniform.hpp"

int main() {

	const gp_node<int> add("add", 2, 0, [](auto p, auto c) {
		return p[0] + p[1];
	});

	const gp_node<int> mul("mul", 2, 0, [](auto p, auto c) {
		return p[0] * p[1];
	});

	const gp_node<int> con("const", 0, 1, [](auto p, auto c) {
		return c[0];
	});

	auto nodes = {
		add,
		mul,
		con,
	};

    std::map<std::vector<int>, int> train_data;
    
    auto target_fun = [](int x, int y) {
        return x*x*x + y*(y + 5) + 3;
    };
    
    for (int i = -50; i < 50; i++) {
        int x = uniform::uniform_int(-50, 50);
        int y = uniform::uniform_int(-50, 50);
        train_data[{x, y}] = target_fun(x, y);
    }
    

    trainer<int> trn(2, 15, 15, nodes);
    trn.train(200, 100, [&train_data](const auto &gp_tree) {
        double score = 0;
        for (const auto &iter : train_data) {
            int output = gp_tree.call(iter.first);
            double diff = output - iter.second;
            score += std::sqrt(diff * diff);
        }
        return score;
    });
    
    auto best = trn.get_best();
    
    std::cout << gp_tree<int>::tree_to_string(best);
    std::cout << "score: " << trn.m_best_score << std::endl;
    //std::cout << "output: " << best->call({1, 1}) << std::endl;

	return 0;
}
