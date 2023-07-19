
#pragma once

#include <string>
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
	int m_num_params;

	std::vector<std::unique_ptr<gp_tree<state>>> m_children;

	gp_tree(int num_params) {
		m_num_params = num_params;
	}

	state call(const std::vector<state> params) const {
		if (params.size() != m_num_params) {
			throw std::runtime_error("Expected " + std::to_string(m_num_params) + " parameters but got " + std::to_string(params.size()));
		}

		if (m_node.m_num_child == 0) {
			return m_node.m_function(params);
		}

		std::vector<state> inp;
		std::transform(m_children.cbegin(), m_children.cend(), std::back_inserter(inp), [&params](const auto &child) {
			return child->call(params);
		});
		return m_node.m_function(inp);
	}

	static gp_tree make_random_tree(int num_params, int max_depth, int max_width, const std::vector<gp_node<state>> &nodes) {
        
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> rnd_dist2(0, num_params - 1);

        const int param = rnd_dist2(rng);
        
        gp_node<state> param_node("param", 0, [param](auto p) {
            return p[param];
        });

		gp_tree ret(num_params);
        ret.m_node = param_node;

		if (max_depth <= 0) return ret;
		if (max_width <= 0) return ret;

		// add child tree
		std::uniform_int_distribution<std::mt19937::result_type> rnd_dist(0, nodes.size());
		const int node_num = rnd_dist(rng);

		auto random_node = (node_num == nodes.size()) ? param_node : nodes[node_num];

		if (max_width < random_node.m_num_child) return ret;
        
        ret.m_node = random_node;

		for (int i = 0; i < random_node.m_num_child; i++) {
			gp_tree child = gp_tree<state>::make_random_tree(num_params, max_depth - 1, max_width - random_node.m_num_child, nodes);
			auto child_ptr = std::make_unique<gp_tree>(std::move(child));
			ret.m_children.push_back(std::move(child_ptr));
		}
		return ret;
	}

	static std::string tree_to_string(const gp_tree<state> &tree, int depth = 0) {
		std::string ret;
		ret += std::string(depth, '\t') + "[" + tree.m_node.m_name + "] ("+std::to_string(tree.m_children.size())+")\n";
		for (const auto &child : tree.m_children) {
			ret += tree_to_string(*child, depth + 1);
		}
		return ret;
	}

};

