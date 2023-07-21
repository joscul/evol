
#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <functional>
#include <memory>
#include <random>

#include "gp_node.hpp"
#include "uniform.hpp"

template <typename state>
class gp_tree {

public:

	gp_node<state> m_node;
	int m_num_params;

	std::vector<std::unique_ptr<gp_tree<state>>> m_children;

	gp_tree(int num_params = 0) {
		m_num_params = num_params;
	}
	
	gp_tree(const gp_tree<state> &tree) {
		m_node = tree.m_node;
		m_num_params = tree.m_num_params;
		for (const auto &child : tree.m_children) {
			auto ptr = std::make_unique<gp_tree<state>>(*child);
			m_children.emplace_back(std::move(ptr));
		}
	}
	
	gp_tree<state> &operator=(const gp_tree<state> &tree) {
		m_node = tree.m_node;
		m_num_params = tree.m_num_params;
		m_children.clear();
		for (const auto &child : tree.m_children) {
			auto ptr = std::make_unique<gp_tree<state>>(*child);
			m_children.emplace_back(std::move(ptr));
		}
		
		return *this;
	}
	
	gp_tree(gp_tree<state> &&tree) {
		m_node = std::move(tree.m_node);
		m_num_params = tree.m_num_params;
		m_children = std::move(tree.m_children);
	}
	
	gp_tree &operator=(gp_tree<state> &&tree) {
		m_node = std::move(tree.m_node);
		m_num_params = tree.m_num_params;
		m_children = std::move(tree.m_children);
		
		return *this;
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
	
	bool mutate(double mutate_prob, int max_depth, int max_width, const std::vector<gp_node<state>> &nodes) {
		if (m_children.size() > 0) {
			if (mutate_prob < uniform::uniform_double()) {
				// replace first child with random subtree.
				m_children[0] = std::make_unique<gp_tree<state>>(make_random_tree(m_num_params, max_depth, max_width, nodes));
				return true;
			} else if (m_children[0]->mutate(mutate_prob, max_depth - 1, max_width, nodes)) {
				return true;
			}
		}
		if (m_children.size() > 1) {
			if (mutate_prob < uniform::uniform_double()) {
				// replace first child with random subtree.
				m_children[1] = std::make_unique<gp_tree<state>>(make_random_tree(m_num_params, max_depth, max_width, nodes));
				return true;
			} else if (m_children[1]->mutate(mutate_prob, max_depth - 1, max_width, nodes)) {
				return true;
			}
		}
		return false;
	}

	std::vector<gp_tree<state> *> all_nodes() {
		std::vector<gp_tree<state> *> ret;
		ret.push_back(this);
		for (const auto &child : m_children) {
			auto sub = child->all_nodes();
			ret.insert(ret.end(), sub.begin(), sub.end());
		}
		return ret;
	}

	bool crossover(double crossover_prob, gp_tree &other) {
		if (crossover_prob > uniform::uniform_double()) {
			auto all_nodes1 = all_nodes();
			auto all_nodes2 = other.all_nodes();

			auto random_node1 = all_nodes1[uniform::uniform_int(0, all_nodes1.size() - 1)];
			auto random_node2 = all_nodes2[uniform::uniform_int(0, all_nodes2.size() - 1)];

			if (random_node1->m_children.size() && random_node2->m_children.size()) {
				auto child1 = uniform::uniform_int(0, random_node1->m_children.size() - 1);
				auto child2 = uniform::uniform_int(0, random_node2->m_children.size() - 1);
				random_node1->m_children[child1].swap(random_node2->m_children[child2]);
				return true;
			}
		}
		return false;
	}

	static gp_tree make_random_tree(int num_params, int max_depth, int max_width, const std::vector<gp_node<state>> &nodes) {

		const int param = uniform::uniform_int(0, num_params - 1);

		gp_node<state> param_node("param_" + std::to_string(param), 0, [param](auto p) {
			return p[param];
		});

		gp_tree ret(num_params);
		ret.m_node = param_node;

		if (max_depth <= 0) return ret;
		if (max_width <= 0) return ret;

		// add child tree
		const int node_num = uniform::uniform_int(0, nodes.size());

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

