#pragma once

#include <webgpu/webgpu.h>
#include <iostream>

namespace rd_utils {
    WGPUAdapter requestAdapter(WGPUInstance instance, WGPURequestAdapterOptions const * options);

    WGPUDevice requestDevice(WGPUAdapter adapter, WGPUDeviceDescriptor const * descriptor);

    void set_stencil_face_default(WGPUStencilFaceState &stencilFaceState);

    void set_bind_group_entry_default(WGPUBindGroupLayoutEntry &bindingLayout);

    uint32_t ceil_to_next_multiple(uint32_t value, uint32_t step);

    void parse_string(const std::string& str, std::string* type = nullptr, std::string* color = nullptr);
}