
#include <iostream>
#include <map>
#include "gp_tree.hpp"
#include "trainer.hpp"

int main() {

	const gp_node<int> add("add", 2, [](auto p) {
		return p[0] + p[1];
	});

	const gp_node<int> mul("mul", 2, [](auto p) {
		return p[0] * p[1];
	});

	auto nodes = {
		add,
		mul,
	};
    
    auto tree1 = gp_tree<int>::make_random_tree(2, 5, 6, nodes);
    auto tree2 = tree1;
    
    std::cout << gp_tree<int>::tree_to_string(tree1);
    std::cout << gp_tree<int>::tree_to_string(tree2);
    
    return 0;
    
    std::map<std::vector<int>, int> train_data;
    
    auto target_fun = [](int x, int y) {
        return x*x*x + y*y;
    };
    
    for (int i = -50; i < 50; i++) {
        int x = i;
        int y = -i;
        train_data[{x, y}] = target_fun(x, y);
    }
    

    trainer<int> trn(2, 5, 6, nodes);
    trn.train(100, 100, [&train_data](const auto &gp_tree) {
        double score = 0;
        for (const auto &iter : train_data) {
            int output = gp_tree.call(iter.first);
            double diff = output - iter.second;
            score += std::sqrt(diff * diff);
        }
        return score;
    });
    
    //auto best = trn.get_best();
    
    //std::cout << gp_tree<int>::tree_to_string(*best);
    //std::cout << "output: " << best->call({1, 1}) << std::endl;

	return 0;
}
