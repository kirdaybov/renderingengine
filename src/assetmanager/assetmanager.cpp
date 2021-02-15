#include "assetmanager.h"

#include "logger/logger.h"
#include "paths/paths.h"

#include <iostream>
#include <fstream>
#include <Windows.h>
#include <string>
#include "utils/utils.h"

AssetManager AssetManager::ms_AssetManager = AssetManager();

Asset* AssetManager::GetAsset(UID uid)
{
  auto it = m_Assets.find(uid);
  if (it != m_Assets.end())
    return it->second;
  {
    LOGF("No such asset!");
    return nullptr;
  }
}

void AssetManager::LoadAssets()
{
  AssetInitFolderCrawler crawler(this);
  LOGF("Asset loading started...");
  crawler.Crawl(*Paths::GetContentPath());
  crawler.Crawl(*Paths::GetCompiledShaderPath());
  LOGF("Asset loading finished");    
}

void AssetInitFolderCrawler::ProcessFile(std::string name, int level)
{
  std::ifstream file(name, std::ios::binary | std::ios::ate);
  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<char> buffer(size);
  if (file.read(buffer.data(), size))
  {
    Asset* asset = new Asset();
    asset->m_Size = static_cast<int>(size);
    asset->m_Data = (byte*)malloc(size);
    memcpy(asset->m_Data, &buffer[0], size);
    
    std::size_t lastDot = name.find_last_of(".");
    strcpy_s(asset->m_Extension, name.substr(lastDot, name.length()).c_str());

    std::string assetName = Utils::StripPathFromName(name);
    asset->m_Path = FilePath(name.c_str());
          
    m_AssetManager->m_Assets.insert(std::make_pair(assetName.c_str(), asset));
    LOGF("Asset of type %s : %s is loaded successfully", asset->m_Extension, assetName.c_str());
  }
}