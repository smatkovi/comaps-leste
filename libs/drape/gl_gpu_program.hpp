#pragma once

#include "drape/gl_constants.hpp"
#include "drape/gpu_program.hpp"
#include "drape/pointers.hpp"
#include "drape/shader.hpp"

#include <array>
#include <cstdint>
#include <string>

#include "3party/ankerl/unordered_dense.h"

namespace dp
{
class GLGpuProgram : public GpuProgram
{
public:
  GLGpuProgram(std::string const & programName, ref_ptr<Shader> vertexShader, ref_ptr<Shader> fragmentShader);
  ~GLGpuProgram() override;

  void Bind() override;
  void Unbind() override;

  int8_t GetAttributeLocation(std::string const & attributeName) const;
  int8_t GetUniformLocation(std::string const & uniformName) const;
  glConst GetUniformType(std::string const & uniformName) const;

  struct UniformInfo
  {
    int8_t m_location = -1;
    glConst m_type = gl_const::GLFloatType;

    // Uniform values are a part of the program object state, so an unchanged value
    // doesn't need to be uploaded again. Returns false if |value| equals the last uploaded one.
    bool UpdateCachedValue(void const * value, size_t size) const;

  private:
    mutable std::array<uint8_t, 64> m_cachedValue = {};  // Big enough for mat4.
    mutable uint8_t m_cachedValueSize = 0;
  };

  UniformInfo const * FindUniform(std::string const & uniformName) const;

  using UniformsInfo = ankerl::unordered_dense::map<std::string, UniformInfo>;
  UniformsInfo const & GetUniformsInfo() const;
  uint32_t GetNumericUniformsCount() const { return m_numericUniformsCount; }

private:
  void LoadUniformLocations();
  uint32_t CalculateNumericUniformsCount() const;

  uint32_t m_programID;

  ref_ptr<Shader> m_vertexShader;
  ref_ptr<Shader> m_fragmentShader;

  UniformsInfo m_uniforms;
  uint8_t m_textureSlotsCount = 0;
  uint32_t m_numericUniformsCount = 0;
};
}  // namespace dp
