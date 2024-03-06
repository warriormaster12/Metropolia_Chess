#pragma once

#include <vector>
#include <unordered_map>

#include <webgpu/webgpu.h>

#include <GLFW/glfw3.h>

#include "model_loader.h"

class WindowSurface {
public:
    WindowSurface() = default;
    WindowSurface(WGPUInstance& p_instance,GLFWwindow* p_window);
    void setup_surface_format(WGPUAdapter& p_adapter) {m_swapchain_format = wgpuSurfaceGetPreferredFormat(m_surface, p_adapter);}
    void resize(WGPUDevice& p_device);
    void present() const {wgpuSwapChainPresent(m_swapchain);}
    void release(bool p_resizing_window);
    WGPUSurface& get_compatible_surface() {return m_surface;}
    WGPUTextureView get_current_swapchain_texture_view() const {return wgpuSwapChainGetCurrentTextureView(m_swapchain);}
    WGPUTextureView get_current_depth_texture_view() const {return m_depth_texture_view;}
    WGPUTextureFormat m_swapchain_format;
    WGPUTextureFormat m_depth_texture_format = WGPUTextureFormat_Depth24Plus;
    int current_resolution[2] = {0, 0};
private:
    WGPUSwapChain m_swapchain = nullptr;
    WGPUSurface m_surface = nullptr;
    WGPUTexture m_depth_texture = nullptr;
    WGPUTextureView m_depth_texture_view = nullptr;
    GLFWwindow* m_window = nullptr;

    void init_swapchain(WGPUDevice& p_device);
    void init_depth_buffer(WGPUDevice& p_device);
};

class Position;

class Camera {
public:
    Camera() = default;
    Camera(WGPUDevice& p_device);
    void update(int p_width, int p_height, WGPUQueue &p_queue, const Position& p_pos);
    WGPUBuffer buffer;
    float fov = 90;
    float zNear = 0.1;
    float zFar = 10;
private:
    float m_last_aspect_ratio = 0.0;
    int m_last_turn = -1;
};


struct FrameInfo {
    WGPUCommandEncoder encoder;
    WGPURenderPassEncoder render_pass;
    WGPUTextureView next_texture;
};

struct RenderPipeline {
    std::string id;
    WGPURenderPipeline pipeline;
    WGPUBindGroup material_group;
    std::vector<int> mesh_node_indexes;
};

class Move;

class Renderer {
public:
    Renderer(int width, int height);
    bool is_active() const;
    FrameInfo prepare_frame(const Position& p_pos, const bool moved);
    void render_board(FrameInfo& p_info);
    // only called at the end of the application
    void destroy();
private:
    void load_resources();
    bool init_gui() const;
    void terminate_gui() const;
    bool m_initialized = false;
    bool m_gui_enabled = false;
    GLFWwindow* m_window = nullptr;
    WindowSurface m_surface = {};
    WGPUInstance m_instance = nullptr;
    WGPUAdapter m_adapter = nullptr;
    WGPUDevice m_device = nullptr;
    WGPUQueue m_graphics_queue = nullptr;
    WGPUBindGroup m_chess_camera_bind_group = nullptr;
    WGPUBindGroup m_chess_model_bind_group = nullptr;
    std::unordered_map<std::string, size_t> m_vertex_offsets = {};
    std::unordered_map<std::string, size_t> m_index_offsets = {};
    LoaderInfo m_reference_mesh_load_info; // starting state
    std::vector<MeshNode> m_drawable_meshes = {};
    std::vector<RenderPipeline> m_render_pipelines = {};
    WGPUBuffer m_global_vertex_buffer = nullptr;
    WGPUBuffer m_global_index_buffer = nullptr;
    WGPUBuffer m_model_matrix_buffer = nullptr;
    WGPUSampler m_global_sampler = nullptr;
    Camera m_camera = {};
    uint32_t m_storage_alignment = 0;
};
