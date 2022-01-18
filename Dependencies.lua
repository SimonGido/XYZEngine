VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/XYZEngine/vendor/glfw/include"
IncludeDir["GLEW"] = "%{wks.location}/XYZEngine/vendor/GLEW/include"
IncludeDir["glm"] = "%{wks.location}/XYZEngine/vendor/glm"
IncludeDir["OpenAL"] = "%{wks.location}/XYZEngine/vendor/OpenAL-Soft"
IncludeDir["MiniMp3"] = "%{wks.location}/XYZEngine/vendor/minimp3"
IncludeDir["FreeType"] = "%{wks.location}/XYZEngine/vendor/freetype-2.10.1"
IncludeDir["Asio"] = "%{wks.location}/XYZEngine/vendor/asio/include"
IncludeDir["mono"] = "%{wks.location}/XYZEngine/vendor/mono/include"
IncludeDir["TrianglePP"] = "%{wks.location}/XYZEngine/vendor/TrianglePP/source"
IncludeDir["yaml"] = "%{wks.location}/XYZEngine/vendor/yaml-cpp/include"
IncludeDir["ImGui"] = "%{wks.location}/XYZEngine/vendor/imgui"
IncludeDir["ImGuizmo"] = "%{wks.location}/XYZEngine/vendor/ImGuizmo"
IncludeDir["box2d"] = "%{wks.location}/XYZEngine/vendor/box2d/include"
IncludeDir["optick"] = "%{wks.location}/XYZEngine/vendor/optick/src"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"

LibraryDir = {}
LibraryDir["mono"] = "vendor/mono/lib/Debug/mono-2.0-sgen.lib"

LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["VulkanSDK_Debug"] = "%{wks.location}/XYZEngine/vendor/VulkanSDK/Lib"
LibraryDir["VulkanSDK_DebugDLL"] = "%{wks.location}/XYZEngine/vendor/VulkanSDK/Bin"


Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"