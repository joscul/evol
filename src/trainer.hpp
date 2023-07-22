//
//  trainer.hpp
//  evol
//
//  Created by Josef Cullhed on 2023-07-19.
//

#pragma once

#include <vector>
#include "gp_tree.hpp"

template <typename state>
class trainer {
	
public:
	
	const double m_prob_mutation = 0.2;
	const double m_prob_crossover = 0.8;
	
	int m_num_params;
	int m_max_width;
	int m_max_depth;
	
	std::vector<gp_node<state>> m_nodes;
	
	gp_tree<state> m_best_individual;
	double m_best_score;
	std::vector<gp_tree<state>> m_last_generation;
	std::vector<double> m_last_scores;
	
	trainer(int num_params, int max_width, int max_depth, const std::vector<gp_node<state>> &nodes) {
		m_num_params = num_params;
		m_max_width = max_width;
		m_max_depth = max_depth;
		m_nodes = nodes;
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
                mutated.mutate(m_max_depth, m_max_width, m_nodes);
                next_generation.emplace_back(std::move(mutated));
            }

			auto parent1 = m_last_generation[permutation[uniform::uniform_int(0, percentile_10 - 1)]];
			auto parent2 = m_last_generation[permutation[uniform::uniform_int(0, percentile_10 - 1)]];

			parent1.crossover(m_prob_crossover, parent2);

			next_generation.emplace_back(std::move(me));
			next_generation.emplace_back(std::move(parent1));
			next_generation.emplace_back(std::move(parent2));
		}
		
		m_last_generation = std::move(next_generation);
		m_last_scores.resize(m_last_generation.size());
	}
	
	void train(int population_size, int num_generations, std::function<double(const gp_tree<state> &)> utility_function) {
		for (int i = 0; i < num_generations; i++) {
			std::cout << "running gen " << i << std::endl;
			run_generation(population_size, utility_function);
			if (m_best_score == 0) break;
		}

	}
	
	gp_tree<state> get_best() {
		return m_best_individual;
	}

};
