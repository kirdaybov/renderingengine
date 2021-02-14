#include "shaderbinary.h"
#include <winerror.h>
#include <combaseapi.h>
#include "dxcapi.h"
#include "logger/logger.h"
#include <vector>
#include <chrono>
#include <filesystem>
#include "paths/paths.h"
#include "utils/utils.h"

enum class ShaderType
{
  Vertex,
  Pixel,
  Compute,
  Max
};

// will parse myshaderXX.hlsl where XX is ps, vs, cs, example meshvs.hlsl
ShaderType GetShaderType(const std::string& name)
{
  int lastDot = name.find_last_of(".");
  const std::string shaderType = name.substr(lastDot - 2, 2);
  if (shaderType == "vs")
  {
    return ShaderType::Vertex;
  }
  else if (shaderType == "ps")
  {
    return ShaderType::Pixel;
  }
  else if (shaderType == "cs")
  {
    return ShaderType::Compute;
  }
  DEBUG_BREAK("Uknown shader type");
  return ShaderType::Max;
}

const wchar_t* GetShaderProfile(ShaderType type)
{
  static const wchar_t* types[] =
  {
    L"vs_6_5",
    L"ps_6_5",
    L"cs_6_5"
  };
  return types[(int)type];
}

void ShaderBinary::Compile(const std::string& name)
{
  IDxcLibrary* library;
  if (FAILED(DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library))))
  {
    DEBUG_BREAK("Couldn't create dxc library");
  }
  IDxcCompiler3* compiler;
  if (FAILED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler))))
  {
    DEBUG_BREAK("Couldn't create dxc compiler");
  }

  std::ifstream file(name, std::ios::binary | std::ios::ate);
  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<char> buffer(size);

  if (file.read(buffer.data(), size))
  {
    ShaderType type = GetShaderType(name);

    std::vector<LPCWSTR> args;
    args.push_back(L"-spirv");
    args.push_back(L"-T");
    args.push_back(GetShaderProfile(type));
    
    DxcBuffer sourceBuffer;
    sourceBuffer.Ptr = buffer.data();
    sourceBuffer.Size = size;
    sourceBuffer.Encoding = 0;
    IDxcResult* result;
    compiler->Compile(&sourceBuffer, args.data(), args.size(), nullptr, IID_PPV_ARGS(&result));

    HRESULT hrCompilation;
    result->GetStatus(&hrCompilation);

    if (hrCompilation < 0)
    {
      IDxcBlobEncoding* printBlob;
      result->GetErrorBuffer(&printBlob);
      DEBUG_BREAK("%s: %s", name.data(), printBlob->GetBufferPointer());
    }
    else
    {
      IDxcBlob* code;
      result->GetResult(&code);
      m_BinarySize = code->GetBufferSize();
      m_Binary = static_cast<char*>(code->GetBufferPointer());
      auto lastWriteTime = std::filesystem::last_write_time(name);
      m_Date = std::chrono::duration_cast<std::chrono::seconds>(lastWriteTime.time_since_epoch()).count();
      auto fileName = std::string("\\") + Utils::StripPathFromName(name) + ".spirv";
      Serialization::Serialize(*Paths::GetCompiledShaderPath() + fileName, this, false);
      LOGF("%s: successfuly compiled", name.data());
    }
  }
}
