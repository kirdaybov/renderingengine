#include "meshrenderable.h"
#include "shadercompiler/shaderbinary.h"
#include "app/application.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "assetmanager/assetmanager.h"
#include <fbxsdk.h>

struct UniformBufferObject
{
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
  glm::fvec3 sunDirection;
};

void MeshRenderable::CreateGraphicsPipeline()
{
  ShaderBinary* vsShader = gApp.GetShaderCompiler().GetShader("meshvs");
  ShaderBinary* psShader = gApp.GetShaderCompiler().GetShader("meshps");

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

  auto bindingDescription = Vertex::GetBindingDescription();
  auto attributeDescription = Vertex::GetAttributeDescriptions();

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
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f; // Optional
  rasterizer.depthBiasClamp = 0.0f; // Optional
  rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

  VkPipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f; // Optional
  multisampling.pSampleMask = nullptr; // Optional
  multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
  multisampling.alphaToOneEnable = VK_FALSE; // Optional

  VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
  colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
  colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

  VkPipelineColorBlendStateCreateInfo colorBlending = {};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f; // Optional
  colorBlending.blendConstants[1] = 0.0f; // Optional
  colorBlending.blendConstants[2] = 0.0f; // Optional
  colorBlending.blendConstants[3] = 0.0f; // Optional

  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1; // Optional
  pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout; // Optional
  pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
  pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
  pipelineLayoutInfo.pNext = nullptr;
  pipelineLayoutInfo.flags = 0;

  VK_CHECK(vkCreatePipelineLayout(gRenderer.GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout));

  VkPipelineDepthStencilStateCreateInfo depthStencil = {};
  depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.depthTestEnable = VK_TRUE;
  depthStencil.depthWriteEnable = VK_TRUE;
  depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
  depthStencil.depthBoundsTestEnable = VK_FALSE;
  depthStencil.minDepthBounds = 0.0f; // Optional
  depthStencil.maxDepthBounds = 1.0f; // Optional
  depthStencil.stencilTestEnable = VK_FALSE;
  depthStencil.front = {}; // Optional
  depthStencil.back = {}; // Optional

  VkGraphicsPipelineCreateInfo pipelineInfo = {};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = &depthStencil; // Optional
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = nullptr; // Optional
  pipelineInfo.layout = m_PipelineLayout;
  pipelineInfo.renderPass = gRenderer.GetRenderPass();
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
  pipelineInfo.basePipelineIndex = -1; // Optional

  VK_CHECK(vkCreateGraphicsPipelines(gRenderer.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline));

  vkDestroyShaderModule(gRenderer.GetDevice(), fragShaderModule, nullptr);
  vkDestroyShaderModule(gRenderer.GetDevice(), vertShaderModule, nullptr);
}

void MeshRenderable::CreateVertexBuffer()
{
  VkDeviceSize bufferSize = sizeof(Vertex) * m_Vertices.size();

  Buffer stagingBuffer;
  gRenderer.CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer);
  stagingBuffer.CopyDataToBufferMemory(gRenderer.GetDevice(), bufferSize, m_Vertices.data());
  gRenderer.CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VertexBuffer);
  gRenderer.CopyBuffer(stagingBuffer, m_VertexBuffer, bufferSize);
  stagingBuffer.Cleanup();
}

void MeshRenderable::CreateIndexBuffer()
{
  VkDeviceSize bufferSize = sizeof(m_Indices[0]) * m_Indices.size();

  Buffer stagingBuffer;
  gRenderer.CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer);
  stagingBuffer.CopyDataToBufferMemory(gRenderer.GetDevice(), bufferSize, m_Indices.data());
  gRenderer.CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_IndexBuffer);
  gRenderer.CopyBuffer(stagingBuffer, m_IndexBuffer, bufferSize);
  stagingBuffer.Cleanup();
}

void MeshRenderable::CreateDescriptorSetLayout()
{
  std::array<VkDescriptorSetLayoutBinding, 3> bindings;

  // ubo layout binding
  bindings[0].binding = 0;
  bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  bindings[0].descriptorCount = 1;
  bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  bindings[0].pImmutableSamplers = nullptr;

  // texture binding
  bindings[1].binding = 1;
  bindings[1].descriptorCount = 1;
  bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  bindings[1].pImmutableSamplers = nullptr;
  bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  // normal binding
  bindings[2].binding = 2;
  bindings[2].descriptorCount = 1;
  bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  bindings[2].pImmutableSamplers = nullptr;
  bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutCreateInfo layoutInfo = {};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  layoutInfo.pBindings = bindings.data();

  if (vkCreateDescriptorSetLayout(gRenderer.GetDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create descriptor set layout!");
  }
}

void MeshRenderable::CreateUniformBuffer()
{
  VkDeviceSize bufferSize = sizeof(UniformBufferObject);
  gRenderer.CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_UniformBuffer);
}

void MeshRenderable::UpdateUniformBuffer()
{
  static auto startTime = std::chrono::high_resolution_clock::now();

  auto currentTime = std::chrono::high_resolution_clock::now();
  float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
  UniformBufferObject ubo = {};
  const GameState& gameState = gApp.GetGameState();
  auto model = glm::rotate(glm::mat4(1.0f), glm::radians(gameState.GetRotation(0)), glm::vec3(1.0f, 0.0f, 0.0f));
  model = glm::rotate(model, glm::radians(gameState.GetRotation(1)), glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::rotate(model, glm::radians(gameState.GetRotation(2)), glm::vec3(0.0f, 0.0f, 1.0f));

  ubo.model = model;
  ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.proj = glm::perspective(glm::radians(45.0f), gRenderer.GetSwapChainExtent().width / (float)gRenderer.GetSwapChainExtent().height, 0.1f, 10.0f);
  ubo.proj[1][1] *= -1;
  ubo.sunDirection = glm::fvec3(gameState.GetSunLightDirection(0), gameState.GetSunLightDirection(1), gameState.GetSunLightDirection(2));

  m_UniformBuffer.CopyDataToBufferMemory(gRenderer.GetDevice(), sizeof(ubo), &ubo);
}

void MeshRenderable::CreateDescriptorPool()
{
  int swapChainImageCount = static_cast<int>(gRenderer.GetSwapChainImagesCount());
  std::array<VkDescriptorPoolSize, 2> poolSizes = {};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChainImageCount);
  poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChainImageCount);
  
  VkDescriptorPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.maxSets = static_cast<uint32_t>(swapChainImageCount);

  if (vkCreateDescriptorPool(gRenderer.GetDevice(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create descriptor pool!");
  }
}

void MeshRenderable::CreateDescriptorSets()
{
  int swapChainImageCount = static_cast<int>(gRenderer.GetSwapChainImagesCount());
  std::vector<VkDescriptorSetLayout> layouts(swapChainImageCount, m_DescriptorSetLayout);
  VkDescriptorSetAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = m_DescriptorPool;
  allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
  allocInfo.pSetLayouts = layouts.data();

  m_DescriptorSets.resize(swapChainImageCount);
  VK_CHECK(vkAllocateDescriptorSets(gRenderer.GetDevice(), &allocInfo, m_DescriptorSets.data()));

  for (size_t i = 0; i < swapChainImageCount; i++)
  {
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = m_UniformBuffer.GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    std::array<VkDescriptorImageInfo, 2> imageInfos = {};
    imageInfos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfos[0].imageView = m_TextureImageView;
    imageInfos[0].sampler = m_TextureSampler;

    imageInfos[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfos[1].imageView = m_NormalImageView;
    imageInfos[1].sampler = m_NormalSampler;

    std::array<VkWriteDescriptorSet, 3> descriptorWrites = {};
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = m_DescriptorSets[i];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;
    descriptorWrites[0].pImageInfo = nullptr; // Optional
    descriptorWrites[0].pTexelBufferView = nullptr; // Optional

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = m_DescriptorSets[i];
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &imageInfos[0];

    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[2].dstSet = m_DescriptorSets[i];
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pImageInfo = &imageInfos[1];

    vkUpdateDescriptorSets(gRenderer.GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
  }
}

void MeshRenderable::CreateImage(const char* name, VkImage& image, VkDeviceMemory& memory)
{
  int texWidth, texHeight, texChannels;
  stbi_uc* pixels = stbi_load(*gAssetManager.GetAsset(name)->m_Path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  VkDeviceSize imageSize = texWidth * texHeight * 4;

  if (!pixels)
  {
    throw std::runtime_error("failed to load texture image!");
  }

  Buffer stagingBuffer;

  gRenderer.CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer);

  stagingBuffer.CopyDataToBufferMemory(gRenderer.GetDevice(), imageSize, pixels);

  stbi_image_free(pixels);

  gRenderer.CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, memory);
  gRenderer.TransitionImageLayout(image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  gRenderer.CopyBufferToImage(stagingBuffer, image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
  gRenderer.TransitionImageLayout(image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  stagingBuffer.Cleanup();
}

void MeshRenderable::CreateTextureImage()
{
  CreateImage("cube_DefaultMaterial_BaseColor", m_TextureImage, m_TextureImageMemory);
  CreateImage("cube_DefaultMaterial_Normal", m_NormalImage, m_NormalImageMemory);
}

void MeshRenderable::CreateTextureImageView()
{
  m_TextureImageView = gRenderer.CreateImageView(m_TextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
  m_NormalImageView = gRenderer.CreateImageView(m_NormalImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
}

void MeshRenderable::CreateTextureSampler()
{
  VkSamplerCreateInfo samplerInfo = {};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.anisotropyEnable = VK_TRUE;
  samplerInfo.maxAnisotropy = 16;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = 0.0f;

  VK_CHECK(vkCreateSampler(gRenderer.GetDevice(), &samplerInfo, nullptr, &m_TextureSampler));
  VK_CHECK(vkCreateSampler(gRenderer.GetDevice(), &samplerInfo, nullptr, &m_NormalSampler));
}

void MeshRenderable::LoadModelFBX()
{
  FbxManager* fbxSdkManager = FbxManager::Create();
  FbxIOSettings* ios = FbxIOSettings::Create(fbxSdkManager, IOSROOT);
  fbxSdkManager->SetIOSettings(ios);

  FbxImporter* fbxImporter = FbxImporter::Create(fbxSdkManager, "");

  const char* filename = *gAssetManager.GetAsset("cube")->m_Path;
  // Use the first argument as the filename for the importer.
  if (!fbxImporter->Initialize(filename, -1, fbxSdkManager->GetIOSettings()))
  {
    printf("Call to FbxImporter::Initialize() failed.\n");
    printf("Error returned: %s\n\n", fbxImporter->GetStatus().GetErrorString());
    exit(-1);
  }

  FbxScene* fbxScene = FbxScene::Create(fbxSdkManager, "myScene");
  FbxAxisSystem axisSystem = fbxScene->GetGlobalSettings().GetAxisSystem();
  fbxImporter->Import(fbxScene);
  fbxImporter->Destroy();

  //TODO: recursion
  if (FbxNode* rootNode = fbxScene->GetRootNode())
  {
    for (int nodeIdx = 0; nodeIdx < rootNode->GetChildCount(); ++nodeIdx)
    {
      FbxNode* childNode = rootNode->GetChild(nodeIdx);
      if (FbxMesh* mesh = childNode->GetMesh())
      {
        FbxGeometryElementUV* leUV = mesh->GetElementUV(0);
        FbxGeometryElementNormal* leNormal = mesh->GetElementNormal(0);
        FbxGeometryElementTangent* leTangent = mesh->GetElementTangent(0);
        FbxGeometryElementBinormal* leBitangent = mesh->GetElementBinormal(0);
        for (int polyIdx = 0; polyIdx < mesh->GetPolygonCount(); ++polyIdx)
        {
          for (int subIdx = 0; subIdx < mesh->GetPolygonSize(polyIdx); ++subIdx)
            //for (int vertexIdx = 0; vertexIdx < mesh->GetControlPointsCount(); ++vertexIdx)
          {
            int vertexIdx = mesh->GetPolygonVertex(polyIdx, subIdx);
            FbxVector4 position = mesh->GetControlPointAt(vertexIdx);

            Vertex vertex = {};

            vertex.m_Position =
            {
              float(position[0]),
              float(position[1]),
              float(position[2])
            };

            //FbxVector4 normal = leNormal->GetDirectArray().GetAt(vertexIdx);
            //vertex.m_Normal = { normal[0], normal[1], normal[2] };

            //FbxVector4 tangent = leTangent->GetDirectArray().GetAt(vertexIdx);
            //vertex.m_Tangent = { tangent[0], tangent[1], tangent[2], tangent[3]};

            int lTextureUVIndex = mesh->GetTextureUVIndex(polyIdx, subIdx);
            FbxVector2 uv = leUV->GetDirectArray().GetAt(lTextureUVIndex);
            vertex.m_TexCoord = { uv[0], -uv[1] };
            m_Vertices.push_back(vertex);
          }
        }

        //int* vertices = mesh->GetPolygonVertices();
        // TODO: hardcoded parsing of quads into triangles
        for (int indexIdx = 0; indexIdx < mesh->GetPolygonVertexCount(); indexIdx += 4)
        {
          {
            m_Indices.push_back(indexIdx + 0);
            m_Indices.push_back(indexIdx + 1);
            m_Indices.push_back(indexIdx + 2);
            Vertex& v1 = m_Vertices[indexIdx + 0];
            Vertex& v2 = m_Vertices[indexIdx + 1];
            Vertex& v3 = m_Vertices[indexIdx + 2];
            glm::vec3 edge1 = v2.m_Position - v1.m_Position;
            glm::vec3 edge2 = v3.m_Position - v1.m_Position;
            glm::vec2 deltaUV1 = v2.m_TexCoord - v1.m_TexCoord;
            glm::vec2 deltaUV2 = v3.m_TexCoord - v1.m_TexCoord;

            float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

            glm::vec3 normal, tangent, bitangent;

            normal = { 0, 0, 1 };

            tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
            tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
            tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

            bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
            bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
            bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

            tangent = glm::normalize(tangent);
            bitangent = glm::normalize(bitangent);
            normal = glm::normalize(glm::cross(tangent, bitangent));

            v1.m_Normal = normal;
            v1.m_Tangent = tangent;
            v1.m_Bitangent = bitangent;
            v2.m_Normal = normal;
            v2.m_Tangent = tangent;
            v2.m_Bitangent = bitangent;
            v3.m_Normal = normal;
            v3.m_Tangent = tangent;
            v3.m_Bitangent = bitangent;
          }
          {
            m_Indices.push_back(indexIdx + 2);
            m_Indices.push_back(indexIdx + 3);
            m_Indices.push_back(indexIdx + 0);

            Vertex& v1 = m_Vertices[indexIdx + 2];
            Vertex& v2 = m_Vertices[indexIdx + 3];
            Vertex& v3 = m_Vertices[indexIdx + 0];
            glm::vec3 edge1 = v2.m_Position - v1.m_Position;
            glm::vec3 edge2 = v3.m_Position - v1.m_Position;
            glm::vec2 deltaUV1 = v2.m_TexCoord - v1.m_TexCoord;
            glm::vec2 deltaUV2 = v3.m_TexCoord - v1.m_TexCoord;

            float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

            glm::vec3 normal, tangent, bitangent;

            normal = { 0, 0, 1 };

            tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
            tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
            tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

            bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
            bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
            bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

            tangent = glm::normalize(tangent);
            bitangent = glm::normalize(bitangent);
            normal = glm::normalize(glm::cross(tangent, bitangent));

            v1.m_Normal = normal;
            v1.m_Tangent = tangent;
            v1.m_Bitangent = bitangent;
            v2.m_Normal = normal;
            v2.m_Tangent = tangent;
            v2.m_Bitangent = bitangent;
            v3.m_Normal = normal;
            v3.m_Tangent = tangent;
            v3.m_Bitangent = bitangent;
          }
          
        }
      }
    }
  }
}

void MeshRenderable::Init()
{
  CreateDescriptorSetLayout();
  CreateGraphicsPipeline();
  CreateTextureImage();
  CreateTextureImageView();
  CreateTextureSampler();
  LoadModelFBX();
  CreateVertexBuffer();
  CreateIndexBuffer();
  CreateUniformBuffer();
  CreateDescriptorPool();
  CreateDescriptorSets();
}

void MeshRenderable::Update(RenderContext& ctx)
{
  UpdateUniformBuffer();
}

void MeshRenderable::Render(RenderContext& ctx)
{
  VkCommandBuffer commandBuffer = ctx.m_CommandBuffer;
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

  VkBuffer vertexBuffers[] = { m_VertexBuffer.GetBuffer() };
  VkDeviceSize offsets[] = { 0 };
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

  vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer.GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSets[ctx.m_CommandBufferIdx], 0, nullptr);

  vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);

}

void MeshRenderable::OnCleanupSwapChain()
{
  vkDestroyPipeline(gRenderer.GetDevice(), m_GraphicsPipeline, nullptr);
  vkDestroyPipelineLayout(gRenderer.GetDevice(), m_PipelineLayout, nullptr);
}

void MeshRenderable::Cleanup()
{
  m_UniformBuffer.Cleanup();

  // TODO: duplication OnCleanupSwapChain()
  vkDestroyPipeline(gRenderer.GetDevice(), m_GraphicsPipeline, nullptr);
  vkDestroyPipelineLayout(gRenderer.GetDevice(), m_PipelineLayout, nullptr);
  
  vkDestroySampler(gRenderer.GetDevice(), m_TextureSampler, nullptr);
  vkDestroyImageView(gRenderer.GetDevice(), m_TextureImageView, nullptr);
  vkDestroyImage(gRenderer.GetDevice(), m_TextureImage, nullptr);
  vkFreeMemory(gRenderer.GetDevice(), m_TextureImageMemory, nullptr);

  vkDestroySampler(gRenderer.GetDevice(), m_NormalSampler, nullptr);
  vkDestroyImageView(gRenderer.GetDevice(), m_NormalImageView, nullptr);
  vkDestroyImage(gRenderer.GetDevice(), m_NormalImage, nullptr);
  vkFreeMemory(gRenderer.GetDevice(), m_NormalImageMemory, nullptr);
  
  vkDestroyDescriptorPool(gRenderer.GetDevice(), m_DescriptorPool, nullptr);

  vkDestroyDescriptorSetLayout(gRenderer.GetDevice(), m_DescriptorSetLayout, nullptr);
  m_IndexBuffer.Cleanup();
  m_VertexBuffer.Cleanup();
}
