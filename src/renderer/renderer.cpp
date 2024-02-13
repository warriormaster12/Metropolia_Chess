#include "renderer.h"
#include "ext/matrix_clip_space.hpp"
#include "ext/matrix_transform.hpp"
#include "fwd.hpp"
#include "glfw3webgpu.h"
#include "renderer/model_loader.h"
#include <imgui.h>
#include <backends/imgui_impl_wgpu.h>
#include <backends/imgui_impl_glfw.h>
#include "renderer_utils.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <webgpu/webgpu.h>

#include "chess/move.h"
#include "chess/position.h"

WindowSurface::WindowSurface(WGPUInstance& p_instance,GLFWwindow* p_window){
    m_surface = glfwGetWGPUSurface(p_instance, p_window);
    m_window = p_window;
}

void WindowSurface::init_swapchain(WGPUDevice& p_device) {
    std::cout << "Creating swapchain device..." << std::endl;
    if (!m_window) {
        std::cout << "Failed to initialize WindowSurface swapchain\n GLFWWindow pointer is null" << std::endl;;
        return;
    }
    int width, height;
	glfwGetFramebufferSize(m_window, &width, &height);
    current_resolution[0] = width;
    current_resolution[1] = height;
	WGPUSwapChainDescriptor swapChainDesc = {};
	swapChainDesc.width = width;
	swapChainDesc.height = height;
	swapChainDesc.usage = WGPUTextureUsage_RenderAttachment;
    swapChainDesc.presentMode = WGPUPresentMode_Fifo;


    swapChainDesc.format = m_swapchain_format;

    m_swapchain = wgpuDeviceCreateSwapChain(p_device, m_surface, &swapChainDesc);
}

void WindowSurface::init_depth_buffer(WGPUDevice& p_device) {
    if (!m_window) {
        std::cout << "Failed to initialize WindowSurface depth buffer\n GLFWWindow pointer is null" << std::endl;;
        return;
    }
    int width, height;
	glfwGetFramebufferSize(m_window, &width, &height);

	// Create the depth texture
	WGPUTextureDescriptor depthTextureDesc = {};
	depthTextureDesc.dimension = WGPUTextureDimension_2D;
	depthTextureDesc.format = m_depth_texture_format;
	depthTextureDesc.mipLevelCount = 1;
	depthTextureDesc.sampleCount = 1;
	depthTextureDesc.size = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };
	depthTextureDesc.usage = WGPUTextureUsage_RenderAttachment;
	depthTextureDesc.viewFormatCount = 1;
	depthTextureDesc.viewFormats = (WGPUTextureFormat*)&m_depth_texture_format;
	m_depth_texture = wgpuDeviceCreateTexture(p_device, &depthTextureDesc);
	std::cout << "Depth texture: " << m_depth_texture << std::endl;

	// Create the view of the depth texture manipulated by the rasterizer
	WGPUTextureViewDescriptor depthTextureViewDesc = {};
	depthTextureViewDesc.aspect = WGPUTextureAspect_DepthOnly;
	depthTextureViewDesc.baseArrayLayer = 0;
	depthTextureViewDesc.arrayLayerCount = 1;
	depthTextureViewDesc.baseMipLevel = 0;
	depthTextureViewDesc.mipLevelCount = 1;
	depthTextureViewDesc.dimension = WGPUTextureViewDimension_2D;
	depthTextureViewDesc.format = m_depth_texture_format;
	m_depth_texture_view = wgpuTextureCreateView(m_depth_texture, &depthTextureViewDesc);
	std::cout << "Depth texture view: " << m_depth_texture_view << std::endl;
}

void WindowSurface::resize(WGPUDevice& p_device) {
    release(true);
    init_swapchain(p_device);
    init_depth_buffer(p_device);
}

void WindowSurface::release(bool p_resizing_window) {
    if (m_depth_texture_view) {
        wgpuTextureViewRelease(m_depth_texture_view);
    }
    if (m_depth_texture) {
        wgpuTextureRelease(m_depth_texture);
    }
    if (m_swapchain) {
        wgpuSwapChainRelease(m_swapchain);
    }
    if (!p_resizing_window) {
        wgpuSurfaceRelease(m_surface);
    }
}

Camera::Camera(WGPUDevice& p_device) {
    WGPUBufferDescriptor buffer_description = {};
    buffer_description.label = "Camera buffer";
    buffer_description.mappedAtCreation = false;
    buffer_description.nextInChain = nullptr;
    buffer_description.size = sizeof(glm::mat4x4);
    buffer_description.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform;
    buffer = wgpuDeviceCreateBuffer(p_device, &buffer_description);
}

void Camera::update(int p_width, int p_height, WGPUQueue &p_queue, const Position& p_pos) {
    float aspect_ratio = (float) p_width / (float) p_height;
    if (m_last_aspect_ratio == 0.0 || m_last_aspect_ratio != aspect_ratio || m_last_turn != p_pos.get_moving_player()) {
        glm::mat4x4 projection = glm::perspective(glm::radians(fov), aspect_ratio, zNear, zFar);
        glm::mat4x4 view = glm::lookAt(glm::vec3(0.0f, 0.5f, p_pos.get_moving_player() == WHITE ? -0.5f : 0.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4x4 proj_view = projection * view;
        wgpuQueueWriteBuffer(p_queue, buffer, 0.0, &proj_view, sizeof(glm::mat4x4));
        m_last_aspect_ratio = aspect_ratio;
        m_last_turn = p_pos.get_moving_player();
    }
}

bool Renderer::init_gui() const {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO();
    ImGui_ImplWGPU_InitInfo info = {};
    info.Device = m_device;
    info.NumFramesInFlight = 3;
    info.RenderTargetFormat = m_surface.m_swapchain_format;
    info.DepthStencilFormat = m_surface.m_depth_texture_format;
    return ImGui_ImplGlfw_InitForOther(m_window, true) && ImGui_ImplWGPU_Init(&info);
}

void Renderer::terminate_gui() const {
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplWGPU_Shutdown();
}

Renderer::Renderer(int width, int height) {
    WGPUInstanceDescriptor desc = {};
    desc.nextInChain = nullptr;
    m_instance = wgpuCreateInstance(&desc);

    if (!glfwInit()) {
        std::cerr << "Could not initialize GLFW!" << std::endl;
        return;
    }

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	m_window = glfwCreateWindow(width, height, "Metropolia Chess", NULL, NULL);
	if (!m_window) {
		std::cerr << "Could not open window!" << std::endl;
		return;
	}
    m_surface = WindowSurface(m_instance,m_window);
    WGPURequestAdapterOptions adapterOpts = {};
    adapterOpts.nextInChain = nullptr;
    adapterOpts.compatibleSurface = m_surface.get_compatible_surface();
    adapterOpts.powerPreference = WGPUPowerPreference_HighPerformance;
    m_adapter = rd_utils::requestAdapter(m_instance, &adapterOpts);

    std::cout << "Got adapter: " << m_adapter << std::endl;
    m_surface.setup_surface_format(m_adapter);
    std::cout << "Requesting device..." << std::endl;

    WGPUDeviceDescriptor deviceDesc = {};
	deviceDesc.nextInChain = nullptr;
	deviceDesc.label = "My Device";
	deviceDesc.requiredFeaturesCount = 0;
	deviceDesc.requiredLimits = nullptr;
	deviceDesc.defaultQueue.nextInChain = nullptr;
	deviceDesc.defaultQueue.label = "The default queue";
	m_device = rd_utils::requestDevice(m_adapter, &deviceDesc);
	std::cout << "Got device: " << m_device << std::endl;

	m_graphics_queue = wgpuDeviceGetQueue(m_device);

    WGPUErrorCallback onDeviceError = [](WGPUErrorType type, char const* message, void* /* pUserData */) {
		std::cout << "Uncaptured device error: type " << type;
		if (message) std::cout << " (" << message << ")";
		std::cout << std::endl;
	};
	wgpuDeviceSetUncapturedErrorCallback(m_device, onDeviceError, nullptr /* pUserData */);
    WGPUSupportedLimits supportedLimits = {};
    wgpuDeviceGetLimits(m_device, &supportedLimits);
    WGPULimits deviceLimits = supportedLimits.limits;
    m_storage_alignment = deviceLimits.minStorageBufferOffsetAlignment;
    m_surface.resize(m_device);
    m_gui_enabled = init_gui();

    if (!m_gui_enabled) {
        std::cout << "Initialized without gui" << std::endl;
    }
    std::cout << "Loading resources ..." << std::endl;
    load_resources();
    m_initialized = true;
}

bool Renderer::is_active() const {
    return !m_window || !m_initialized || !glfwWindowShouldClose(m_window);
}

WGPUShaderModule create_shader_module(const char* p_filepath, WGPUDevice &p_device) {
    std::ifstream file(p_filepath, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
        std::cout << "failed to open " << p_filepath << std::endl;
		return nullptr;
	}

	//find what the size of the file is by looking up the location of the cursor
	//because the cursor is at the end, it gives the size directly in bytes
	size_t fileSize = (size_t)file.tellg();

    std::string shader_source(fileSize, ' ');

	//put file cursor at beggining
	file.seekg(0);

	//load the entire file into the buffer
	file.read(shader_source.data(), fileSize);

	//now that the file is loaded into the buffer, we can close it
	file.close();

    WGPUShaderModuleDescriptor shader_desc = {};
	shader_desc.nextInChain = nullptr;
	shader_desc.hintCount = 0;
	shader_desc.hints = nullptr;

	// Use the extension mechanism to load a WGSL shader source code
	WGPUShaderModuleWGSLDescriptor shader_code_desc = {};
	// Set the chained struct's header
	shader_code_desc.chain.next = nullptr;
	shader_code_desc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
	// Connect the chain
	shader_desc.nextInChain = &shader_code_desc.chain;

	// Setup the actual payload of the shader code descriptor
	shader_code_desc.code = shader_source.c_str();

	return wgpuDeviceCreateShaderModule(p_device, &shader_desc);
}



void Renderer::load_resources() {
    // loading a model
    std::pair<std::vector<Material>, std::vector<MeshNode>> loaded_info = model_loader::load_gltf("assets/models/chess_set.gltf", m_device);
    std::vector<Material> materials = loaded_info.first;
    m_mesh_nodes = loaded_info.second;
    m_render_pipelines.resize(materials.size());
    for (int i=0; i < m_render_pipelines.size(); ++i) {
        RenderPipeline& pipeline = m_render_pipelines[i];
        pipeline.id = materials[i].id;
        for (int j=0; j < m_mesh_nodes.size(); j++) {
            const MeshNode& node = m_mesh_nodes[j];
            if (pipeline.id == node.material_id) {
                pipeline.mesh_node_indexes.push_back(j);
            }
        }
    }
    uint32_t m_model_buffer_size = rd_utils::ceil_to_next_multiple(sizeof(glm::mat4x4), m_storage_alignment) * m_mesh_nodes.size();
    uint32_t m_model_buffer_stride = rd_utils::ceil_to_next_multiple(sizeof(glm::mat4x4), m_storage_alignment);
    {
        WGPUBufferDescriptor model_transformation_matrix_description = {};
        model_transformation_matrix_description.label = "Model transformation matrix buffer";
        model_transformation_matrix_description.mappedAtCreation = false;
        model_transformation_matrix_description.nextInChain = nullptr;
        model_transformation_matrix_description.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Storage;
        model_transformation_matrix_description.size = m_model_buffer_size;
        m_model_matrix_buffer = wgpuDeviceCreateBuffer(m_device, &model_transformation_matrix_description);
    }
    {
        size_t total_index_buffer_size = 0;
        size_t total_vertex_buffer_size = 0;
        for (int i = 0; i < m_mesh_nodes.size(); ++i) {
            const MeshNode& node = m_mesh_nodes[i];
            std::string id;
            std::string color;
            rd_utils::parse_string(node.id, &id, &color);
            if (id.length() == 0) {
                id = node.id;
            }
            if (id == "knight") {
                id += "_" + color;
            }
            if (!m_vertex_offsets.contains(id)) {
                m_vertex_offsets[id] = total_vertex_buffer_size;
                total_vertex_buffer_size += node.verticies.size() * sizeof(Vertex);
            }
            if (!m_index_offsets.contains(id)) {
                m_index_offsets[id] = total_index_buffer_size;
                total_index_buffer_size += node.indices.size() * sizeof(uint32_t);
            }
        }
        WGPUBufferDescriptor vertex_buffer_description = {};
        vertex_buffer_description.label = "Global vertex buffer";
        vertex_buffer_description.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex;
        vertex_buffer_description.mappedAtCreation = false;
        vertex_buffer_description.size = total_vertex_buffer_size;
        
        m_global_vertex_buffer = wgpuDeviceCreateBuffer(m_device, &vertex_buffer_description);

        WGPUBufferDescriptor index_buffer_description = {};
        index_buffer_description.label = "Global index buffer";
        index_buffer_description.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Index;
        index_buffer_description.mappedAtCreation = false;
        index_buffer_description.size = total_index_buffer_size;

        m_global_index_buffer = wgpuDeviceCreateBuffer(m_device, &index_buffer_description);

        for (int i = 0; i < m_mesh_nodes.size(); ++i) {
            const MeshNode& node = m_mesh_nodes[i];
            std::string id;
            std::string color;
            rd_utils::parse_string(node.id, &id, &color);
            if (id.length() == 0) {
                id = node.id;
            }
            if (id == "knight") {
                id += "_" + color;
            }
            wgpuQueueWriteBuffer(m_graphics_queue, m_global_vertex_buffer, m_vertex_offsets.at(id), node.verticies.data(), sizeof(Vertex) * node.verticies.size());
            wgpuQueueWriteBuffer(m_graphics_queue, m_global_index_buffer, m_index_offsets.at(id), node.indices.data(), sizeof(uint32_t) * node.indices.size());
            if (!m_model_matrix_offsets.contains(node.id)) {
                m_model_matrix_offsets[node.id] = m_model_buffer_stride * i;
            }
        }
    }

    // create texture sampler
    WGPUSamplerDescriptor sampler_desc = {};
	sampler_desc.addressModeU = WGPUAddressMode_Repeat;
	sampler_desc.addressModeV = WGPUAddressMode_Repeat;
	sampler_desc.addressModeW = WGPUAddressMode_Repeat;
	sampler_desc.magFilter = WGPUFilterMode_Linear;
	sampler_desc.minFilter = WGPUFilterMode_Linear;
	sampler_desc.mipmapFilter = WGPUMipmapFilterMode_Linear;
	sampler_desc.lodMinClamp = 0.0f;
	sampler_desc.lodMaxClamp = 8.0f;
	sampler_desc.compare = WGPUCompareFunction_Undefined;
	sampler_desc.maxAnisotropy = 1;
	m_global_sampler = wgpuDeviceCreateSampler(m_device,&sampler_desc);

    // creating a pipeline for the shader
    WGPUShaderModule shader_module = create_shader_module("assets/shaders/chess_shader.wgsl", m_device);
    if (shader_module) {
        // Create bind groups
        WGPUBindGroupLayoutEntry camera_layout_entry = {};
        rd_utils::set_bind_group_entry_default(camera_layout_entry);
        camera_layout_entry.binding = 0;
        camera_layout_entry.visibility = WGPUShaderStage_Vertex;
        camera_layout_entry.buffer.type = WGPUBufferBindingType_Uniform;
        camera_layout_entry.buffer.minBindingSize = sizeof(glm::mat4x4);

        WGPUBindGroupLayoutDescriptor camera_bind_group_layout_description = {};
        camera_bind_group_layout_description.entries = &camera_layout_entry;
        camera_bind_group_layout_description.entryCount = 1;
        WGPUBindGroupLayout camera_bind_group_layout = wgpuDeviceCreateBindGroupLayout(m_device, &camera_bind_group_layout_description);

        WGPUBindGroupLayoutEntry model_layout_entry = {};
        rd_utils::set_bind_group_entry_default(model_layout_entry);
        model_layout_entry.binding = 0;
        model_layout_entry.visibility = WGPUShaderStage_Vertex;
        model_layout_entry.buffer.type = WGPUBufferBindingType_ReadOnlyStorage;
        model_layout_entry.buffer.hasDynamicOffset = true;
        model_layout_entry.buffer.minBindingSize = m_model_buffer_stride;

        WGPUBindGroupLayoutDescriptor model_bind_group_layout_description = {};
        model_bind_group_layout_description.entries = &model_layout_entry;
        model_bind_group_layout_description.entryCount = 1;
        WGPUBindGroupLayout model_bind_group_layout = wgpuDeviceCreateBindGroupLayout(m_device, &model_bind_group_layout_description);

        WGPUBindGroupLayoutEntry sampler_layout_entry = {};
        rd_utils::set_bind_group_entry_default(sampler_layout_entry);
        sampler_layout_entry.binding = 0;
        sampler_layout_entry.visibility = WGPUShaderStage_Fragment;
        sampler_layout_entry.sampler.type = WGPUSamplerBindingType_Filtering;
        
        WGPUBindGroupLayoutEntry texture_view_layout_entry = {};
        rd_utils::set_bind_group_entry_default(texture_view_layout_entry);
        texture_view_layout_entry.binding = 1;
        texture_view_layout_entry.visibility = WGPUShaderStage_Fragment;
        texture_view_layout_entry.texture.sampleType = WGPUTextureSampleType_Float;
        texture_view_layout_entry.texture.viewDimension = WGPUTextureViewDimension_2D;

        WGPUBindGroupLayoutEntry material_layout_entries[2] = {sampler_layout_entry, texture_view_layout_entry};

        WGPUBindGroupLayoutDescriptor material_bind_group_layout_description = {};
        material_bind_group_layout_description.entries = material_layout_entries;
        material_bind_group_layout_description.entryCount = 2;
        WGPUBindGroupLayout material_bind_group_layout = wgpuDeviceCreateBindGroupLayout(m_device, &material_bind_group_layout_description);

        std::array<WGPUBindGroupLayout, 3> bind_group_layouts = {camera_bind_group_layout, model_bind_group_layout, material_bind_group_layout};

        // Create bind group itself
        m_camera = Camera(m_device);
        WGPUBindGroupEntry camera_bind_group_entry = {};
        camera_bind_group_entry.nextInChain = nullptr;
        camera_bind_group_entry.size = sizeof(glm::mat4x4);
        camera_bind_group_entry.binding = 0;
        camera_bind_group_entry.offset = 0;
        camera_bind_group_entry.buffer = m_camera.buffer;
        camera_bind_group_entry.textureView = nullptr;
        camera_bind_group_entry.sampler = nullptr;

        WGPUBindGroupEntry model_bind_group_entry = {};
        model_bind_group_entry.nextInChain = nullptr;
        model_bind_group_entry.size = m_model_buffer_stride;
        model_bind_group_entry.binding = 0;
        model_bind_group_entry.offset = 0;
        model_bind_group_entry.buffer = m_model_matrix_buffer;
        model_bind_group_entry.textureView = nullptr;
        model_bind_group_entry.sampler = nullptr;

        WGPUBindGroupDescriptor camera_bind_group_description = {};
        camera_bind_group_description.label = "camera chess bind group";
        camera_bind_group_description.entryCount = 1; 
        camera_bind_group_description.entries = &camera_bind_group_entry;
        camera_bind_group_description.nextInChain = nullptr;
        camera_bind_group_description.layout = camera_bind_group_layout;

        WGPUBindGroupDescriptor model_bind_group_description = {};
        model_bind_group_description.label = "model chess bind group";
        model_bind_group_description.entryCount = 1; 
        model_bind_group_description.entries = &model_bind_group_entry;
        model_bind_group_description.nextInChain = nullptr;
        model_bind_group_description.layout = model_bind_group_layout;


        WGPUBindGroupEntry sampler_bind_group_entry = {};
        sampler_bind_group_entry.nextInChain = nullptr;
        sampler_bind_group_entry.size = 0;
        sampler_bind_group_entry.binding = 0;
        sampler_bind_group_entry.offset = 0;
        sampler_bind_group_entry.buffer = nullptr;
        sampler_bind_group_entry.textureView = nullptr;
        sampler_bind_group_entry.sampler = m_global_sampler;

        for (int i = 0; i < materials.size(); ++i) {

            WGPUBindGroupEntry texture_view_bind_group_entry = {};
            texture_view_bind_group_entry.nextInChain = nullptr;
            texture_view_bind_group_entry.size = 0;
            texture_view_bind_group_entry.binding = 1;
            texture_view_bind_group_entry.offset = 0;
            texture_view_bind_group_entry.buffer = nullptr;
            texture_view_bind_group_entry.textureView = materials[i].base_color_view;
            texture_view_bind_group_entry.sampler = nullptr;

            WGPUBindGroupEntry material_bind_group_entries[2] = {sampler_bind_group_entry, texture_view_bind_group_entry};
            WGPUBindGroupDescriptor material_bind_group_description = {};
            material_bind_group_description.label = "material chess bind group";
            material_bind_group_description.entryCount = 2; 
            material_bind_group_description.entries = material_bind_group_entries;
            material_bind_group_description.nextInChain = nullptr;
            material_bind_group_description.layout = material_bind_group_layout;

            m_render_pipelines[i].material_group = wgpuDeviceCreateBindGroup(m_device, &material_bind_group_description);
        }
        m_chess_camera_bind_group = wgpuDeviceCreateBindGroup(m_device, &camera_bind_group_description);
        m_chess_model_bind_group = wgpuDeviceCreateBindGroup(m_device, &model_bind_group_description);

        for (int i = 0; i < m_render_pipelines.size(); ++i) {
            WGPUVertexAttribute attributes[3];
            attributes[0].shaderLocation = 0;
            attributes[0].format = WGPUVertexFormat_Float32x3;
            attributes[0].offset = offsetof(Vertex, position);
            attributes[1].shaderLocation = 1;
            attributes[1].format = WGPUVertexFormat_Float32x3;
            attributes[1].offset = offsetof(Vertex, normal);
            attributes[2].shaderLocation = 2;
            attributes[2].format = WGPUVertexFormat_Float32x2;
            attributes[2].offset = offsetof(Vertex, uv);
            WGPUVertexBufferLayout vertex_buffer_layout = {};
            vertex_buffer_layout.stepMode = WGPUVertexStepMode_Vertex;
            vertex_buffer_layout.arrayStride = sizeof(Vertex);
            vertex_buffer_layout.attributeCount = 3;
            vertex_buffer_layout.attributes = attributes;

            WGPURenderPipelineDescriptor pipeline_descriptor = {};
            pipeline_descriptor.vertex.bufferCount = 1;
            pipeline_descriptor.vertex.buffers = &vertex_buffer_layout;
            pipeline_descriptor.vertex.module = shader_module;
            pipeline_descriptor.vertex.entryPoint = "vs_main";
            pipeline_descriptor.vertex.constantCount = 0;
            pipeline_descriptor.vertex.constants = nullptr;
            pipeline_descriptor.primitive.topology = WGPUPrimitiveTopology_TriangleList;  
            pipeline_descriptor.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
            pipeline_descriptor.primitive.frontFace = WGPUFrontFace_CCW;
            pipeline_descriptor.primitive.cullMode = WGPUCullMode_Back;

            WGPUFragmentState fragment_state = {};
            fragment_state.nextInChain = nullptr;
            fragment_state.module = shader_module;
            fragment_state.entryPoint = "fs_main";
            fragment_state.constantCount = 0;
            fragment_state.constants = nullptr;
            pipeline_descriptor.fragment = &fragment_state;

            // Configure blend state
            WGPUBlendState blend_state;
            // Usual alpha blending for the color:
            blend_state.color.srcFactor = WGPUBlendFactor_SrcAlpha;
            blend_state.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
            blend_state.color.operation = WGPUBlendOperation_Add;
            // We leave the target alpha untouched:
            blend_state.alpha.srcFactor = WGPUBlendFactor_Zero;
            blend_state.alpha.dstFactor = WGPUBlendFactor_One;
            blend_state.alpha.operation = WGPUBlendOperation_Add;

            WGPUColorTargetState colorTarget = {};
            colorTarget.nextInChain = nullptr;
            colorTarget.format = m_surface.m_swapchain_format;
            colorTarget.blend = &blend_state;
            colorTarget.writeMask = WGPUColorWriteMask_All; // We could write to only some of the color channels.

            // We have only one target because our render pass has only one output color
            // attachment.
            fragment_state.targetCount = 1;
            fragment_state.targets = &colorTarget;
            WGPUDepthStencilState depth_stencil_state = {};
            depth_stencil_state.format = m_surface.m_depth_texture_format;
            depth_stencil_state.depthWriteEnabled = true;
            depth_stencil_state.depthCompare = WGPUCompareFunction_LessEqual;
            depth_stencil_state.stencilReadMask = 0xFFFFFFFF;
            depth_stencil_state.stencilWriteMask = 0xFFFFFFFF;
            depth_stencil_state.depthBias = 0;
            depth_stencil_state.depthBiasSlopeScale = 0;
            depth_stencil_state.depthBiasClamp = 0;
            rd_utils::set_stencil_face_default(depth_stencil_state.stencilFront);
            rd_utils::set_stencil_face_default(depth_stencil_state.stencilBack);
            pipeline_descriptor.depthStencil = &depth_stencil_state;

            // Multi-sampling
            // Samples per pixel
            pipeline_descriptor.multisample.count = 1;
            // Default value for the mask, meaning "all bits on"
            pipeline_descriptor.multisample.mask = ~0u;
            // Default value as well (irrelevant for count = 1 anyways)
            pipeline_descriptor.multisample.alphaToCoverageEnabled = false;

            WGPUPipelineLayoutDescriptor pipeline_layout_description = {};
            pipeline_layout_description.label = "Chess pipeline layout";
            pipeline_layout_description.nextInChain = nullptr;
            pipeline_layout_description.bindGroupLayouts = bind_group_layouts.data();
            pipeline_layout_description.bindGroupLayoutCount = bind_group_layouts.size();
            // Pipeline layout
            pipeline_descriptor.layout = wgpuDeviceCreatePipelineLayout(m_device, &pipeline_layout_description);

            m_render_pipelines[i].pipeline = wgpuDeviceCreateRenderPipeline(m_device, &pipeline_descriptor);
        }
    }
}

FrameInfo Renderer::prepare_frame(const Position& p_pos) {
    if (!m_initialized) {
        return {};
    }
    m_camera.update(m_surface.current_resolution[0], m_surface.current_resolution[1], m_graphics_queue, p_pos);
    glfwPollEvents();

    WGPUTextureView nextTexture = m_surface.get_current_swapchain_texture_view();
    // Getting the texture may fail, in particular if the window has been resized
    // and thus the target surface changed.
    if (!nextTexture) {
        std::cerr << "Cannot acquire next swap chain texture" << std::endl;
        return {};
    }

    WGPUCommandEncoderDescriptor commandEncoderDesc = {};
    commandEncoderDesc.nextInChain = nullptr;
    commandEncoderDesc.label = "Command Encoder";
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(m_device, &commandEncoderDesc);
		
    WGPURenderPassColorAttachment renderPassColorAttachment = {};
    renderPassColorAttachment.view = nextTexture;
    renderPassColorAttachment.resolveTarget = nullptr;
    renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
    renderPassColorAttachment.storeOp = WGPUStoreOp_Store;
    renderPassColorAttachment.clearValue = WGPUColor{ 0.0, 0.0, 0.0, 1.0 };

    WGPURenderPassDepthStencilAttachment depthStencilAttachment = {};
	depthStencilAttachment.view = m_surface.get_current_depth_texture_view();
	depthStencilAttachment.depthClearValue = 1.0f;
	depthStencilAttachment.depthLoadOp = WGPULoadOp_Clear;
	depthStencilAttachment.depthStoreOp = WGPUStoreOp_Store;
	depthStencilAttachment.depthReadOnly = false;
    depthStencilAttachment.stencilClearValue = 0;
    depthStencilAttachment.stencilLoadOp = WGPULoadOp_Clear;
    depthStencilAttachment.stencilStoreOp = WGPUStoreOp_Store;
    depthStencilAttachment.stencilReadOnly = true;

    WGPURenderPassDescriptor renderPassDesc = {};
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &renderPassColorAttachment;
    renderPassDesc.depthStencilAttachment = &depthStencilAttachment;

    // We do not use timers for now neither
    renderPassDesc.timestampWriteCount = 0;
    renderPassDesc.timestampWrites = nullptr;

    renderPassDesc.nextInChain = nullptr;

    // Create a render pass. We end it immediately because we use its built-in
    // mechanism for clearing the screen when it begins (see descriptor).
    WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
    for (RenderPipeline& pipeline : m_render_pipelines) {
        if (m_chess_camera_bind_group && m_chess_model_bind_group && m_global_vertex_buffer && m_global_index_buffer) {
            wgpuRenderPassEncoderSetPipeline(renderPass, pipeline.pipeline);
            wgpuRenderPassEncoderSetBindGroup(renderPass, 0, m_chess_camera_bind_group, 0, nullptr);
            for (int mesh_index: pipeline.mesh_node_indexes) {
                MeshNode& node = m_mesh_nodes[mesh_index];
                std::string id;
                std::string color;
                rd_utils::parse_string(node.id, &id, &color);
                if (id.length() == 0) {
                    id = node.id;
                }
                if (id == "knight") {
                    id += "_" + color;
                }
                if (node.get_is_dirty()) {
                    uint32_t offset = m_model_matrix_offsets.at(node.id);
                    wgpuQueueWriteBuffer(m_graphics_queue, m_model_matrix_buffer, offset, &node.transform_matrix,sizeof(glm::mat4x4));
                    node.reset_dirt();
                }
                uint32_t storage_offset = m_model_matrix_offsets.at(node.id);
                wgpuRenderPassEncoderSetBindGroup(renderPass, 1, m_chess_model_bind_group, 1, &storage_offset);
                wgpuRenderPassEncoderSetBindGroup(renderPass, 2, pipeline.material_group, 0, nullptr);
                size_t vertex_offset = m_vertex_offsets.at(id);
                wgpuRenderPassEncoderSetVertexBuffer(renderPass, 0, m_global_vertex_buffer, vertex_offset, node.verticies.size() * sizeof(Vertex));
                if (node.indices.size() > 0) {
                    size_t index_offset = m_index_offsets.at(id);
                    wgpuRenderPassEncoderSetIndexBuffer(renderPass, m_global_index_buffer, WGPUIndexFormat_Uint32, index_offset, node.indices.size() * sizeof(uint32_t));
                    wgpuRenderPassEncoderDrawIndexed(renderPass, node.indices.size(), 1, 0, 0, 0);
                } else {
                    wgpuRenderPassEncoderDraw(renderPass, node.verticies.size(), 1, 0, 0);
                }

            }
        }
    }
    if (m_gui_enabled) {
        ImGui_ImplWGPU_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }
    FrameInfo frame_info = {};
    frame_info.encoder = encoder;
    frame_info.next_texture = nextTexture;
    frame_info.render_pass = renderPass;
    return frame_info;
}

void Renderer::render_board(FrameInfo& p_info, const Move& p_move) {
    if (!(p_move.get_start_pos()[0] == -1 || p_move.get_start_pos()[1] == -1) && !(p_move.get_end_pos()[0] == -1 || p_move.get_end_pos()[1] == -1)) {
        float distance = 0.05788809061050415;
        float origin_x = 0.20260828733444214;
        float origin_z = 0.2018999457359314;
        float tolerance = 0.001;
        // if square is not empty, delete mesh node.
        bool chess_piece_deleted = false;
        for (int i = 0; i < m_mesh_nodes.size(); ++i) {
            const MeshNode& node = m_mesh_nodes[i];
            if (node.id == "board") {
                continue;
            }
            glm::vec3 pos = node.get_position();
            int z_dir = (p_move.get_end_pos()[0] - p_move.get_start_pos()[0]);
            z_dir = z_dir/abs(z_dir);
            int x_dir = (p_move.get_end_pos()[1] - p_move.get_start_pos()[1]); 
            x_dir = x_dir/abs(x_dir);
            float row = origin_z - distance * p_move.get_end_pos()[0];
            float col = origin_x -  distance * p_move.get_end_pos()[1];
            float z_distance = std::abs(pos.z - row);
            float x_distance = std::abs(pos.x - col);
            if (z_distance < tolerance && x_distance < tolerance) {
                bool found = false;
                for (RenderPipeline& pipeline : m_render_pipelines) {
                    if (found) {
                        break;
                    }
                    for (int j = 0; j < pipeline.mesh_node_indexes.size(); j++) {
                        if (pipeline.mesh_node_indexes[j] == i) {
                            pipeline.mesh_node_indexes.erase(pipeline.mesh_node_indexes.begin() + j);
                            chess_piece_deleted = true;
                            m_mesh_nodes.erase(m_mesh_nodes.begin() + i);
                            found = true;
                            break;
                        }
                    }
                }
                if (found) {
                    break;
                }
            }
        }
        if (chess_piece_deleted) {
            for (RenderPipeline& pipeline : m_render_pipelines) {
                pipeline.mesh_node_indexes.clear();
            }
            for (int i = 0; i < m_mesh_nodes.size(); ++i) {
                for (RenderPipeline& pipeline : m_render_pipelines) {
                    if (m_mesh_nodes[i].material_id == pipeline.id) {
                        pipeline.mesh_node_indexes.push_back(i);
                    }
                }
            }
        }
        // update moving chess piece's position
        for (MeshNode& node : m_mesh_nodes) {
            if (node.id == "board") {
                continue;
            }
            glm::vec3 pos = node.get_position();
            float row = origin_z - distance * p_move.get_start_pos()[0];
            float col = origin_x - distance * p_move.get_start_pos()[1];
            float z_distance = std::abs(pos.z - row);
            float x_distance = std::abs(pos.x - col);
            if (z_distance < tolerance && x_distance < tolerance) {
                pos.x = pos.x - distance *  (p_move.get_end_pos()[1] - p_move.get_start_pos()[1]);
                pos.z = pos.z - distance * (p_move.get_end_pos()[0] - p_move.get_start_pos()[0]);
                node.set_position(pos);
            }
        }
    }
    if (!p_info.render_pass || !p_info.encoder || !p_info.next_texture) {
        std::cout << "FrameInfo is incomplete" << std::endl;
        return;
    } 
    if (m_gui_enabled) {
        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), p_info.render_pass);
    }
    wgpuRenderPassEncoderEnd(p_info.render_pass);
    wgpuRenderPassEncoderRelease(p_info.render_pass);

    wgpuTextureViewRelease(p_info.next_texture);

    WGPUCommandBufferDescriptor cmdBufferDescriptor = {};
    cmdBufferDescriptor.nextInChain = nullptr;
    cmdBufferDescriptor.label = "Command buffer";
    WGPUCommandBuffer command = wgpuCommandEncoderFinish(p_info.encoder, &cmdBufferDescriptor);
    wgpuCommandEncoderRelease(p_info.encoder);
    wgpuQueueSubmit(m_graphics_queue, 1, &command);
    wgpuCommandBufferRelease(command);

    // We can tell the swap chain to present the next texture.
    m_surface.present();
}

void Renderer::destroy() {
    terminate_gui();
    wgpuBufferRelease(m_global_vertex_buffer);
    wgpuBufferRelease(m_global_index_buffer);
    m_surface.release(false);
	wgpuDeviceRelease(m_device);
	wgpuAdapterRelease(m_adapter);
	wgpuInstanceRelease(m_instance);
    glfwDestroyWindow(m_window);
	glfwTerminate();
}