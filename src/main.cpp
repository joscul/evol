
#include <iostream>
#include "gp_tree.hpp"

int main() {

	const gp_node<int> add("add", 2, [](auto p) {
		return p[0] + p[1];
	});

	const gp_node<int> mul("mul", 2, [](auto p) {
		return p[0] * p[1];
	});

	const std::vector<gp_node<int>> nodes = {
		add,
		mul,
	};

	auto tree = gp_tree<int>::make_random_tree(5, 6, nodes, 2);

	//std::cout << tree << std::endl;
	std::cout << gp_tree<int>::tree_to_string(tree);

	return 0;
}
