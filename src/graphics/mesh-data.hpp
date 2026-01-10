#pragma once

template<typename V>
struct MeshData {

    vector<V> vertices;
    vector<uint16_t> indices;

    bool isEmpty() {
        return vertices.size() == 0 || indices.size() == 0;
    }

    void clear() {
        vertices.clear();
        indices.clear();
    }

};