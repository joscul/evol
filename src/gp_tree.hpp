
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

	gp_tree(const gp_node<state> &node, int num_params = 0) {
		m_node = node;
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
			std::cout << tree_to_string(*this);
			throw std::runtime_error("Expected " + std::to_string(m_num_params) + " parameters but got " + std::to_string(params.size()));
		}

		if (m_node.m_num_child == 0) {
			return m_node.call(params);
		}

		std::vector<state> inp;
		std::transform(m_children.cbegin(), m_children.cend(), std::back_inserter(inp), [&params](const auto &child) {
			return child->call(params);
		});
		return m_node.call(inp);
	}
	
	std::vector<gp_tree<state> *> all_nodes(const std::string &type = "") {
		std::vector<gp_tree<state> *> ret;
		if (type == "") ret.push_back(this);
		else if (type == m_node.m_name) ret.push_back(this);
		for (const auto &child : m_children) {
			auto sub = child->all_nodes(type);
			ret.insert(ret.end(), sub.begin(), sub.end());
		}
		return ret;
	}
	
	gp_tree<state> *random_node(const std::string &type = "") {
		auto nodes = all_nodes(type);
		if (nodes.size() == 0) return NULL;
		return nodes[uniform::uniform_int(0, nodes.size() - 1)];
	}
	
	bool mutate(int max_depth, int max_width, const std::vector<gp_node<state>> &nodes) {
		auto random_child = random_node();
		if (random_child == NULL) return false;
		if (random_child->m_children.size() > 0) {
			// replace random child with random subtree.
			auto child_num = uniform::uniform_int(0, m_children.size() - 1);
			m_children[child_num] = std::make_unique<gp_tree<state>>(make_random_tree(m_num_params, max_depth, max_width, nodes));
			return true;
		}
		return false;
	}

	bool mutate_constant() {

		auto random_child = random_node("const");
		if (random_child == NULL) return false;

		random_child->m_node.m_constants[0] += uniform::uniform_int(-1, 1);

		return false;
	}

	bool minify(double score, std::function<double(const gp_tree<state> &)> utility_function, const gp_node<state> &null_node) {
	
		auto nodes = all_nodes();
		auto tmp = std::make_unique<gp_tree<state>>(null_node, m_num_params);

		for (const auto &node : nodes) {
			for (auto &child : node->m_children) {
				// Try to replace each node with a null node.
				if (child->m_node.m_name != "null") {
					child.swap(tmp);
					const double new_score = utility_function(*this);
					if (new_score <= score) {
						return true;
					}
					child.swap(tmp);
				}

				// Try to replace a node with one of its children.
				for (auto &child_child : child->m_children) {
					// So try to replace child and child_child
					child.swap(child_child);
					const double new_score = utility_function(*this);
					if (new_score <= score) {
						child_child.reset(nullptr);
						return true;
					}
					child.swap(child_child);
				}
			}
		}
		return false;
	}

	bool crossover(gp_tree &other) {

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
		return false;
	}

	static gp_tree make_random_tree(int num_params, int max_depth, int max_width, const std::vector<gp_node<state>> &nodes) {

		const int param = uniform::uniform_int(0, num_params - 1);

		gp_node<state> param_node("param_" + std::to_string(param), 0, 0, [param](auto node, auto p, auto c) {
			return p[param];
		});

		gp_tree ret(num_params);
		ret.m_node = param_node;

		if (max_depth <= 1) return ret;
		if (max_width <= 1) return ret;

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
		if (tree.m_node.m_constants.size() > 0) {
			std::string cns;
			for (auto &c : tree.m_node.m_constants) {
				cns += ", " + std::to_string(c);
			}
			ret += std::string(depth, '\t') + "[" + tree.m_node.m_name + "] ("+std::to_string(tree.m_children.size())+") (" + cns + ")\n";
		} else {
			ret += std::string(depth, '\t') + "[" + tree.m_node.m_name + "] ("+std::to_string(tree.m_children.size())+")\n";
		}
		for (const auto &child : tree.m_children) {
			ret += tree_to_string(*child, depth + 1);
		}
		return ret;
	}

	int size() const {
		int size = 1;
		for (const auto &child : m_children) {
			size += child->size();
		}
		return size;
	}

	int depth() const {
		int depth = 1;
		int largest_child_depth = 0;
		for (const auto &child : m_children) {
			auto child_depth = child->depth();
			if (child_depth > largest_child_depth) {
				largest_child_depth = child_depth;
			}
		}
		depth += largest_child_depth;
		return depth;
	}

	std::string hash_str() const {
		std::string ret = m_node.m_name + "_" + std::to_string(m_node.m_num_child) + std::to_string(m_node.m_num_const) + ";";
		for (const auto &child : m_children) {
			ret += child->hash_str();
		}
		return ret;
	}

	/*
	 * Returns a map with the hash of the subtree as key and a pointer to the subtree as value.
	 * A hash of a tree should identify the exact tree.
	 * */
	std::map<long, const gp_tree<state> *> hash_map() const {
		std::map<long, const gp_tree<state> *> ret;
		std::hash<std::string> hasher;

		// we only want to add hashes for nodes that has children. other nodes are end nodes and are useless.
		if (m_children.size()) {
			auto hash_str = tree_to_string(*this);
			ret[hasher(hash_str)] = this;
		}
		for (const auto &child : m_children) {
			auto other = child->hash_map();
			ret.merge(other);
		}
		return ret;
	}

};

