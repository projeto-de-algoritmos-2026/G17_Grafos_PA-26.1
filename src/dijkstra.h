#pragma once
#include "heap.h"
#include <bits/stdc++.h>

using namespace std;

struct Edge {
    int from, to, weight;
};

struct DijkstraResult {
    vector<int> path;
    int totalCost;
};

DijkstraResult runDijkstra(int start, int target, int NUM_NODES, const vector<Edge>& edges) {
    vector<vector<pair<int,int>>> adj(NUM_NODES);
    for (auto& e : edges) {
        adj[e.from].push_back({e.to,   e.weight});
    }

    vector<int> dist(NUM_NODES, numeric_limits<int>::max());
    vector<int> parent(NUM_NODES, -1);

    MinHeap heap(NUM_NODES);

    for (int i = 0; i < NUM_NODES; i++) {
        heap.push({i, dist[i]});
    }

    dist[start] = 0;
    heap.decreaseKey(start, 0);

    while (!heap.empty()) {
        HeapNode current = heap.pop();

        if (current.cost == numeric_limits<int>::max()) break;
        if (current.id == target) break;

        for (auto [nb, w] : adj[current.id]) {
            if (!heap.contains(nb)) continue;

            int newCost = dist[current.id] + w;
            if (newCost < dist[nb]) {
                dist[nb]   = newCost;
                parent[nb] = current.id;

                heap.decreaseKey(nb, newCost);
            }
        }
    }

    DijkstraResult result;
    result.totalCost = dist[target];

    if (dist[target] == numeric_limits<int>::max()) return result;

    int curr = target;
    while (curr != -1) {
        result.path.push_back(curr);
        curr = parent[curr];
    }
    reverse(result.path.begin(), result.path.end());
    return result;
}