#include "shadercompiler.h"
#include <filesystem>
#include "utils/utils.h"
#include "logger/logger.h"

void ShaderCompiler::AddShader(UID uid, ShaderBinary* shaderBinary)
{
  m_Shaders.insert(std::make_pair(uid, shaderBinary));
}

void ShaderFolderCrawler::ProcessFile(std::string name, int level)
{
  ShaderBinary* shader = new ShaderBinary();
  Serialization::Serialize(name, shader, true);
  m_ShaderCompiler->AddShader(Utils::StripPathFromName(name).data(), shader);
}

void ShaderSourceFolderCrawler::ProcessFile(std::string name, int level)
{
  if (Utils::GetFileExtension(name) != "hlsl")
  {
    return;
  }
  auto fileName = Utils::StripPathFromName(name);
  UID uid = fileName.data();
  if (ShaderBinary* shader = m_ShaderCompiler->GetShader(uid))
  {
    auto lastWriteTime = std::filesystem::last_write_time(name);
    int lastWriteTimeInt = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(lastWriteTime.time_since_epoch()).count());
    if (shader->IsOld(lastWriteTimeInt))
    {
      shader->Compile(name);
    }
  }
  else
  {
    ShaderBinary* newShader = new ShaderBinary();
    newShader->Compile(name);
    m_ShaderCompiler->AddShader(uid, newShader);
  }
}

void ShaderCompiler::Init()
{
  ShaderFolderCrawler shaderFolderCrawler(this);
  shaderFolderCrawler.Crawl(*Paths::GetCompiledShaderPath());

  ShaderSourceFolderCrawler shaderSourceCrawler(this);
  shaderSourceCrawler.Crawl(*Paths::GetShaderCodePath());
}