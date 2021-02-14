#pragma once

// usage Paths::GetContentPath() returns C:/work/tbs/content

struct FilePath
{
  FilePath() {}
  FilePath(const char* path);
  enum { MaxSize = 256 };
  //FilePath(const char* path) 
  //  :m_Path(path)
  //{}
  const char* operator*() const { return m_Path; }

private:
  char m_Path[MaxSize];
  friend class Paths;
};

struct Paths
{
  Paths();
  static FilePath GetModulePath() { return ms_ModulePath; }
  static FilePath GetContentPath() { return ms_ContentPath; }
  static FilePath GetCompiledShaderPath() { return ms_CompiledShaderPath; }
  static FilePath GetShaderCodePath() { return ms_ShaderCodePath; }
private:
  static FilePath ms_ContentPath;
  static FilePath ms_ModulePath;
  static FilePath ms_CompiledShaderPath;
  static FilePath ms_ShaderCodePath;
};

