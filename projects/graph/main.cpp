#include <functional>
#include <iomanip>
#include <iostream>
#include <queue>
#include <string>
#include <vector>

void* operator new(std::size_t count) {
  if (!count) {
    throw std::bad_alloc{};
  }

  if (void* ptr = std::malloc(count)) {
    std::printf("Allocated %lud bytes started at %p\n",
                (unsigned long) count,
                ptr);
    return ptr;
  }

  throw std::bad_alloc{};
}

void operator delete(void* ptr) noexcept {
  std::printf("Deallocated memory starting at %p\n", ptr);
  std::free(ptr);
}

struct Node {
  explicit Node(std::string name_) : name{std::move(name_)} {
    std::cout << "Node " << std::quoted(name) << " created\n";
  }

  ~Node() { std::cout << "Node " << std::quoted(name) << " destroyed\n"; }

  std::vector<Node*> edges;
  std::string name;
};

void FreeGraph(Node* graph) {
  std::deque<Node*> to_visit;
  std::vector<Node*> visited;
  to_visit.push_back(graph);

  while (!to_visit.empty()) {
    auto* node = to_visit.front();
    visited.push_back(node);
    to_visit.pop_front();

    std::cout << "Visiting " << std::quoted(node->name) << std::endl;

    for (auto* edge : node->edges) {
      const auto already_marked_to_visit =
        std::find(to_visit.begin(), to_visit.end(), edge) !=
        to_visit.end();

      const auto is_edge_visited =
        std::find(visited.begin(), visited.end(), edge) !=
        visited.end();

      if (is_edge_visited) {
        std::cout << "Edge " << std::quoted(edge->name)
                  << " has already been visited\n";
        continue;
      }

      if (!already_marked_to_visit) {
        std::cout << "Edge " << std::quoted(edge->name)
                  << " marked to visit" << std::endl;
        to_visit.push_back(edge);
      }
    }

    delete node;

    std::string to_visit_str;
    std::for_each(to_visit.begin(), to_visit.end(), [&](Node* node) {
      if (to_visit_str.empty()) {
        to_visit_str = node->name;
      } else {
        to_visit_str += ", " + node->name;
      }
    });

    if (!to_visit_str.empty()) {
      std::cout << "To visit " << std::quoted(to_visit_str) << std::endl;
    } else {
      std::cout << "All nodes were visited and deleted\n";
    }
  }
}

int main() {
  const auto a = new Node{"a"};
  const auto b = new Node{"b"};
  const auto c = new Node{"c"};
  const auto d = new Node{"d"};

  a->edges.push_back(a);
  a->edges.push_back(b);
  a->edges.push_back(c);
  a->edges.push_back(d);

  b->edges.push_back(a);
  b->edges.push_back(b);
  b->edges.push_back(c);
  b->edges.push_back(d);

  c->edges.push_back(a);
  c->edges.push_back(b);
  c->edges.push_back(c);
  c->edges.push_back(d);

  d->edges.push_back(a);
  d->edges.push_back(b);
  d->edges.push_back(c);
  d->edges.push_back(d);

  FreeGraph(a);
}
