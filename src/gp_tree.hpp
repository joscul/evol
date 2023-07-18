
#pragma once

#include <iostream>
#include <vector>
#include <functional>
#include <memory>
#include <random>

#include "gp_node.hpp"

template <typename state>
class gp_tree {

public:

	gp_node<state> m_node;

	std::vector<std::unique_ptr<gp_tree<state>>> m_children;

	static gp_tree make_random_tree(int input_params, int max_depth, int max_width, const std::vector<gp_node<state>> &nodes) {

		gp_tree ret;
		if (max_depth <= 0) return ret;
		if (max_width <= 0) return ret;

		// add child tree
		std::random_device dev;
		std::mt19937 rng(dev());
		std::uniform_int_distribution<std::mt19937::result_type> rnd_dist(0, nodes.size() - 1);
		const int node_num = rnd_dist(rng);
		auto random_node = nodes[node_num];

		if (max_width < random_node.m_num_child) return ret;

		for (int i = 0; i < random_node.m_num_child; i++) {
			gp_tree child = gp_tree<state>::make_random_tree(max_depth - 1, max_width - random_node.m_num_child, nodes);
			child.m_node = random_node;
			auto child_ptr = std::make_unique<gp_tree>(std::move(child));
			ret.m_children.push_back(std::move(child_ptr));
		}
		return ret;
	}

	static std::string tree_to_string(const gp_tree<state> &tree, int depth = 0) {
		std::string ret;
		ret += std::string(depth, '\t') + "[" + tree.m_node.m_name + "]\n";
		for (const auto &child : tree.m_children) {
			ret += tree_to_string(*child, depth + 1);
		}
		return ret;
	}

};

