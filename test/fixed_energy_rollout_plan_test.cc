#include "fixed_energy_api.hh"

#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace {

bool expect(const bool condition, const std::string &message) {
  if (!condition) {
    std::cerr << "FAILED: " << message << std::endl;
    return false;
  }
  return true;
}

bool run_registry_check() {
  bool ok = true;
  const auto &states = cparty::internal::fixed_energy_target_states();
  const auto &plan = cparty::internal::fixed_energy_rollout_plan();

  ok &= expect(states.size() == 16, "target state count must be 16");
  ok &= expect(plan.size() == 16, "rollout plan entry count must be 16");

  std::set<std::string> unique_states(states.begin(), states.end());
  ok &= expect(unique_states.size() == states.size(), "target states must be unique");

  std::set<std::string> plan_states;
  for (const auto &entry : plan) {
    plan_states.insert(entry.state);
  }

  for (const auto &state : states) {
    ok &= expect(plan_states.count(state) == 1,
                 "each target state must appear exactly once in rollout plan: " + state);
  }
  for (const auto &state : plan_states) {
    ok &= expect(unique_states.count(state) == 1,
                 "rollout plan contains unknown state: " + state);
  }
  return ok;
}

bool run_slice_check(const std::string &slice) {
  bool ok = true;
  const auto &plan = cparty::internal::fixed_energy_rollout_plan();

  const std::vector<std::string> expected = [&]() {
    if (slice == "a") {
      return std::vector<std::string>{"W", "WI", "V"};
    }
    if (slice == "b") {
      return std::vector<std::string>{"VM", "WM", "WMv", "WMp"};
    }
    if (slice == "c") {
      return std::vector<std::string>{"WIP", "VP", "VPL", "VPR"};
    }
    if (slice == "d") {
      return std::vector<std::string>{"WMB", "WMBP", "WMBW", "BE"};
    }
    return std::vector<std::string>{};
  }();

  const std::string expected_story = [&]() {
    if (slice == "a") {
      return "014";
    }
    if (slice == "b") {
      return "015";
    }
    if (slice == "c") {
      return "016";
    }
    if (slice == "d") {
      return "017";
    }
    return "";
  }();

  ok &= expect(!expected.empty(), "slice must be one of: a, b, c, d");

  for (const auto &state : expected) {
    const auto it = std::find_if(plan.begin(), plan.end(), [&](const auto &entry) {
      return entry.state == state;
    });
    ok &= expect(it != plan.end(), "state missing in rollout plan: " + state);
    if (it != plan.end()) {
      ok &= expect(it->story_id == expected_story,
                   "state/story mismatch for " + state + ": expected " + expected_story +
                       ", got " + it->story_id);
    }
  }

  return ok;
}

}  // namespace

int main(int argc, char **argv) {
  if (argc == 2) {
    return run_slice_check(argv[1]) ? 0 : 1;
  }
  return run_registry_check() ? 0 : 1;
}
