#pragma once

#include "foldercrawler.h"
#include "uid/uid.h"
#include "paths/paths.h"

#include <map>
#include <string>

struct Asset
{
  byte* m_Data;
  int m_Size;
  FilePath m_Path;
  char m_Extension[8];
};

class AssetManager;
class AssetInitFolderCrawler : public FolderCrawler
{
public:
  AssetInitFolderCrawler(AssetManager* assetManager) : m_AssetManager(assetManager) {}
  
  void ProcessFile(std::string name, int level) override;

private:
  AssetManager* m_AssetManager;
};  

class AssetManager
{
public:
  Asset* GetAsset(UID uid);
  void LoadAssets();

  inline static AssetManager& GetInstance() { return ms_AssetManager; }
private:
  std::map<UID, Asset*> m_Assets;
  static AssetManager ms_AssetManager;
  
  friend class AssetInitFolderCrawler;
};

#define gAssetManager AssetManager::GetInstance()