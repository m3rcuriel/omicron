#pragma once

#include "uct.h"

namespace chess {

namespace agent {

class Task {
public:
  Task(int depth, int num_rollouts) : depth(depth), num_rollouts(num_rollouts) {}
  double operator()();

private:
  OurUctNode* root_node;
  OurUctNode* parent_node;

  int depth, num_rollouts;
};

class Scheduler {
public:
  Scheduler(OurUctNode* root) : root(root) {}

  void run(int num_threads, int num_rollouts);

private:
  OurUctNode* root;
};

}

}
