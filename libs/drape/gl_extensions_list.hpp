#pragma once

#include "drape/drape_global.hpp"

#include "base/macros.hpp"

#include <map>
#include <string>

namespace dp
{
class GLExtensionsList
{
public:
  enum ExtensionName
  {
    VertexArrayObject,
    MapBuffer,
    DiscardFramebuffer,
  };

  GLExtensionsList() = default;
  void Init(dp::ApiVersion apiVersion);
  bool IsSupported(ExtensionName extName) const;
  void SetExtension(ExtensionName enumName, bool isSupported);

private:
  void CheckExtension(ExtensionName enumName, std::string const & extName);

  std::map<ExtensionName, bool> m_supportedMap;

  DISALLOW_COPY_AND_MOVE(GLExtensionsList);
};
}  // namespace dp
