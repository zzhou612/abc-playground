#include <queue>
#include <boost/range/adaptor/reversed.hpp>
#include "sta.h"


static const int INF = 1000000;

struct T_Node {
    ObjPtr node;
    int max_delay_to_sink;

    T_Node(ObjPtr node, int max_delay_to_sink) : node(std::move(node)), max_delay_to_sink(max_delay_to_sink) {}
};

struct PartialPath {
    std::vector<ObjPtr> path;
    int max_delay;

    PartialPath(ObjPtr node, int max_delay,
                std::vector<ObjPtr> path = std::vector<ObjPtr>()) : path(std::move(path)), max_delay(max_delay) {
        this->path.emplace_back(node);
    }
};

struct Edge {
    int u, v;
    double cap, flow;

    Edge() = default;

    Edge(int u, int v, double cap) : u(u), v(v), cap(cap), flow(0) {}
};

struct Dinic {
    int N;
    std::vector<Edge> E;
    std::vector<std::vector<int>> g;
    std::vector<int> d, pt;
    std::vector<bool> res_visited;

    explicit Dinic(int N) : N(N), E(0), g(N), d(N), pt(N), res_visited(N, false) {}

    void AddEdge(int u, int v, double cap) {
        if (u != v) {
            E.emplace_back(Edge(u, v, cap));
            g[u].emplace_back(E.size() - 1);
            E.emplace_back(Edge(v, u, 0));
            g[v].emplace_back(E.size() - 1);
        }
    }

    bool BFS(int S, int T) {
        std::queue<int> q({S});
        fill(d.begin(), d.end(), N + 1);
        d[S] = 0;
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            if (u == T) break;
            for (int k: g[u]) {
                Edge &e = E[k];
                if (e.flow < e.cap && d[e.v] > d[e.u] + 1) {
                    d[e.v] = d[e.u] + 1;
                    q.emplace(e.v);
                }
            }
        }
        return d[T] != N + 1;
    }

    double DFS(int u, int T, double flow = -1) {
        if (u == T || flow == 0) return flow;
        for (int &i = pt[u]; i < (int) g[u].size(); ++i) {
            Edge &e = E[g[u][i]];
            Edge &oe = E[g[u][i] ^ 1];
            if (d[e.v] == d[e.u] + 1) {
                double amt = e.cap - e.flow;
                if (flow != -1 && amt > flow) amt = flow;
                if (double pushed = DFS(e.v, T, amt)) {
                    e.flow += pushed;
                    oe.flow -= pushed;
                    return pushed;
                }
            }
        }
        return 0;
    }

    double MaxFlow(int S, int T) {
        double total = 0;
        while (BFS(S, T)) {
            fill(pt.begin(), pt.end(), 0);
            while (double flow = DFS(S, T))
                total += flow;
        }
        return total;
    }

    void DFS_ResidualNetwork(int u) {
        res_visited[u] = true;
        for (auto i:g[u]) {
            Edge &e = E[i];
            if (!res_visited[e.v]) {
                if (e.cap > 0 && e.flow > 0 && e.cap - e.flow > 0)
                    DFS_ResidualNetwork(e.v);
                else if (e.cap == 0 && e.flow < 0)
                    DFS_ResidualNetwork(e.v);
            }
        }
    }

    std::vector<Edge> MinCut(int S, int T) {
//        double max_flow = MaxFlow(S, T);
        MaxFlow(S, T);
        std::vector<Edge> min_cut;
        DFS_ResidualNetwork(S);
        for (auto e:E)
            if (e.cap > 0 && e.flow > 0 && res_visited[e.u] && !res_visited[e.v])
                min_cut.push_back(e);
        return min_cut;
    }
};

std::unordered_map<ObjPtr, TimeObject> CalculateSlack(const NtkPtr &ntk, bool print_result) {
    std::vector<ObjPtr> sorted_nodes = TopologicalSort(ntk);

    std::unordered_map<ObjPtr, TimeObject> t_objs;

    /* Initialization */
    for (auto node : sorted_nodes)
        t_objs.emplace(node, TimeObject(-1 * INF, INF, 0));

    int max_at = -1 * INF;

    /* Update at */
    for (const auto &node : sorted_nodes) {
        if (node->IsPrimaryInput())
            t_objs.at(node).arrival_time = 1;
        else
            for (const auto &fan_in : node->GetFanins())
                t_objs.at(node).arrival_time = std::max(t_objs.at(node).arrival_time,
                                                        t_objs.at(fan_in).arrival_time + 1);
        if (t_objs.at(node).arrival_time > max_at)
            max_at = t_objs.at(node).arrival_time;
    }

    /* Update rat */
    for (const auto &node : sorted_nodes)
        if (node->IsPrimaryOutputNode())
            t_objs.at(node).required_time = max_at;

    for (const auto &node : boost::adaptors::reverse(sorted_nodes)) {
        for (const auto &fan_out : node->GetFanouts())
            if (!fan_out->IsPrimaryOutput() && t_objs.find(fan_out) != t_objs.end()) {
                t_objs.at(node).required_time = std::min(t_objs.at(node).required_time,
                                                         t_objs.at(fan_out).required_time - 1);
            }
    }

    /* Update slack */
    for (const auto &node:sorted_nodes) {
        if (print_result)
            std::cout << node->GetName() << "=" << t_objs.at(node).required_time << "-" << t_objs.at(node).arrival_time
                      << "=" << t_objs.at(node).slack << " ";
        t_objs.at(node).slack = t_objs.at(node).required_time - t_objs.at(node).arrival_time;
    }
    if (print_result)
        std::cout << std::endl;
    return t_objs;
}

int PrintKMostCriticalPaths(const NtkPtr &ntk, int k, bool show_slack) {
    int delay = -1;
    std::unordered_map<ObjPtr, TimeObject> time_man = CalculateSlack(ntk);
    std::vector<ObjPtr> sorted_nodes = TopologicalSort(ntk);
    std::unordered_map<ObjPtr, int> max_delay_to_sink;

    /* Computation of Maximum Delays to Sink */
    for (const auto &node : sorted_nodes) {
        max_delay_to_sink.emplace(node, 0);
        if (node->IsPrimaryOutputNode())
            max_delay_to_sink.at(node) = 1;
    }
    for (const auto &node : boost::adaptors::reverse(sorted_nodes)) {
        for (const auto &fan_out : node->GetFanouts())
            if (!fan_out->IsPrimaryOutput() && max_delay_to_sink.find(fan_out) != max_delay_to_sink.end())
                max_delay_to_sink.at(node) = std::max(
                        max_delay_to_sink.at(node),
                        max_delay_to_sink.at(fan_out) + 1);
    }

    /* Path Enumeration */
    auto comp = [](PartialPath a, PartialPath b) { return a.max_delay < b.max_delay; };

    std::priority_queue<PartialPath, std::vector<PartialPath>, decltype(comp)> paths(comp);

    for (auto node : ntk->GetPIs())
        paths.emplace(node, max_delay_to_sink.at(node));

    while (!paths.empty() && k > 0) {
        PartialPath t_path = paths.top();
        paths.pop();
        if (t_path.path.back()->IsPrimaryOutput()) {
            k--;
            delay = std::max(delay, t_path.max_delay);
            std::cout << "Delay: " << t_path.max_delay << " ";
            t_path.path.pop_back();
            for (const auto &node : t_path.path) {
                std::cout << node->GetName();
                if (show_slack)
                    std::cout << "=" << time_man.at(node).slack;
                std::cout << " ";
            }
            std::cout << std::endl;
        } else {
            ObjPtr node_t = t_path.path.back();
            for (const auto &successor : node_t->GetFanouts()) {
                if (!successor->IsPrimaryOutput() && max_delay_to_sink.find(successor) != max_delay_to_sink.end())
                    paths.emplace(successor,
                                  (int) t_path.path.size() + max_delay_to_sink.at(successor),
                                  t_path.path);
                else
                    paths.emplace(successor,
                                  (int) t_path.path.size() + 0,
                                  t_path.path);
            }
        }
    }
    return delay;
}

std::vector<Path> GetKMostCriticalPaths(const NtkPtr &ntk, int k) {
    int critical_path_delay = -1;
    std::unordered_map<ObjPtr, TimeObject> time_man = CalculateSlack(ntk);
    std::vector<ObjPtr> sorted_nodes = TopologicalSort(ntk);
    std::unordered_map<ObjPtr, int> max_delay_to_sink;
    std::vector<Path> critical_paths;


    /* Computation of Maximum Delays to Sink */
    for (const auto &node : sorted_nodes) {
        max_delay_to_sink.emplace(node, 0);
        if (node->IsPrimaryOutputNode())
            max_delay_to_sink.at(node) = 1;
    }
    for (const auto &node : boost::adaptors::reverse(sorted_nodes)) {
        for (const auto &fan_out : node->GetFanouts())
            if (!fan_out->IsPrimaryOutput() && max_delay_to_sink.find(fan_out) != max_delay_to_sink.end())
                max_delay_to_sink.at(node) = std::max(
                        max_delay_to_sink.at(node),
                        max_delay_to_sink.at(fan_out) + 1);
    }

    /* Path Enumeration */
    auto comp = [](PartialPath a, PartialPath b) { return a.max_delay < b.max_delay; };

    std::priority_queue<PartialPath, std::vector<PartialPath>, decltype(comp)> paths(comp);

    for (auto node : ntk->GetPIs())
        paths.emplace(node, max_delay_to_sink.at(node));

    while (!paths.empty() && (k == -1 || k > 0)) {
        PartialPath t_path = paths.top();
        paths.pop();
        if (t_path.path.back()->IsPrimaryOutput()) {
            if (k != -1)
                k--;
            if (t_path.max_delay >= critical_path_delay) {
                critical_path_delay = t_path.max_delay;
                t_path.path.pop_back();
                Path critical_path(t_path.path);
                critical_paths.emplace_back(critical_path);
            } else
                break;
        } else {
            ObjPtr node_t = t_path.path.back();
            for (const auto &successor : node_t->GetFanouts()) {
                if (!successor->IsPrimaryOutput() && max_delay_to_sink.find(successor) != max_delay_to_sink.end())
                    paths.emplace(successor,
                                  (int) t_path.path.size() + max_delay_to_sink.at(successor),
                                  t_path.path);
                else
                    paths.emplace(successor,
                                  (int) t_path.path.size() + 0,
                                  t_path.path);
            }
        }
    }
    return critical_paths;
}

std::vector<ObjPtr> MinCut(const NtkPtr &ntk, const std::unordered_map<ObjPtr, double> &node_error) {
    auto time_objs = CalculateSlack(ntk);
    auto N = abc::Abc_NtkObjNum(ntk->_Get_Abc_Ntk()) + 2;
    Dinic dinic(N * 2);
    bool is_connected[N][N];
    int source = 0, sink = N - 1;

    memset(is_connected, 0, sizeof(is_connected[0][0]) * N * N);

    for (const auto &node : TopologicalSort(ntk))
        if (time_objs.at(node).slack == 0) {
            int u = node->GetID();
            if (node->IsPrimaryInput())
                dinic.AddEdge(u, u + N, INF);
            else
                dinic.AddEdge(u, u + N, node_error.at(node));
        }

    for (const auto &path : GetKMostCriticalPaths(ntk)) {
        for (int i = 0; i < (int) path.nodes.size() - 1; i++) {
            int u = path.nodes[i]->GetID();
            int v = path.nodes[i + 1]->GetID();
            if (!is_connected[u][v]) {
                dinic.AddEdge(u + N, v, INF);
                is_connected[u][v] = true;
            }
        }
        int front = path.nodes.front()->GetID();
        if (!is_connected[source][front]) {
            dinic.AddEdge(source, front, INF);
            is_connected[source][front] = true;
        }

        int back = path.nodes.back()->GetID();
        if (!is_connected[back][sink]) {
            dinic.AddEdge(back + N, sink, INF);
            is_connected[back][sink] = true;
        }
    }

    std::cout << "Max Flow: " << dinic.MaxFlow(source, sink) << std::endl;
    std::vector<Edge> min_cut = dinic.MinCut(source, sink);
    std::vector<ObjPtr> min_cut_nodes;
    for (auto e:min_cut)
        min_cut_nodes.push_back(ntk->GetObjbyID(e.u));
    return min_cut_nodes;
}
