//
//  trainer.hpp
//  evol
//
//  Created by Josef Cullhed on 2023-07-19.
//

#pragma once

#include <vector>
#include "gp_node.hpp"

template <typename state>
class trainer {
    
public:
    int m_num_params;
    int m_max_width;
    int m_max_depth;
    
    std::vector<gp_node<state>> m_nodes;
    
};
