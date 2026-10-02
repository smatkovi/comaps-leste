#include "drape/gl_extensions_list.hpp"
#include "drape/gl_functions.hpp"

#include "base/assert.hpp"

#include "std/target_os.hpp"

namespace dp
{
void GLExtensionsList::Init(dp::ApiVersion apiVersion)
{
  if (apiVersion == dp::ApiVersion::OpenGLES2)
  {
    CheckExtension(VertexArrayObject, "GL_OES_vertex_array_object");
    // OpenGL ES2 has no glMapBufferRange, and mapping a whole buffer with GL_OES_mapbuffer stalls
    // the pipeline on old mobile drivers, so buffers are updated with glBufferSubData.
    SetExtension(MapBuffer, false);
    // Lets tile-based GPUs (PowerVR SGX, Mali-400, Adreno 2xx) skip writing depth/stencil back to memory.
    CheckExtension(DiscardFramebuffer, "GL_EXT_discard_framebuffer");
    return;
  }

  SetExtension(VertexArrayObject, true);
  // Framebuffer discarding is used only on the OpenGL ES2 path.
  SetExtension(DiscardFramebuffer, false);
#ifdef OMIM_OS_ANDROID
  // NOTE: MapBuffer/MapBufferRange are disabled by performance reasons according to
  // https://github.com/organicmaps/organicmaps/commit/d72ab7c8cd8be0eb5a622d9d33ae943b391d5707
  SetExtension(MapBuffer, false);
#else
  SetExtension(MapBuffer, true);
#endif
}

bool GLExtensionsList::IsSupported(ExtensionName extName) const
{
  auto const it = m_supportedMap.find(extName);
  if (it != m_supportedMap.end())
    return it->second;

  ASSERT(false, ("Not all used extensions are checked"));
  return false;
}

void GLExtensionsList::CheckExtension(ExtensionName enumName, std::string const & extName)
{
  m_supportedMap[enumName] = GLFunctions::glHasExtension(extName);
}

void GLExtensionsList::SetExtension(ExtensionName enumName, bool isSupported)
{
  m_supportedMap[enumName] = isSupported;
}
}  // namespace dp
