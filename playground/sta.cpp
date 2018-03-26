#include <queue>
#include <boost/range/adaptor/reversed.hpp>
#include <ectl_utils.h>
#include "sta.h"

using namespace ECTL;

static const int INF = 1000000;

struct T_Node {
    Node node;
    int max_delay_to_sink;

    T_Node(Node node, int max_delay_to_sink) {
        this->node = node;
        this->max_delay_to_sink = max_delay_to_sink;
    }
};

struct PartialPath {
    std::vector<Node> path;
    int max_delay;

    PartialPath(Node node, int max_delay,
                std::vector<Node> path = std::vector<Node>()) {
        this->path = std::move(path);
        this->path.emplace_back(node);
        this->max_delay = max_delay;
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
        double max_flow = MaxFlow(S, T);
        std::vector<Edge> min_cut;
        DFS_ResidualNetwork(S);
        for (auto e:E)
            if (e.cap > 0 && e.flow > 0 && res_visited[e.u] && !res_visited[e.v])
                min_cut.push_back(e);
        return min_cut;
    }
};

static std::vector<Node> GetSortedNodes(Network ntk) {
    std::vector<Node> sorted_internal_nodes = TopologicalSort(ntk);
    std::vector<Node> sorted_nodes = GetPrimaryInputs(ntk);
    sorted_nodes.insert(sorted_nodes.end(),
                        std::make_move_iterator(sorted_internal_nodes.begin()),
                        std::make_move_iterator(sorted_internal_nodes.end()));
    return sorted_nodes;
}

static void SetFanouts(Node node, std::vector<Node> fan_outs) {
    // generate the fanin order
    using namespace abc;
    Vec_Int_t *vFanouts;
    vFanouts = Vec_IntAlloc(100);
    Vec_IntClear(vFanouts);
    for (auto fan_out : fan_outs)
        Vec_IntPush(vFanouts, GetNodeID(fan_out));
    Vec_IntClear(&node->vFanouts);
    Vec_IntAppend(&node->vFanouts, vFanouts);
}

std::map<Node, int> CalculateSlack(const Network ntk) {
    std::vector<Node> sorted_nodes = GetSortedNodes(ntk);

    std::map<Node, int> arrival_time;
    std::map<Node, int> required_time;
    std::map<Node, int> slack;
    /* Initialization */
    for (auto node : sorted_nodes) {
        arrival_time.insert(std::pair<Node, int>(node, -1 * INF));
        required_time.insert(std::pair<Node, int>(node, INF));
        slack.insert(std::pair<Node, int>(node, 0));
    }
    int max_at = -1 * INF;
    /* Update at */
    for (auto node : sorted_nodes) {
        if (IsPrimaryInput(node))
            arrival_time.at(node) = 1;
        else
            for (const auto fan_in:GetFanins(node))
                arrival_time.at(node) = std::max(arrival_time.at(node), arrival_time.at(fan_in) + 1);
        if (arrival_time.at(node) > max_at)
            max_at = arrival_time.at(node);
    }
    /* Update rat */
    for (auto node : sorted_nodes)
        if (IsPrimaryOutputNode(node))
            required_time.at(node) = max_at;
    for (Node &node : boost::adaptors::reverse(sorted_nodes)) {
        for (const auto fan_out:GetFanouts(node))
            if (!IsPrimaryOutput(fan_out))
                required_time.at(node) = std::min(required_time.at(node), required_time.at(fan_out) - 1);
    }
    /* Update slack */
    for (auto node:sorted_nodes) {
//        std::cout << GetNodeName(node) << "=" << required_time.at(node) << "-" << arrival_time.at(node) << " ";
        slack.at(node) = required_time.at(node) - arrival_time.at(node);
    }
//    std::cout << std::endl;
    return slack;
}

void KMostCriticalPaths(const Network ntk, int k, bool show_slack) {
    std::map<Node, int> slack = CalculateSlack(ntk);
    std::vector<Node> sorted_nodes = GetSortedNodes(ntk);
    std::map<Node, int> max_delay_to_sink;
    /* Computation of Maximum Delays to Sink */
    for (auto node : sorted_nodes) {
        max_delay_to_sink.insert(std::pair<Node, int>(node, 0));
        if (IsPrimaryOutputNode(node))
            max_delay_to_sink.at(node) = 1;
    }
    for (Node &node : boost::adaptors::reverse(sorted_nodes)) {
        for (auto fan_out:GetFanouts(node))
            if (!IsPrimaryOutput(fan_out))
                max_delay_to_sink.at(node) = std::max(
                        max_delay_to_sink.at(node),
                        max_delay_to_sink.at(fan_out) + 1);
    }

    /* Sorting the Successors of Each Vertex */
    auto t_comp = [](T_Node a, T_Node b) {
        return a.max_delay_to_sink > b.max_delay_to_sink;
    };

    for (auto node: sorted_nodes) {
        std::vector<T_Node> t_fan_outs;
        std::vector<Node> fan_outs;

        for (const auto &fan_out:GetFanouts(node))
            if (!IsPrimaryOutput(fan_out))
                t_fan_outs.emplace_back(T_Node(fan_out, max_delay_to_sink.at(fan_out)));
            else
                t_fan_outs.emplace_back(T_Node(fan_out, 0));

        std::sort(t_fan_outs.begin(), t_fan_outs.end(), t_comp);

        for (const auto &t_fan_out:t_fan_outs)
            fan_outs.emplace_back(t_fan_out.node);

        SetFanouts(node, fan_outs);
    }

    /* Path Enumeration */
    auto comp = [](PartialPath a, PartialPath b) { return a.max_delay < b.max_delay; };
    std::priority_queue<PartialPath, std::vector<PartialPath>, decltype(comp)> paths(comp);

    for (auto node : GetPrimaryInputs(ntk))
        paths.push(PartialPath(node, max_delay_to_sink.at(node)));

    while (!paths.empty() && k > 0) {
        PartialPath t_path = paths.top();
        paths.pop();
        if (IsPrimaryOutput(t_path.path.back())) {
            k--;
            std::cout << "Delay: " << t_path.max_delay << " ";
            t_path.path.pop_back();
            for (const auto &node : t_path.path) {
                std::cout << GetNodeName(node);
                if (show_slack)
                    std::cout << "=" << slack.at(node);
                std::cout << " ";
            }
            std::cout << std::endl;
        } else {
            Node node_t = t_path.path.back();
            for (const auto &successor : GetFanouts(node_t)) {
                if (!IsPrimaryOutput(successor))
                    paths.push(PartialPath(successor,
                                           (int) t_path.path.size() + max_delay_to_sink.at(successor),
                                           t_path.path));
                else
                    paths.push(PartialPath(successor,
                                           (int) t_path.path.size() + 0,
                                           t_path.path));
            }
        }
    }
}

std::vector<Path> GetCriticalPaths(Network ntk) {
    std::vector<Path> critical_paths;
    std::map<Node, int> slack = CalculateSlack(ntk);
    std::vector<Node> sorted_nodes = GetSortedNodes(ntk);
    std::map<Node, int> max_delay_to_sink;
    /* Computation of Maximum Delays to Sink */
    for (auto node : sorted_nodes) {
        max_delay_to_sink.insert(std::pair<Node, int>(node, 0));
        if (IsPrimaryOutputNode(node))
            max_delay_to_sink.at(node) = 1;
    }
    for (Node &node : boost::adaptors::reverse(sorted_nodes)) {
        for (auto fan_out:GetFanouts(node))
            if (!IsPrimaryOutput(fan_out))
                max_delay_to_sink.at(node) = std::max(
                        max_delay_to_sink.at(node),
                        max_delay_to_sink.at(fan_out) + 1);
    }

    /* Sorting the Successors of Each Vertex */
    auto t_comp = [](T_Node a, T_Node b) {
        return a.max_delay_to_sink > b.max_delay_to_sink;
    };

    for (auto node: sorted_nodes) {
        std::vector<T_Node> t_fan_outs;
        std::vector<Node> fan_outs;

        for (const auto &fan_out:GetFanouts(node))
            if (!IsPrimaryOutput(fan_out))
                t_fan_outs.emplace_back(T_Node(fan_out, max_delay_to_sink.at(fan_out)));
            else
                t_fan_outs.emplace_back(T_Node(fan_out, 0));

        std::sort(t_fan_outs.begin(), t_fan_outs.end(), t_comp);

        for (const auto &t_fan_out:t_fan_outs)
            fan_outs.emplace_back(t_fan_out.node);

        SetFanouts(node, fan_outs);
    }

    /* Path Enumeration */
    auto comp = [](PartialPath a, PartialPath b) { return a.max_delay < b.max_delay; };
    std::priority_queue<PartialPath, std::vector<PartialPath>, decltype(comp)> paths(comp);

    for (auto node : GetPrimaryInputs(ntk))
        paths.push(PartialPath(node, max_delay_to_sink.at(node)));

    int critical_path_delay = -1;
    while (!paths.empty()) {
        PartialPath t_path = paths.top();
        paths.pop();
        if (IsPrimaryOutput(t_path.path.back())) {
            if (t_path.max_delay >= critical_path_delay) {
                critical_path_delay = t_path.max_delay;
                t_path.path.pop_back();
                Path critical_path(t_path.path);
                critical_paths.emplace_back(critical_path);
            } else
                break;
        } else {
            Node node_t = t_path.path.back();
            for (const auto &successor : GetFanouts(node_t)) {
                if (!IsPrimaryOutput(successor))
                    paths.push(PartialPath(successor,
                                           (int) t_path.path.size() + max_delay_to_sink.at(successor),
                                           t_path.path));
                else
                    paths.push(PartialPath(successor,
                                           (int) t_path.path.size() + 0,
                                           t_path.path));
            }
        }
    }
    return critical_paths;
}

std::vector<Node> MinCut(const Network ntk, std::map<Node, double> error) {
    std::map<Node, int> slack = CalculateSlack(ntk);

    auto N = (int) abc::Abc_NtkObjNum(ntk) + 2;
    Dinic dinic(N * 2);
    bool is_connected[N][N];
    int source = 0, sink = N - 1;

    memset(is_connected, 0, sizeof(is_connected[0][0]) * N * N);

    for (auto node:GetSortedNodes(ntk))
        if (slack.at(node) == 0) {
            int u = GetNodeID(node);
            dinic.AddEdge(u, u + N, error.at(node));
        }

    std::vector<Path> critical_paths = GetCriticalPaths(ntk);

    for (const auto &path : critical_paths) {
        for (int i = 0; i < (int) path.nodes.size() - 1; i++) {
            int u = GetNodeID(path.nodes[i]);
            int v = GetNodeID(path.nodes[i + 1]);
            if (!is_connected[u][v]) {
                dinic.AddEdge(u + N, v, INF);
                is_connected[u][v] = true;
            }
        }
        int front = GetNodeID(path.nodes.front());
        if (!is_connected[source][front]) {
            dinic.AddEdge(source, front, INF);
            is_connected[source][front] = true;
        }

        int back = GetNodeID(path.nodes.back());
        if (!is_connected[back][sink]) {
            dinic.AddEdge(back + N, sink, INF);
            is_connected[back][sink] = true;
        }
    }

    std::cout << "Max Flow: " << dinic.MaxFlow(source, sink) << std::endl;
    std::vector<Edge> min_cut = dinic.MinCut(source, sink);
    std::vector<Node> min_cut_nodes;
    for (auto e:min_cut)
        min_cut_nodes.push_back(GetNodebyID(ntk, e.u));
    return min_cut_nodes;
}
