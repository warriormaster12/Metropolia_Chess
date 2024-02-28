#include "model_loader.h"
#include "ext/matrix_transform.hpp"
#include "tiny_gltf.h"
#include "renderer_utils.h"
#include <vector>
#include "gtc/type_ptr.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "gtx/quaternion.hpp"
#include "chess/chess.h"

void MeshNode::set_position(const glm::vec3 position) {
    if (m_position != position) {
        m_position = position;
        glm::vec3 scale = glm::vec3(1.0f);
        glm::quat rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        transform_matrix = glm::translate(glm::mat4(1.0), m_position) *  glm::toMat4(rot) * glm::scale(glm::mat4(1.0f), scale);
        m_is_dirty = true;
    }
}

namespace model_loader {

void write_mip_maps(WGPUDevice p_device, WGPUTexture p_texture, WGPUExtent3D p_textureSize, uint32_t p_mipLevelCount, const unsigned char* p_pixelData) {


    WGPUQueue queue = wgpuDeviceGetQueue(p_device);
    // Arguments telling which part of the texture we upload to
	WGPUImageCopyTexture destination;
	destination.texture = p_texture;
	destination.origin = { 0, 0, 0 };
	destination.aspect = WGPUTextureAspect_All;

	// Arguments telling how the C++ side pixel memory is laid out
	WGPUTextureDataLayout source = {};
    source.offset = 0;

	// Create image data
	WGPUExtent3D mipLevelSize = p_textureSize;
	std::vector<unsigned char> previousLevelPixels;
	WGPUExtent3D previousMipLevelSize;
	for (uint32_t level = 0; level < p_mipLevelCount; ++level) {
		// Pixel data for the current level
		std::vector<unsigned char> pixels(4 * mipLevelSize.width * mipLevelSize.height);
		if (level == 0) {
			// We cannot really avoid this copy since we need this
			// in previousLevelPixels at the next iteration
			memcpy(pixels.data(), p_pixelData, pixels.size());
		}
		else {
			// Create mip level data
			for (uint32_t i = 0; i < mipLevelSize.width; ++i) {
				for (uint32_t j = 0; j < mipLevelSize.height; ++j) {
					unsigned char* p = &pixels[4 * (j * mipLevelSize.width + i)];
					// Get the corresponding 4 pixels from the previous level
					unsigned char* p00 = &previousLevelPixels[4 * ((2 * j + 0) * previousMipLevelSize.width + (2 * i + 0))];
					unsigned char* p01 = &previousLevelPixels[4 * ((2 * j + 0) * previousMipLevelSize.width + (2 * i + 1))];
					unsigned char* p10 = &previousLevelPixels[4 * ((2 * j + 1) * previousMipLevelSize.width + (2 * i + 0))];
					unsigned char* p11 = &previousLevelPixels[4 * ((2 * j + 1) * previousMipLevelSize.width + (2 * i + 1))];
					// Average
					p[0] = (p00[0] + p01[0] + p10[0] + p11[0]) / 4;
					p[1] = (p00[1] + p01[1] + p10[1] + p11[1]) / 4;
					p[2] = (p00[2] + p01[2] + p10[2] + p11[2]) / 4;
					p[3] = (p00[3] + p01[3] + p10[3] + p11[3]) / 4;
				}
			}
		}

		// Upload data to the GPU texture
		destination.mipLevel = level;
		source.bytesPerRow = 4 * mipLevelSize.width;
		source.rowsPerImage = mipLevelSize.height;
        wgpuQueueWriteTexture(queue, &destination, pixels.data(), pixels.size(), &source, &mipLevelSize);

		previousLevelPixels = std::move(pixels);
		previousMipLevelSize = mipLevelSize;
		mipLevelSize.width /= 2;
		mipLevelSize.height /= 2;
	}
    wgpuQueueRelease(queue);
}

uint32_t bit_width(uint32_t m) {
	if (m == 0) return 0;
	else { uint32_t w = 0; while (m >>= 1) ++w; return w; }
}



std::pair<std::vector<Material>,std::vector<MeshNode>> load_gltf(const std::string& file, WGPUDevice& p_device) {
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;
    tinygltf::Model model;
    std::vector<MeshNode> out_nodes;
    std::vector<Material> out_materials;
    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, file);

    if (!warn.empty()) {
        std::cout <<"Warn: " << warn.c_str() <<std::endl;
    }

    if (!err.empty()) {
        std::cout <<"Error: " << err.c_str() <<std::endl;
        return std::make_pair(out_materials, out_nodes);;
    }

    if (!ret) {
        std::cout << "Failed to parse gltf" <<std::endl;
        return std::make_pair(out_materials, out_nodes);
    }

    for (tinygltf::Node node : model.nodes) {
        if (node.mesh != -1) {
            MeshNode mesh_node;
            mesh_node.id = node.name;
            std::string piece_name, color;
            rd_utils::parse_string(node.name, &piece_name,&color);
            if (piece_name == "rook") {
                mesh_node.chess_piece = color == "black" ? bR : wR;
            } else if (piece_name == "pawn") {
                mesh_node.chess_piece = color == "black" ? bP : wP;
            } else if (piece_name == "bishop") {
                mesh_node.chess_piece = color == "black" ? bB : wB;
            } else if (piece_name == "knight") {
                mesh_node.chess_piece = color == "black" ? bN : wN;
            } else if (piece_name == "queen") {
                mesh_node.chess_piece = color == "black" ? bQ : wQ;
            } else if (piece_name == "king") {
                mesh_node.chess_piece = color == "black" ? bK : wK;
            }
            glm::vec3 pos = glm::vec3(0.0f);
            glm::vec3 scale = glm::vec3(1.0f);
            glm::quat rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
            if (node.translation.size() > 0) {
                pos = glm::make_vec3(node.translation.data());
                mesh_node.set_position(pos);
            }
            if (node.scale.size() > 0) {
                scale = glm::make_vec3(node.scale.data());
            }
            if (node.rotation.size() > 0) {
                rot = glm::make_quat(node.rotation.data());
            }
            mesh_node.transform_matrix = glm::translate(glm::mat4(1.0), pos) *  glm::toMat4(rot) * glm::scale(glm::mat4(1.0f), scale);
            for(tinygltf::Primitive primitive : model.meshes[node.mesh].primitives) {
                size_t starting_vertex_index = mesh_node.verticies.size();
                if(primitive.attributes.find("POSITION") != primitive.attributes.end())  {
                    const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("POSITION")->second];
                    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];

                    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
                    const float* positions = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
                    mesh_node.verticies.resize(mesh_node.verticies.size() + accessor.count);
                    for (size_t i = starting_vertex_index; i < mesh_node.verticies.size(); ++i) {
                        mesh_node.verticies[i].position = glm::vec3(positions[i * 3 + 0], positions[i * 3 + 1], positions[i * 3 + 2]);
                    }
                }
                if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("NORMAL")->second];
                    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];

                    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
                    const float* normals = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
                    for (size_t i = starting_vertex_index; i < mesh_node.verticies.size(); ++i) {
                        mesh_node.verticies[i].normal = glm::vec3(normals[i * 3 + 0], normals[i * 3 + 1], normals[i * 3 + 2]);
                    }
                }
                if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
                    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];

                    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
                    const float* tex_coords = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
                    for (size_t i = starting_vertex_index; i < mesh_node.verticies.size(); ++i) {
                        mesh_node.verticies[i].uv = glm::vec2(tex_coords[i * 2 + 0], tex_coords[i * 2 + 1]);
                    }
                }
                if(primitive.indices > 0) {
                    size_t starting_indicies_index = mesh_node.indices.size();
                    const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
                    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];

                    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
                    const uint32_t* indices = reinterpret_cast<const uint32_t*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
                    mesh_node.indices.resize(mesh_node.indices.size() + accessor.count);

                    switch (accessor.componentType) {
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                            uint32_t* buf = new uint32_t[accessor.count];
                            memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint32_t));
                            for (size_t index = 0; index < accessor.count; ++index) {
                                mesh_node.indices[index + starting_indicies_index] = buf[index];
                            }
                            break;
                        }
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                            uint16_t* buf = new uint16_t[accessor.count];
                            memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint16_t));
                            for (size_t index = 0; index < accessor.count; ++index) {
                                mesh_node.indices[index + starting_indicies_index] = buf[index];
                            }
                            break;
                        }
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                            uint8_t* buf = new uint8_t[accessor.count];
                            memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint8_t));
                            for (size_t index = 0; index < accessor.count; ++index) {
                                mesh_node.indices[index + starting_indicies_index] = buf[index];
                            }
                            break;
                        }
                        default:
                            std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
                    }
                }

                const tinygltf::Material& material = model.materials[primitive.material];
                mesh_node.material_id = material.name;
            }
            out_nodes.push_back(mesh_node);
        }
    }
    for (const tinygltf::Material& material: model.materials) {
        tinygltf::Texture base_color = model.textures[material.pbrMetallicRoughness.baseColorTexture.index];
        if (base_color.source != -1) {
            tinygltf::Image &image = model.images[base_color.source];
            // Get the image data from the glTF loader
            unsigned char* buffer = nullptr;
            bool deleteBuffer = false;
            uint64_t bufferSize = 0;
            // We convert RGB-only images to RGBA, as most devices don't support RGB-formats in Vulkan
            if (image.component == 3) {
                bufferSize = image.width * image.height * 4;
                buffer = new unsigned char[bufferSize];
                unsigned char* rgba = buffer;
                unsigned char* rgb = &image.image[0];
                for (size_t i = 0; i < image.width * image.height; ++i) {
                    memcpy(rgba, rgb, sizeof(unsigned char) * 3);
                    rgba += 4;
                    rgb += 3;
                }
                deleteBuffer = true;
            }
            else {
                buffer = &image.image[0];
                bufferSize = image.image.size();
            }

            WGPUTextureDescriptor texture_desc = {};
            texture_desc.nextInChain = nullptr;
            texture_desc.dimension = WGPUTextureDimension_2D;
            texture_desc.format = WGPUTextureFormat_RGBA8Unorm; // by convention for bmp, png and jpg file. Be careful with other formats.
            texture_desc.size = { (unsigned int)image.width, (unsigned int)image.height, 1 };
            texture_desc.mipLevelCount = bit_width(std::max(texture_desc.size.width, texture_desc.size.height));;
            texture_desc.sampleCount = 1;
            texture_desc.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
            texture_desc.viewFormatCount = 0;
            texture_desc.viewFormats = nullptr;
            WGPUTexture texture = wgpuDeviceCreateTexture(p_device, &texture_desc);
            write_mip_maps(p_device, texture, texture_desc.size, texture_desc.mipLevelCount, buffer);
            WGPUTextureViewDescriptor texture_view_desc = {};
            texture_view_desc.aspect = WGPUTextureAspect_All;
            texture_view_desc.baseArrayLayer = 0;
            texture_view_desc.arrayLayerCount = 1;
            texture_view_desc.baseMipLevel = 0;
            texture_view_desc.mipLevelCount = texture_desc.mipLevelCount;
            texture_view_desc.dimension = WGPUTextureViewDimension_2D;
            texture_view_desc.format = texture_desc.format;
            Material out_mat;
            out_mat.id = material.name;
            out_mat.base_color_view = wgpuTextureCreateView(texture, &texture_view_desc);
            out_mat.base_color_texture = texture;
            if (deleteBuffer) {
                delete[] buffer;
            }
            out_materials.push_back(out_mat);
        }
    }
    return std::make_pair(out_materials, out_nodes);
}

}