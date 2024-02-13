#pragma once

#include <iostream>
#include <vector>
#include "glm.hpp"
#include <webgpu/webgpu.h>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct Material {
    std::string id;
    WGPUTexture base_color_texture;
    WGPUTextureView base_color_view;
};

class MeshNode {
public:
    std::string id;
    int chess_piece;
    glm::mat4x4 transform_matrix;
    std::vector<Vertex> verticies;
    std::vector<uint32_t> indices;
    std::string material_id;
    bool get_is_dirty() const {return m_is_dirty;}
    void reset_dirt() {m_is_dirty = false;}
    void set_position(const glm::vec3 position);
    glm::vec3 get_position() const {return m_position;}
private:
    bool m_is_dirty = true;
    glm::vec3 m_position;
};

namespace model_loader {
    std::pair<std::vector<Material>,std::vector<MeshNode>> load_gltf(const std::string& file, WGPUDevice& p_device);
}