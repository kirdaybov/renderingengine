#include "imguirenderable.h"
#include "shadercompiler/shaderbinary.h"
#include "app/application.h"
#include "vulkansubsystem.h"

void ImGuiRenderable::CreateImGuiPipeline()
{
  ShaderBinary* vsShader = gApplicationInstanceManager.GetInstance().GetShaderCompiler().GetShader("uivs");
  ShaderBinary* psShader = gApplicationInstanceManager.GetInstance().GetShaderCompiler().GetShader("uips");

  VkShaderModule vertShaderModule = gRenderer.CreateShaderModule(vsShader->GetData(), vsShader->GetSize());
  VkShaderModule fragShaderModule = gRenderer.CreateShaderModule(psShader->GetData(), psShader->GetSize());

  VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
  vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
  fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

  auto bindingDescription = ImGuiVertexDescriptor::GetBindingDescription();
  auto attributeDescription = ImGuiVertexDescriptor::GetAttributeDescriptions();

  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
  vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport = {};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)gRenderer.GetSwapChainExtent().width;
  viewport.height = (float)gRenderer.GetSwapChainExtent().height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor = {};
  scissor.offset = { 0, 0 };
  scissor.extent = gRenderer.GetSwapChainExtent();

  VkPipelineViewportStateCreateInfo viewportState = {};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterizer = {};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE; // ?
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_NONE;
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE; // ?
  rasterizer.depthBiasConstantFactor = 0.0f; // Optional
  rasterizer.depthBiasClamp = 0.0f; // Optional
  rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

  VkPipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  // multisampling.sampleShadingEnable = VK_FALSE; // ?
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f; // Optional
  multisampling.pSampleMask = nullptr; // Optional
  multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
  multisampling.alphaToOneEnable = VK_FALSE; // Optional

  VkPipelineDepthStencilStateCreateInfo depthStencil = {};
  depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.depthTestEnable = VK_FALSE;
  depthStencil.depthWriteEnable = VK_FALSE;
  depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
  //depthStencil.depthBoundsTestEnable = VK_FALSE;
  //depthStencil.minDepthBounds = 0.0f; // Optional
  //depthStencil.maxDepthBounds = 1.0f; // Optional
  //depthStencil.stencilTestEnable = VK_FALSE;
  depthStencil.front = depthStencil.back; // Optional
  depthStencil.back.compareOp = VK_COMPARE_OP_ALWAYS; // Optional

  VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
  colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_TRUE;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

  VkPipelineColorBlendStateCreateInfo colorBlending = {};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  //colorBlending.logicOpEnable = VK_FALSE;
  //colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  //colorBlending.blendConstants[0] = 0.0f; // Optional
  //colorBlending.blendConstants[1] = 0.0f; // Optional
  //colorBlending.blendConstants[2] = 0.0f; // Optional
  //colorBlending.blendConstants[3] = 0.0f; // Optional

  VkDynamicState dynamicStateEnables[2] =
  {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
  };

  VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo;
  pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStateEnables;
  pipelineDynamicStateCreateInfo.dynamicStateCount = 2;
  pipelineDynamicStateCreateInfo.pNext = nullptr;
  pipelineDynamicStateCreateInfo.flags = 0;

  VkPushConstantRange pushConstantRange;
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(ImGuiConst);

  VkDescriptorSetLayoutBinding setLayoutBinding;
  setLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  setLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  setLayoutBinding.binding = 0;
  setLayoutBinding.descriptorCount = 1;

  VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
  samplerLayoutBinding.binding = 1;
  samplerLayoutBinding.descriptorCount = 1;
  samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  samplerLayoutBinding.pImmutableSamplers = nullptr;
  samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
      setLayoutBinding,
      samplerLayoutBinding
  };

  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
  descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptorSetLayoutCreateInfo.pBindings = setLayoutBindings.data();
  descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());

  if (vkCreateDescriptorSetLayout(gRenderer.GetDevice(), &descriptorSetLayoutCreateInfo, nullptr, &m_ImGuiDescriptorSetLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create descriptor set layout!");
  }

  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
  pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutCreateInfo.setLayoutCount = 1;
  pipelineLayoutCreateInfo.pSetLayouts = &m_ImGuiDescriptorSetLayout;
  pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
  pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
  pipelineLayoutCreateInfo.pNext = nullptr;
  pipelineLayoutCreateInfo.flags = 0;

  if (vkCreatePipelineLayout(gRenderer.GetDevice(), &pipelineLayoutCreateInfo, nullptr, &m_ImGuiPipelineLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create pipeline layout!");
  }

  VkGraphicsPipelineCreateInfo pipelineInfo = {};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = &depthStencil;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &pipelineDynamicStateCreateInfo;
  pipelineInfo.layout = m_ImGuiPipelineLayout;
  pipelineInfo.renderPass = gRenderer.GetRenderPass(); //TODO: examine later
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
  pipelineInfo.basePipelineIndex = -1; // Optional

  if (vkCreateGraphicsPipelines(gRenderer.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_ImGuiPipeline) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create graphics pipeline!");
  }

  vkDestroyShaderModule(gRenderer.GetDevice(), fragShaderModule, nullptr);
  vkDestroyShaderModule(gRenderer.GetDevice(), vertShaderModule, nullptr);
}

void ImGuiRenderable::CreateDescriptorSets()
{
  VkDescriptorPoolSize poolSize = {};
  poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSize.descriptorCount = 1;

  VkDescriptorPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = 1;
  poolInfo.pPoolSizes = &poolSize;
  poolInfo.maxSets = 1;

  if (vkCreateDescriptorPool(gRenderer.GetDevice(), &poolInfo, nullptr, &m_ImGuiDescriptorPool) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create descriptor pool!");
  }

  VkDescriptorSetAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = m_ImGuiDescriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &m_ImGuiDescriptorSetLayout;

  VK_CHECK(vkAllocateDescriptorSets(gRenderer.GetDevice(), &allocInfo, &m_ImGuiDescriptorSet));

  // ImGui
  VkDescriptorImageInfo imageInfo = {};
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  imageInfo.imageView = m_ImGuiFontView;
  imageInfo.sampler = m_ImGuiFontSampler;

  VkWriteDescriptorSet descriptorWrite = {};
  descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrite.dstSet = m_ImGuiDescriptorSet;
  descriptorWrite.dstBinding = 1;
  descriptorWrite.dstArrayElement = 0;
  descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  descriptorWrite.descriptorCount = 1;
  descriptorWrite.pImageInfo = &imageInfo;

  vkUpdateDescriptorSets(gRenderer.GetDevice(), 1, &descriptorWrite, 0, nullptr);
}

void ImGuiRenderable::CreateImGuiBuffers()
{
  ImGuiIO& io = ImGui::GetIO();

  // Create font texture
  unsigned char* fontData;
  int texWidth, texHeight;
  io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
  VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);

  // Staging buffers for font data upload
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  gRenderer.CreateBuffer(uploadSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

  void* data;
  vkMapMemory(gRenderer.GetDevice(), stagingBufferMemory, 0, uploadSize, 0, &data);
  memcpy(data, fontData, uploadSize);
  vkUnmapMemory(gRenderer.GetDevice(), stagingBufferMemory);

  gRenderer.CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_ImGuiFontImage, m_ImGuiFontMemory);
  gRenderer.TransitionImageLayout(m_ImGuiFontImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  gRenderer.CopyBufferToImage(stagingBuffer, m_ImGuiFontImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
  gRenderer.TransitionImageLayout(m_ImGuiFontImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  // Image view
  VkImageViewCreateInfo viewInfo = {};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = m_ImGuiFontImage;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.layerCount = 1;
  VK_CHECK(vkCreateImageView(gRenderer.GetDevice(), &viewInfo, nullptr, &m_ImGuiFontView));

  vkDestroyBuffer(gRenderer.GetDevice(), stagingBuffer, nullptr);
  vkFreeMemory(gRenderer.GetDevice(), stagingBufferMemory, nullptr);

  // Font texture Sampler
  VkSamplerCreateInfo samplerInfo = {};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.maxAnisotropy = 1.0f;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
  VK_CHECK(vkCreateSampler(gRenderer.GetDevice(), &samplerInfo, nullptr, &m_ImGuiFontSampler));
}

void ImGuiRenderable::UpdateImGuiBuffers(bool unmap)
{
  ImDrawData* imDrawData = ImGui::GetDrawData();
  if (!imDrawData)
  {
    return;
  }

  // Note: Alignment is done inside buffer creation
  VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
  VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

  if ((vertexBufferSize == 0) || (indexBufferSize == 0)) {
    return;
  }

  // Update buffers only if vertex or index count has been changed compared to current buffer size

  // Vertex buffer
  //if ((m_ImGuiVertexBuffer.buffer == VK_NULL_HANDLE) || (vertexCount != imDrawData->TotalVtxCount))
  void* vertexData;
  void* indexData;

  {
    if (m_ImGuiVertexBufferMemory)
    {
      vkUnmapMemory(gRenderer.GetDevice(), m_ImGuiVertexBufferMemory);
    }
    if (m_ImGuiVertexBuffer)
    {
      vkDestroyBuffer(gRenderer.GetDevice(), m_ImGuiVertexBuffer, nullptr);
    }
    if (m_ImGuiVertexBufferMemory)
    {
      vkFreeMemory(gRenderer.GetDevice(), m_ImGuiVertexBufferMemory, nullptr);
    }
    gRenderer.CreateBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, m_ImGuiVertexBuffer, m_ImGuiVertexBufferMemory);
    //VK_CHECK_RESULT(device->createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &vertexBuffer, vertexBufferSize));
    //vertexCount = imDrawData->TotalVtxCount;
    //vertexBuffer.unmap();
    vkMapMemory(gRenderer.GetDevice(), m_ImGuiVertexBufferMemory, 0, VK_WHOLE_SIZE, 0, &vertexData);
  }

  // Index buffer
  //VkDeviceSize indexSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);
  //if ((indexBuffer.buffer == VK_NULL_HANDLE) || (indexCount < imDrawData->TotalIdxCount))
  {
    if (m_ImGuiIndexBufferMemory)
    {
      vkUnmapMemory(gRenderer.GetDevice(), m_ImGuiIndexBufferMemory);
    }
    if (m_ImGuiIndexBuffer)
    {
      vkDestroyBuffer(gRenderer.GetDevice(), m_ImGuiIndexBuffer, nullptr);
    }
    if (m_ImGuiIndexBufferMemory)
    {
      vkFreeMemory(gRenderer.GetDevice(), m_ImGuiIndexBufferMemory, nullptr);
    }
    gRenderer.CreateBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, m_ImGuiIndexBuffer, m_ImGuiIndexBufferMemory);

    vkMapMemory(gRenderer.GetDevice(), m_ImGuiIndexBufferMemory, 0, VK_WHOLE_SIZE, 0, &indexData);
  }

  // Upload data
  ImDrawVert* vtxDst = (ImDrawVert*)vertexData;
  ImDrawIdx* idxDst = (ImDrawIdx*)indexData;

  for (int n = 0; n < imDrawData->CmdListsCount; n++) {
    const ImDrawList* cmd_list = imDrawData->CmdLists[n];
    memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
    memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
    vtxDst += cmd_list->VtxBuffer.Size;
    idxDst += cmd_list->IdxBuffer.Size;
  }

  // Flush to make writes visible to GPU
  VkMappedMemoryRange mappedRange = {};
  mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  mappedRange.memory = m_ImGuiVertexBufferMemory;
  mappedRange.offset = 0;
  mappedRange.size = VK_WHOLE_SIZE;
  vkFlushMappedMemoryRanges(gRenderer.GetDevice(), 1, &mappedRange);

  //VkMappedMemoryRange mappedRange;
  //mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  mappedRange.memory = m_ImGuiIndexBufferMemory;
  mappedRange.offset = 0;
  mappedRange.size = VK_WHOLE_SIZE;
  vkFlushMappedMemoryRanges(gRenderer.GetDevice(), 1, &mappedRange);
}

void ImGuiRenderable::Init()
{
  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  io.DisplaySize = ImVec2(static_cast<float>(gRenderer.GetSwapChainExtent().width), static_cast<float>(gRenderer.GetSwapChainExtent().height));
  io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

  CreateImGuiPipeline();
  CreateImGuiBuffers();
  CreateDescriptorSets();
  UpdateImGuiBuffers(false);
}

void ImGuiRenderable::Update()
{
  ImGui::Render();

  UpdateImGuiBuffers(false);
}

void ImGuiRenderable::Render(VkCommandBuffer commandBuffer)
{
  // ImGui start
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ImGuiPipeline);
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ImGuiPipelineLayout, 0, 1, &m_ImGuiDescriptorSet, 0, nullptr);

  VkViewport viewport = {};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)gRenderer.GetSwapChainExtent().width;
  viewport.height = (float)gRenderer.GetSwapChainExtent().height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  ImGuiIO& io = ImGui::GetIO();

  m_ImGuiConst.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
  m_ImGuiConst.translate = glm::vec2(-1.0f);
  vkCmdPushConstants(commandBuffer, m_ImGuiPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ImGuiConst), &m_ImGuiConst);

  // Render commands
  ImDrawData* imDrawData = ImGui::GetDrawData();
  int32_t vertexOffset = 0;
  int32_t indexOffset = 0;

  if (imDrawData && imDrawData->CmdListsCount > 0) {

    VkDeviceSize offsets[1] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_ImGuiVertexBuffer, offsets);
    vkCmdBindIndexBuffer(commandBuffer, m_ImGuiIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

    for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
    {
      const ImDrawList* cmd_list = imDrawData->CmdLists[i];
      for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
      {
        const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
        VkRect2D scissorRect;
        scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
        scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
        scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
        scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);

        vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);
        vkCmdDrawIndexed(commandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
        indexOffset += pcmd->ElemCount;
      }
      vertexOffset += cmd_list->VtxBuffer.Size;
    }
  }
}

void ImGuiRenderable::Cleanup()
{
  vkDestroyPipeline(gRenderer.GetDevice(), m_ImGuiPipeline, nullptr);
  vkDestroyPipelineLayout(gRenderer.GetDevice(), m_ImGuiPipelineLayout, nullptr);
  vkDestroyDescriptorPool(gRenderer.GetDevice(), m_ImGuiDescriptorPool, nullptr);
  vkDestroyDescriptorSetLayout(gRenderer.GetDevice(), m_ImGuiDescriptorSetLayout, nullptr);
}