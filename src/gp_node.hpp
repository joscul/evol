
#pragma once

#include "gp_tree.hpp"

template <typename state> class gp_tree;

template <typename state>
class gp_node {

	public:

	std::string m_name;
	int m_num_child;
	int m_num_const;
	const gp_tree<state> *m_tree;
	std::function<state(const gp_node<state> &node, const std::vector<state> &p, const std::vector<double> &c)> m_function;
	std::vector<double> m_constants;

	gp_node() {
	}

	gp_node(const std::string name, int num_child, int num_const, std::function<state(const gp_node<state> &node, const std::vector<state> &p, const std::vector<double> &c)> fun) {
		m_name = name;
		m_num_child = num_child;
		m_function = fun;
		m_constants.resize(num_const);
	}

	state call(const std::vector<state> &params) const {
		return m_function(*this, params, m_constants);
	}

};
