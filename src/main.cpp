#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

struct Pos {
    int r;
    int c;

    bool operator==(const Pos& other) const {
        return r == other.r && c == other.c;
    }
};

struct PosHash {
    std::size_t operator()(const Pos& p) const {
        return std::hash<int>{}(p.r * 10007 + p.c);
    }
};

struct SearchResult {
    bool found = false;
    std::vector<Pos> path;
    int visited_nodes = 0;
    double elapsed_ms = 0.0;
};

class GridMap {
public:
    explicit GridMap(const std::string& file_path) {
        std::ifstream ifs(file_path);
        if (!ifs) {
            throw std::runtime_error("Failed to open map file: " + file_path);
        }

        std::string line;
        while (std::getline(ifs, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            if (!line.empty()) {
                grid_.push_back(line);
            }
        }

        if (grid_.empty()) {
            throw std::runtime_error("Map file is empty.");
        }

        const std::size_t width = grid_[0].size();
        for (const auto& row : grid_) {
            if (row.size() != width) {
                throw std::runtime_error("All rows in the map must have the same width.");
            }
        }

        bool has_start = false;
        bool has_goal = false;
        for (int r = 0; r < static_cast<int>(grid_.size()); ++r) {
            for (int c = 0; c < static_cast<int>(grid_[r].size()); ++c) {
                if (grid_[r][c] == 'S') {
                    start_ = {r, c};
                    has_start = true;
                }
                if (grid_[r][c] == 'G') {
                    goal_ = {r, c};
                    has_goal = true;
                }
            }
        }

        if (!has_start || !has_goal) {
            throw std::runtime_error("Map must contain exactly one S and one G.");
        }
    }

    const Pos& start() const { return start_; }
    const Pos& goal() const { return goal_; }
    int rows() const { return static_cast<int>(grid_.size()); }
    int cols() const { return static_cast<int>(grid_[0].size()); }

    bool in_bounds(const Pos& p) const {
        return 0 <= p.r && p.r < rows() && 0 <= p.c && p.c < cols();
    }

    bool passable(const Pos& p) const {
        return grid_[p.r][p.c] != '#';
    }

    std::vector<Pos> neighbors(const Pos& p) const {
        static const std::vector<Pos> dirs{{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
        std::vector<Pos> result;
        for (const auto& d : dirs) {
            Pos next{p.r + d.r, p.c + d.c};
            if (in_bounds(next) && passable(next)) {
                result.push_back(next);
            }
        }
        return result;
    }

    void print_with_path(const std::vector<Pos>& path) const {
        auto view = grid_;
        for (const auto& p : path) {
            if (!(p == start_) && !(p == goal_)) {
                view[p.r][p.c] = '*';
            }
        }

        for (const auto& row : view) {
            std::cout << row << '\n';
        }
    }

private:
    std::vector<std::string> grid_;
    Pos start_{};
    Pos goal_{};
};

std::vector<Pos> reconstruct_path(const std::unordered_map<Pos, Pos, PosHash>& parent, Pos start, Pos goal) {
    std::vector<Pos> path;
    if (parent.find(goal) == parent.end() && !(goal == start)) {
        return path;
    }

    Pos cur = goal;
    path.push_back(cur);
    while (!(cur == start)) {
        cur = parent.at(cur);
        path.push_back(cur);
    }
    std::reverse(path.begin(), path.end());
    return path;
}

SearchResult run_bfs(const GridMap& map) {
    auto t0 = std::chrono::steady_clock::now();

    std::queue<Pos> q;
    std::unordered_map<Pos, Pos, PosHash> parent;
    std::unordered_map<Pos, bool, PosHash> visited;

    q.push(map.start());
    visited[map.start()] = true;

    int visited_nodes = 0;

    while (!q.empty()) {
        Pos cur = q.front();
        q.pop();
        ++visited_nodes;

        if (cur == map.goal()) {
            auto t1 = std::chrono::steady_clock::now();
            return {
                true,
                reconstruct_path(parent, map.start(), map.goal()),
                visited_nodes,
                std::chrono::duration<double, std::milli>(t1 - t0).count()
            };
        }

        for (const auto& next : map.neighbors(cur)) {
            if (!visited[next]) {
                visited[next] = true;
                parent[next] = cur;
                q.push(next);
            }
        }
    }

    auto t1 = std::chrono::steady_clock::now();
    return {false, {}, visited_nodes, std::chrono::duration<double, std::milli>(t1 - t0).count()};
}

SearchResult run_dijkstra(const GridMap& map) {
    auto t0 = std::chrono::steady_clock::now();

    struct Node {
        int cost;
        Pos pos;
        bool operator>(const Node& other) const {
            return cost > other.cost;
        }
    };

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
    std::unordered_map<Pos, int, PosHash> dist;
    std::unordered_map<Pos, Pos, PosHash> parent;

    dist[map.start()] = 0;
    pq.push({0, map.start()});

    int visited_nodes = 0;

    while (!pq.empty()) {
        Node cur = pq.top();
        pq.pop();

        if (cur.cost > dist[cur.pos]) {
            continue;
        }

        ++visited_nodes;

        if (cur.pos == map.goal()) {
            auto t1 = std::chrono::steady_clock::now();
            return {
                true,
                reconstruct_path(parent, map.start(), map.goal()),
                visited_nodes,
                std::chrono::duration<double, std::milli>(t1 - t0).count()
            };
        }

        for (const auto& next : map.neighbors(cur.pos)) {
            int next_cost = cur.cost + 1;
            if (!dist.count(next) || next_cost < dist[next]) {
                dist[next] = next_cost;
                parent[next] = cur.pos;
                pq.push({next_cost, next});
            }
        }
    }

    auto t1 = std::chrono::steady_clock::now();
    return {false, {}, visited_nodes, std::chrono::duration<double, std::milli>(t1 - t0).count()};
}

int manhattan(const Pos& a, const Pos& b) {
    return std::abs(a.r - b.r) + std::abs(a.c - b.c);
}

SearchResult run_astar(const GridMap& map) {
    auto t0 = std::chrono::steady_clock::now();

    struct Node {
        int f;
        int g;
        Pos pos;
        bool operator>(const Node& other) const {
            if (f != other.f) return f > other.f;
            return g > other.g;
        }
    };

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
    std::unordered_map<Pos, int, PosHash> g_score;
    std::unordered_map<Pos, Pos, PosHash> parent;

    g_score[map.start()] = 0;
    pq.push({manhattan(map.start(), map.goal()), 0, map.start()});

    int visited_nodes = 0;

    while (!pq.empty()) {
        Node cur = pq.top();
        pq.pop();

        if (cur.g > g_score[cur.pos]) {
            continue;
        }

        ++visited_nodes;

        if (cur.pos == map.goal()) {
            auto t1 = std::chrono::steady_clock::now();
            return {
                true,
                reconstruct_path(parent, map.start(), map.goal()),
                visited_nodes,
                std::chrono::duration<double, std::milli>(t1 - t0).count()
            };
        }

        for (const auto& next : map.neighbors(cur.pos)) {
            int tentative_g = cur.g + 1;
            if (!g_score.count(next) || tentative_g < g_score[next]) {
                g_score[next] = tentative_g;
                parent[next] = cur.pos;
                int f = tentative_g + manhattan(next, map.goal());
                pq.push({f, tentative_g, next});
            }
        }
    }

    auto t1 = std::chrono::steady_clock::now();
    return {false, {}, visited_nodes, std::chrono::duration<double, std::milli>(t1 - t0).count()};
}

void print_usage(const char* app_name) {
    std::cout << "Usage: " << app_name << " <map-file> <algorithm>\n"
              << "  algorithm: bfs | dijkstra | astar\n";
}

int main(int argc, char* argv[]) {
    try {
        if (argc != 3) {
            print_usage(argv[0]);
            return 1;
        }

        const std::string map_file = argv[1];
        const std::string algorithm = argv[2];

        GridMap map(map_file);
        SearchResult result;

        if (algorithm == "bfs") {
            result = run_bfs(map);
        } else if (algorithm == "dijkstra") {
            result = run_dijkstra(map);
        } else if (algorithm == "astar") {
            result = run_astar(map);
        } else {
            print_usage(argv[0]);
            return 1;
        }

        std::cout << "Algorithm     : " << algorithm << "\n";
        std::cout << "Path found    : " << (result.found ? "yes" : "no") << "\n";
        std::cout << "Visited nodes : " << result.visited_nodes << "\n";
        std::cout << "Path length   : " << (result.found ? static_cast<int>(result.path.size()) - 1 : -1) << "\n";
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "Elapsed time  : " << result.elapsed_ms << " ms\n\n";

        if (result.found) {
            map.print_with_path(result.path);
        } else {
            std::cout << "No path found.\n";
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
