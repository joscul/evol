
#include <iostream>
#include "gp_tree.hpp"

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

	auto tree = gp_tree<int>::make_random_tree(2, 5, 6, nodes);
    
    std::cout << gp_tree<int>::tree_to_string(tree);

	std::cout << "output: " << tree.call({1, 1}) << std::endl;

	//std::cout << tree << std::endl;

	return 0;
}
