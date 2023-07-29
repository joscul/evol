//
//  trainer.hpp
//  evol
//
//  Created by Josef Cullhed on 2023-07-19.
//

#pragma once

#include <vector>
#include <execution>
#include "gp_tree.hpp"

template <typename state>
class trainer {
	
public:
	
	const double m_prob_mutation = 0.2;
	const double m_prob_crossover = 0.2;
	const double m_prob_mutate_constant = 0.2;
	
	int m_num_params;
	int m_max_width;
	int m_max_depth;
	
	std::vector<gp_node<state>> m_nodes;
	gp_node<state> m_null_node;
	std::map<long, std::unique_ptr<gp_tree<state>>> m_extrapolated;
	std::map<long, std::string> m_extrapolated_name;
	
	gp_tree<state> m_best_individual;
	double m_best_score;
	std::vector<gp_tree<state>> m_last_generation;
	std::vector<double> m_last_scores;
	
	trainer(int num_params, int max_width, int max_depth, const std::vector<gp_node<state>> &nodes, const gp_node<state> &null_node) {
		m_num_params = num_params;
		m_max_width = max_width;
		m_max_depth = max_depth;
		m_nodes = nodes;
		m_null_node = null_node;
	}
	
	void run_generation(int population_size, std::function<double(const gp_tree<state> &)>  utility_function) {
		
		while (m_last_generation.size() < population_size) {
			// Generate more random individuals.
			auto new_individual = gp_tree<state>::make_random_tree(m_num_params, m_max_depth, m_max_width, m_nodes);
			m_last_generation.emplace_back(std::move(new_individual));
			m_last_scores.push_back(0.0);
		}
		
		if (m_last_generation.size() != m_last_scores.size()) {
			throw std::runtime_error("Invalid size of last generation and last scores, should be the same");
		}
		
		// re-calculate all the scores.
		std::transform(m_last_generation.cbegin(), m_last_generation.cend(), m_last_scores.begin(), utility_function);
		
		// find permutation that sorts the population by lowest fitness first.
		std::vector<int> permutation(m_last_scores.size());
		std::iota(permutation.begin(), permutation.end(), 0);
		std::sort(permutation.begin(), permutation.end(), [&](int a, int b) {
			return m_last_scores[a] < m_last_scores[b];
		});
		
		// keep the top 10% and mutate them.
		std::vector<gp_tree<state>> next_generation;
		
		m_best_individual = m_last_generation[permutation[0]];
		m_best_score = m_last_scores[permutation[0]];

		const int percentile_10 = permutation.size() * 0.1;
		for (int i = 0; i < percentile_10; i++) {
			auto me = m_last_generation[permutation[i]];
			
			if (uniform::uniform_double() < m_prob_mutation) {
				auto mutated = m_last_generation[permutation[i]];
				mutated.mutate(m_max_depth - mutated.depth(), m_max_width, m_nodes);
				next_generation.emplace_back(std::move(mutated));
			}

			if (uniform::uniform_double() < m_prob_crossover) {
				auto parent1 = m_last_generation[permutation[uniform::uniform_int(0, percentile_10 - 1)]];
				auto parent2 = m_last_generation[permutation[uniform::uniform_int(0, percentile_10 - 1)]];

				parent1.crossover(parent2);
				next_generation.emplace_back(std::move(parent1));
				next_generation.emplace_back(std::move(parent2));
			}

			if (uniform::uniform_double() < m_prob_mutate_constant) {
				auto mutated = m_last_generation[permutation[i]];
				mutated.mutate_constant();
				next_generation.emplace_back(std::move(mutated));
			}

			// also just add the inidividual to the next generation.
			next_generation.emplace_back(std::move(me));
		}
		
		m_last_generation = std::move(next_generation);
		m_last_scores.resize(m_last_generation.size());

		int max_depth = std::transform_reduce(m_last_generation.cbegin(), m_last_generation.cend(), 0, [](auto a, auto b) {
			return std::max(a, b);
		}, [](auto a) {
			return a.depth();
		});
		std::cout << "max depth: " << max_depth << std::endl;
	}

	bool run_minifier(int population_size, std::function<double(const gp_tree<state> &)> utility_function) {
		
		// re-calculate all the scores.
		std::transform(m_last_generation.cbegin(), m_last_generation.cend(), m_last_scores.begin(), utility_function);
		
		// find permutation that sorts the population by lowest fitness first.
		std::vector<int> permutation(m_last_scores.size());
		std::iota(permutation.begin(), permutation.end(), 0);
		std::sort(permutation.begin(), permutation.end(), [&](int a, int b) {
			return m_last_scores[a] < m_last_scores[b];
		});
		
		const int percentile_10 = permutation.size() * 0.1;
		bool did_minify = false;
		for (int i = 0; i < percentile_10; i++) {
			auto me = &m_last_generation[permutation[i]];
			if (me->minify(m_last_scores[permutation[i]], utility_function, m_null_node)) {
				did_minify = true;
			}
		}
		return did_minify;
	}

	void run_node_extrapolation(int population_size, std::function<double(const gp_tree<state> &)> utility_function) {

		// re-calculate all the scores.
		std::transform(m_last_generation.cbegin(), m_last_generation.cend(), m_last_scores.begin(), utility_function);
		
		// find permutation that sorts the population by lowest fitness first.
		std::vector<int> permutation(m_last_scores.size());
		std::iota(permutation.begin(), permutation.end(), 0);
		std::sort(permutation.begin(), permutation.end(), [&](int a, int b) {
			return m_last_scores[a] < m_last_scores[b];
		});
		
		const int percentile_10 = permutation.size() * 0.1;
		std::map<long, int> hash_counts;
		std::map<long, const gp_tree<state> *> hash_maps;
		for (int i = 0; i < percentile_10; i++) {
			auto hmap = m_last_generation[permutation[i]].hash_map();
			for (const auto &iter : hmap) {
				hash_counts[iter.first]++;
				hash_maps[iter.first] = iter.second;
			}
		}

		// take out the most common hashes.
		std::vector<std::pair<long, int>> most_common;
		for (const auto &iter : hash_counts) {
			most_common.push_back(std::make_pair(iter.first, iter.second));
		}

		std::sort(most_common.begin(), most_common.end(), [](auto a, auto b) {
			return a.second > b.second;
		});

		for (const auto &pair : most_common) {
			if (pair.second < 5) continue;
			if (m_extrapolated.count(pair.first) == 0) {
				extrapolate_node(pair.first, hash_maps[pair.first]);
				break;
			}
		}
	}

	void extrapolate_node(long hash, const gp_tree<state> *tree) {
		std::cout << "Extrapolated node " << std::endl;
		auto node_name = "extrap_" + std::to_string(m_extrapolated.size());
		auto node_description = gp_tree<int>::tree_to_string(*tree);
		gp_node<state> extrapolated(node_name, 0, 0, [](auto node, auto p, auto c) {
			return node.m_tree->call(p);
		});
		m_extrapolated[hash] = std::make_unique<gp_tree<state>>(*tree);
		m_extrapolated_name[hash] = node_name;
		extrapolated.m_tree = m_extrapolated[hash].get();
		m_nodes.push_back(extrapolated);
	}

	void train(int population_size, int num_generations, std::function<double(const gp_tree<state> &)> utility_function) {
		for (int i = 0; i < num_generations; i++) {
			std::cout << "running gen " << i << std::endl;
			run_generation(population_size, utility_function);
			if (i % 10 == 0) {
				while (run_minifier(population_size, utility_function)) {
					run_minifier(population_size, utility_function);
				}
			}
			if (i % 10 == 0) run_node_extrapolation(population_size, utility_function);
			if (m_best_score == 0) break;
		}
		while (run_minifier(population_size, utility_function)) {
		}

		// re-calculate all the scores.
		std::transform(m_last_generation.cbegin(), m_last_generation.cend(), m_last_scores.begin(), utility_function);

		// find permutation that sorts the population by lowest fitness first.
		std::vector<int> permutation(m_last_scores.size());
		std::iota(permutation.begin(), permutation.end(), 0);
		std::sort(permutation.begin(), permutation.end(), [&](int a, int b) {
			return m_last_scores[a] < m_last_scores[b];
		});
		for (const auto &iter : m_extrapolated) {
			std::cout << m_extrapolated_name[iter.first] << std::endl;
			std::cout << gp_tree<state>::tree_to_string(*iter.second);
		}
		for (int i = 0; i < 1; i++) {
			auto me = &m_last_generation[permutation[i]];
			std::cout << gp_tree<int>::tree_to_string(*me);
			std::cout << "score: " << m_last_scores[permutation[i]] << std::endl << std::endl;
		}

	}

	gp_tree<state> get_best() {
		return m_best_individual;
	}

};
