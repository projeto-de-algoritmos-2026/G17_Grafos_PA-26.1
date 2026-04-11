#pragma once
#include <bits/stdc++.h>

using namespace std;


struct HeapNode {
    int id,cost;
};

struct MinHeap{
    vector <HeapNode> data; 
    vector  <int> pos;
    int numNodes;

    MinHeap(int n) : numNodes(n), pos(n, -1) {}

    bool empty() const {
        return data.empty();
    }

    void swapNodes(int i, int j) {
        pos[data[i].id] = j;
        pos[data[j].id] = i;
        swap(data[i], data[j]);
    }

    void heapifyUp(int i) {
        while (i > 0) {
            int parent = (i - 1) / 2;
            if (data[parent].cost > data[i].cost) {
                swapNodes(parent, i);
                i = parent;
            } else {
                break;
            }
        }
    }

    void  heapifyDown(int i){
        int n = data.size();
        while (true) {
            int left = 2*i + 1;
            int right = 2*i + 2; 
            int smallest = i;

            if (left < n && data[left].cost < data[smallest].cost) smallest = left;
            if (right < n && data[right].cost < data[smallest].cost) smallest = right;
            
            if (smallest != i) {
                swapNodes(i, smallest);
                i = smallest;
            } else {
                break;
            }
        }
    }

    void push(HeapNode node){

        pos[node.id] = data.size();
        data.push_back(node);
        heapifyUp(data.size() - 1);

    }

    HeapNode pop(){
        HeapNode minNode = data[0];
        pos[minNode.id] = -1;

        data[0] = data.back();
        data.pop_back();

        if (!data.empty()) {
            pos[data[0].id] = 0;
            heapifyDown(0);
        }

        return minNode;
    }

    // só chama se o novo custo for menor, toma cuidado quando for chamar no dijsktra lud <3 
    void decreaseKey(int id, int newCost) {
        int i = pos[id];
        data[i].cost = newCost;
        heapifyUp(i); 
    }

    bool contains(int id) const {
        return pos[id] != -1;
    }
};