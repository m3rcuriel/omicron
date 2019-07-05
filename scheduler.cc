#include "scheduler.h"

namespace chess {

namespace agent {

double Task::operator()() {
    double total_reward = 0;
    for (int i = 0; i < num_rollouts; i++) {
        total_reward += root_node->simulate(depth);
    }
    return total_reward;
}

}

}
