#include "lve_pipeline.hpp"
#include "lve_model.hpp"

#include <fstream>
#include <stdexcept>
#include <iostream>

namespace lve {
    std::vector<char> LvePipeline::ReadFile(const std::string& file_name) {

        // ios::ate = bit flag to seek end immediately after opening. easier to get size.
        // ios::binary = bit flag to read as binary. prevents unwanted text/string transform.
        std::ifstream file{file_name, std::ios::ate | std::ios::binary};
        if (!file.is_open()) {
            throw std::runtime_error("failed to open file" + file_name);
        }
        // [tellg] gets position of stream/file/char. Since we start from the end due to ios::ate,
        // we can use this to get the numel of chars, which is used for initializing buffer.
        size_t file_size = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(file_size);

        // [seekg] sets position of stream/file to an index. which is 0/start for our use case.
        // this is used to start reading the data in the file.
        file.seekg(0);
        // [read] stores "file_size" amount of data from current stream position into pointer
        // given by buffer.data().
        file.read(buffer.data(), file_size);
        file.close();
        return buffer;
    }

    void LvePipeline::CreateGraphicPipeline(const std::string& vert_file_path,
                                            const std::string& frag_file_path,
                                            const PipelineConfigInfo& config_info) {
        auto vertCode = ReadFile(vert_file_path);
        auto fragCode = ReadFile(frag_file_path);

        CreateShaderModule(vertCode, &vert_shader_module_);
        CreateShaderModule(fragCode, &frag_shader_module_);

        VkPipelineShaderStageCreateInfo shader_stages[2];
        shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shader_stages[0].module = vert_shader_module_;
        // pName the name to the entry function in shader.
        shader_stages[0].pName = "main";
        // 0 => set no flags.
        shader_stages[0].flags = 0;
        shader_stages[0].pNext = nullptr;
        // Customizes shader functionality.
        shader_stages[0].pSpecializationInfo = nullptr;

        shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shader_stages[1].module = frag_shader_module_;
        // pName the name to the entry function in shader.
        shader_stages[1].pName = "main";
        // 0 => set no flags.
        shader_stages[1].flags = 0;
        shader_stages[1].pNext = nullptr;
        // Customizes shader functionality.
        shader_stages[1].pSpecializationInfo = nullptr;

        VkPipelineVertexInputStateCreateInfo vertex_input_info{};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        // Setting for supplying data.
        auto attributeDescriptions = LveModel::Vertex::getAttributeDescriptions();
        auto bindingDescriptions = LveModel::Vertex::getBindingDescriptions();
        vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        vertex_input_info.pVertexAttributeDescriptions = attributeDescriptions.data();
        vertex_input_info.pVertexBindingDescriptions = bindingDescriptions.data();

        // Fuse scissor and viewport into ViewPortInfo
        // Some GPU can use multiple ViewPortInfo
        VkPipelineViewportStateCreateInfo view_port_info{};
        view_port_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        view_port_info.viewportCount = 1;
        view_port_info.pViewports = &config_info.viewport;
        view_port_info.scissorCount = 1;
        view_port_info.pScissors = &config_info.scissor;

        VkGraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        // stageCount specify how many programmable stage our pipeline uses.
        pipeline_info.stageCount = 2;
        // Pointer to the shader stages and modules.
        pipeline_info.pStages = shader_stages;
        pipeline_info.pVertexInputState = &vertex_input_info;
        pipeline_info.pInputAssemblyState = &config_info.inputAssemblyInfo;
        pipeline_info.pViewportState = &view_port_info;
        pipeline_info.pRasterizationState = &config_info.rasterizationInfo;
        pipeline_info.pColorBlendState = &config_info.colorBlendInfo;
        pipeline_info.pDepthStencilState = &config_info.depthStencilInfo;
        pipeline_info.pDynamicState = nullptr;

        pipeline_info.layout = config_info.pipelineLayout;
        pipeline_info.renderPass = config_info.renderPass;
        pipeline_info.subpass = config_info.subpass;

        // Set to derive new graphic pipeline for existing one.
        // Might be more cost efficient for GPU.
        pipeline_info.basePipelineIndex = -1;
        pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

        if(vkCreateGraphicsPipelines(lve_device_.device(), /*pipeline cache*/ VK_NULL_HANDLE, /*pipeline count*/ 1,
                                    &pipeline_info, /*alloc callback*/ nullptr, &graphics_pipeline_) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create graphics pipeline.");
        }

    }

    LvePipeline::LvePipeline(LveDevice &device,
                const std::string& vert_file_path,
                const std::string& frag_file_path,
                const PipelineConfigInfo& config_info) : lve_device_{device} {
        CreateGraphicPipeline(vert_file_path, frag_file_path, config_info);
    }

    LvePipeline::~LvePipeline() {
        vkDestroyShaderModule(lve_device_.device(), vert_shader_module_, nullptr);
        vkDestroyShaderModule(lve_device_.device(), frag_shader_module_, nullptr);
        vkDestroyPipeline(lve_device_.device(), graphics_pipeline_, nullptr);
    }

    // Bind a pipeline object to a command buffer
    void LvePipeline::bind(VkCommandBuffer command_buffer) {
        // VK_PIPELINE_BIND_POINT_GRAPHICS = indicate that it is graphic pipeline
        // VK_PIPELINE_BIND_POINT_COMPUTE = indicate that it is compute pipeline
        // VK_PIPELINE_BIND_POINT_RAY_TRACER = indicate that it is ray tracing pipeline
        // No check needed since, check already done during initialization for command_buffer and
        // graphic pipeline.
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_);
    }


    void LvePipeline::CreateShaderModule(const std::vector<char>& code, VkShaderModule* output_module) {
        VkShaderModuleCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize = code.size();
        // Although uint32_t.size != char.size, since we store code in vector, allocator
        // already ensure worst case data alignment requriement. Won't work if we use c-style array.
        create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

        if(vkCreateShaderModule(lve_device_.device(), &create_info, nullptr, output_module) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create shader module.");
        }
    }

    PipelineConfigInfo LvePipeline::defaultPipelineConfigInfo(uint32_t width, uint32_t height) {
        PipelineConfigInfo config_info{};
        // Setting up first stage of pipeline/Input Assembler.
        // Takes in list of vertices and group them as geometry.
        config_info.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST takes in list of vertex and group every 3 vertices as a triangle point.
        // i.e a,b,c,d,e,f -> (a,b),(c,d),(e,f) -> triangle pt1, triangle pt2, triangle pt3.
        // triangle_strip: a different topology that takes in the next vertice to form a triangle from the last
        // two vertices.
        config_info.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        config_info.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        // Viewport describe transformation between pipeline output and target image.
        // pipeline output is from (-1,-1) to (1,1) where (0,0) is center.
        // Viewport convert from pipeline output to pixel coordinates.
        config_info.viewport.x = 0.0f;
        config_info.viewport.y = 0.0f;
        config_info.viewport.width = static_cast<float>(width);
        config_info.viewport.height = static_cast<float>(height);
        // Depth range for viewport to linear transform z position.
        config_info.viewport.minDepth = 0.0f;
        config_info.viewport.maxDepth = 1.0f;

        // Cuts images, any pixel outside of the sciccor rectangle
        // described below will get discarded.
        config_info.scissor.offset = {0, 0};
        config_info.scissor.extent = {width, height};

        // Rasetrization stage: breaks up geometry into fragments for each pixel.
        config_info.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        // Clamp depth/Z value to be [0.0, 1.0]. Don't want this to be true.
        // if less than 0.0, than object behind camera, if greater than 1.0, then too far to see.
        // Would require GPU feature.
        config_info.rasterizationInfo.depthClampEnable = VK_FALSE;
        // Discard primitive all primitive before rasterization.
        // Useful for when only want to use first few stages of grpahic pipeline.
        config_info.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        // For draw triangle: just corners, edges, or line filled in.
        config_info.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        config_info.rasterizationInfo.lineWidth = 1.0f;
        // Can discard triangle based on apparent facing/winding order.
        config_info.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        // based on the order of vertex + cull order(CW, CCW) we give,
        // we can know which side of triangle we are looking at.
        config_info.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        // Alter depth value by adding constant value, or multiply by slope factor.
        config_info.rasterizationInfo.depthBiasEnable = VK_FALSE;
        config_info.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
        config_info.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
        config_info.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional

        // Anti-Aliasing stage:
        // Without aliasing, pixel can be in or out of geometry/shape
        // Leads to jagged features. MSAA/MultiSample Anti Aliasing
        // fixes this by subdividing the pixel into block, and color it more
        // or less based on how much of the subpixel is in geometery/shape.
        config_info.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        config_info.multisampleInfo.sampleShadingEnable = VK_FALSE;
        config_info.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        config_info.multisampleInfo.minSampleShading = 1.0f;           // Optional
        config_info.multisampleInfo.pSampleMask = nullptr;             // Optional
        config_info.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
        config_info.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional

        // Color blending stage: How to combine color in frame buffer.
        // If there's two triangle overlapping, then fragment shader will return
        // multiple color for some pixels. Or how to mix color when there is already
        // a color in the frame buffer/ pixel.
        config_info.colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        config_info.colorBlendAttachment.blendEnable = VK_FALSE;
        config_info.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
        config_info.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
        config_info.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
        config_info.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
        config_info.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
        config_info.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

        config_info.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        config_info.colorBlendInfo.logicOpEnable = VK_FALSE;
        config_info.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
        config_info.colorBlendInfo.attachmentCount = 1;
        config_info.colorBlendInfo.pAttachments = &config_info.colorBlendAttachment;
        config_info.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
        config_info.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
        config_info.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
        config_info.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

        // Depth Buffer stage
        // Do not keep track of all layers, but only keep track of what is closest to camera/shortest depth.
        // range: [0.0, 1.0], 0 is closest, 1 is furthest.
        config_info.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        config_info.depthStencilInfo.depthTestEnable = VK_TRUE;
        config_info.depthStencilInfo.depthWriteEnable = VK_TRUE;
        config_info.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        config_info.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        config_info.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
        config_info.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
        config_info.depthStencilInfo.stencilTestEnable = VK_FALSE;
        config_info.depthStencilInfo.front = {};  // Optional
        config_info.depthStencilInfo.back = {};   // Optional
        return config_info;
    }


} // namespace lve