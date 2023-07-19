
#pragma once

template <typename state>
class gp_node {

	public:

	std::string m_name;
	int m_num_child;
	std::function<state(const std::vector<state> &p)> m_function;

	gp_node() {
	}

	gp_node(const std::string name, int num_child, std::function<state(const std::vector<state> &p)> fun) {
		m_name = name;
		m_num_child = num_child;
		m_function = fun;
	}

};
