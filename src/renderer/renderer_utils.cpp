#include "renderer_utils.h"
#include <cassert>
#include <vector>

namespace rd_utils {
    WGPUAdapter requestAdapter(WGPUInstance instance, WGPURequestAdapterOptions const * options) {
    // A simple structure holding the local information shared with the
    // onAdapterRequestEnded callback.
    struct UserData {
        WGPUAdapter adapter = nullptr;
        bool requestEnded = false;
    };
    UserData userData;

    // Callback called by wgpuInstanceRequestAdapter when the request returns
    // This is a C++ lambda function, but could be any function defined in the
    // global scope. It must be non-capturing (the brackets [] are empty) so
    // that it behaves like a regular C function pointer, which is what
    // wgpuInstanceRequestAdapter expects (WebGPU being a C API). The workaround
    // is to convey what we want to capture through the pUserData pointer,
    // provided as the last argument of wgpuInstanceRequestAdapter and received
    // by the callback as its last argument.
    auto onAdapterRequestEnded = [](WGPURequestAdapterStatus status, WGPUAdapter adapter, char const * message, void * pUserData) {
        UserData& userData = *reinterpret_cast<UserData*>(pUserData);
        if (status == WGPURequestAdapterStatus_Success) {
            userData.adapter = adapter;
        } else {
            std::cout << "Could not get WebGPU adapter: " << message << std::endl;
        }
        userData.requestEnded = true;
    };

    // Call to the WebGPU request adapter procedure
    wgpuInstanceRequestAdapter(
        instance /* equivalent of navigator.gpu */,
        options,
        onAdapterRequestEnded,
        (void*)&userData
    );

    // In theory we should wait until onAdapterReady has been called, which
    // could take some time (what the 'await' keyword does in the JavaScript
    // code). In practice, we know that when the wgpuInstanceRequestAdapter()
    // function returns its callback has been called.
    assert(userData.requestEnded);

    return userData.adapter;
}

WGPUDevice requestDevice(WGPUAdapter adapter, WGPUDeviceDescriptor const * descriptor) {
    struct UserData {
        WGPUDevice device = nullptr;
        bool requestEnded = false;
    };
    UserData userData;

    auto onDeviceRequestEnded = [](WGPURequestDeviceStatus status, WGPUDevice device, char const * message, void * pUserData) {
        UserData& userData = *reinterpret_cast<UserData*>(pUserData);
        if (status == WGPURequestDeviceStatus_Success) {
            userData.device = device;
        } else {
            std::cout << "Could not get WebGPU device: " << message << std::endl;
        }
        userData.requestEnded = true;
    };

    wgpuAdapterRequestDevice(
        adapter,
        descriptor,
        onDeviceRequestEnded,
        (void*)&userData
    );

    assert(userData.requestEnded);

    return userData.device;
}

    void set_stencil_face_default(WGPUStencilFaceState &stencilFaceState) {
        stencilFaceState.compare = WGPUCompareFunction_Always;
        stencilFaceState.failOp = WGPUStencilOperation_Keep;
        stencilFaceState.depthFailOp = WGPUStencilOperation_Keep;
        stencilFaceState.passOp = WGPUStencilOperation_Keep;
    }

    void set_bind_group_entry_default(WGPUBindGroupLayoutEntry &bindingLayout) {
        bindingLayout.buffer.nextInChain = nullptr;
        bindingLayout.buffer.type = WGPUBufferBindingType_Undefined;
        bindingLayout.buffer.hasDynamicOffset = false;

        bindingLayout.sampler.nextInChain = nullptr;
        bindingLayout.sampler.type = WGPUSamplerBindingType_Undefined;

        bindingLayout.storageTexture.nextInChain = nullptr;
        bindingLayout.storageTexture.access = WGPUStorageTextureAccess_Undefined;
        bindingLayout.storageTexture.format = WGPUTextureFormat_Undefined;
        bindingLayout.storageTexture.viewDimension = WGPUTextureViewDimension_Undefined;

        bindingLayout.texture.nextInChain = nullptr;
        bindingLayout.texture.multisampled = false;
        bindingLayout.texture.sampleType = WGPUTextureSampleType_Undefined;
        bindingLayout.texture.viewDimension = WGPUTextureViewDimension_Undefined;
    }

    uint32_t ceil_to_next_multiple(uint32_t value, uint32_t step) {
        uint32_t divide_and_ceil = value / step + (value % step == 0 ? 0 : 1);
        return step * divide_and_ceil;
    }

    void parse_string(const std::string& str, std::string* type, std::string* color) {
        std::vector<std::string> parts;
        std::string delimiter = "_";
        std::string remaining_str = str;
        size_t pos = 0;
        std::string token;
        while ((pos = remaining_str.find(delimiter)) != std::string::npos) {
            token = remaining_str.substr(0, pos);
            parts.push_back(token);
            remaining_str = remaining_str.substr(pos + delimiter.length());
        }
        parts.push_back(remaining_str);

        if (parts.size() >= 3) {
            if (type != nullptr) {
                *type = parts[1];
            }
            if (color != nullptr) {
                *color = parts[2];
            }
        }
    }
}