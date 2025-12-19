#include <iostream>
#include <numeric>
#include <vector>

int main() {
  std::vector<std::vector<long int>> adj{{1}, {0, 2}, {1}, {4}, {3}};
  std::vector<long int> components(adj.size());
  std::iota(components.begin(), components.end(), 0);

  std::vector<long int> curr_level{0};
  std::vector<long int> next_level{};
  std::vector<bool> visited(adj.size(), false);

  long int u{};

  while (!curr_level.empty()) {
    u = curr_level.back();
    std::cout << "u = " << u << "\n";
    curr_level.pop_back();
    for (long int v : adj[u]) {
      std::cout << "   testing " << v << "\n";
      if (!visited[v]) {
        std::cout << "    visiting " << v << "\n";
        visited[v] = true;
        components[v] = components[u];
        std::cout << "    components[" << v << "] = " << components[u] << "\n";
        next_level.push_back(v);
      }
      std::cout << "swapping\n";
      std::swap(next_level, curr_level);
      next_level.clear();
    }
  }
}
