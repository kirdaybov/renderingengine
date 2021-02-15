#include "paths.h"
#include "logger/logger.h"

#include <Windows.h>
#include <string>

FilePath::FilePath(const char* path)
{
  strcpy_s(m_Path, path);
}

Paths::Paths()
{
  GetModuleFileName(nullptr, ms_ModulePath.m_Path, FilePath::MaxSize);
  std::string binPath = ms_ModulePath.m_Path;
  std::size_t lastIndex = binPath.find_last_of("\\");
  binPath = binPath.substr(0, lastIndex);
  std::string path = binPath + std::string("\\..\\content");
  strcpy_s(ms_ContentPath.m_Path, path.c_str());
  path = binPath + std::string("\\..\\intermediate\\shaders");
  strcpy_s(ms_CompiledShaderPath.m_Path, path.c_str());
  path = binPath + std::string("\\..\\shaders");
  strcpy_s(ms_ShaderCodePath.m_Path, path.c_str());
}

FilePath Paths::ms_ModulePath;
FilePath Paths::ms_ContentPath;
FilePath Paths::ms_CompiledShaderPath;
FilePath Paths::ms_ShaderCodePath;

Paths ms_Paths;