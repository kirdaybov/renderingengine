#pragma once
#include "assetmanager/foldercrawler.h"
#include "paths/paths.h"
#include "serialization/serialization.h"
#include "shaderbinary.h"
#include <map>
#include "uid/uid.h"
#include "utils/utils.h"

class ShaderCompiler
{
public:
  ~ShaderCompiler()
  {
    Cleanup();    
  }
  void Init();
  void Update();
  void AddShader(UID uid, ShaderBinary* shaderBinary);
  ShaderBinary* GetShader(UID uid)
  {
    auto it = m_Shaders.find(uid);
    if (it != m_Shaders.end())
    {
      return it->second;
    }
    return nullptr;
  }
private:
  void Cleanup();
  std::map<UID, ShaderBinary*> m_Shaders;
};

class ShaderFolderCrawler : public FolderCrawler
{
public:
  ShaderFolderCrawler(ShaderCompiler* shaderCompiler) : m_ShaderCompiler(shaderCompiler) {}
private:
  ShaderCompiler* m_ShaderCompiler;
  void ProcessFile(std::string name, int level);
};

class ShaderSourceFolderCrawler : public FolderCrawler
{
public:
  ShaderSourceFolderCrawler(ShaderCompiler* shaderCompiler) : m_ShaderCompiler(shaderCompiler) {}
  bool IsShaderUpdateRequired() { return m_UpdateShaders; }
private:
  bool m_UpdateShaders = false;
  ShaderCompiler* m_ShaderCompiler;
  void ProcessFile(std::string name, int level);
};