VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["Assimp"] = "%{wks.location}/XYZEngine/vendor/assimp/include"
IncludeDir["GLFW"] = "%{wks.location}/XYZEngine/vendor/glfw/include"
IncludeDir["GLEW"] = "%{wks.location}/XYZEngine/vendor/GLEW/include"
IncludeDir["glm"] = "%{wks.location}/XYZEngine/vendor/glm"
IncludeDir["OpenAL"] = "%{wks.location}/XYZEngine/vendor/OpenAL-Soft"
IncludeDir["MiniMp3"] = "%{wks.location}/XYZEngine/vendor/minimp3"
IncludeDir["FreeType"] = "%{wks.location}/XYZEngine/vendor/freetype-2.10.1"
IncludeDir["Asio"] = "%{wks.location}/XYZEngine/vendor/asio/include"
IncludeDir["mono"] = "%{wks.location}/XYZEngine/vendor/mono/include"
IncludeDir["yaml"] = "%{wks.location}/XYZEngine/vendor/yaml-cpp/include"
IncludeDir["ImGui"] = "%{wks.location}/XYZEngine/vendor/imgui"
IncludeDir["ImGuiNode"] = "%{wks.location}/XYZEngine/vendor/imgui-node-editor"
IncludeDir["ImGuizmo"] = "%{wks.location}/XYZEngine/vendor/ImGuizmo"
IncludeDir["box2d"] = "%{wks.location}/XYZEngine/vendor/box2d/include"
IncludeDir["optick"] = "%{wks.location}/XYZEngine/vendor/optick/src"
IncludeDir["delaunator"] = "%{wks.location}/XYZEngine/vendor/delaunator/include"
IncludeDir["ozz_animation"] = "%{wks.location}/XYZEngine/vendor/ozz-animation/include"
IncludeDir["entt"]  = "%{wks.location}/XYZEngine/vendor/entt/include"
IncludeDir["cr"]  = "%{wks.location}/XYZEngine/vendor/cr"
IncludeDir["OpenXR"] = "%{wks.location}/XYZEngine/vendor/OpenXR-SDK/include"
IncludeDir["OpenXR-Oculus"] = "%{wks.location}/XYZEngine/vendor/Oculus-OpenXR-Mobile-SDK/OpenXR/Include"
IncludeDir["spdlog"] = "%{wks.location}/XYZEngine/vendor/spdlog/include"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"

LibraryDir = {}
LibraryDir["mono"] = "vendor/mono/lib/Debug/mono-2.0-sgen.lib"

LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["VulkanSDK_Debug"] = "%{VULKAN_SDK}/Lib"
LibraryDir["VulkanSDK_DebugDLL"] = "%{VULKAN_SDK}/Bin"

Library = {}
Library["Assimp_Debug"] = "%{wks.location}/XYZEngine/vendor/assimp/bin/Debug/assimp-vc142-mtd.lib"
Library["Assimp_Release"] = "%{wks.location}/XYZEngine/vendor/assimp/bin/Release/assimp-vc142-mt.lib"
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"

Binaries = {}
Binaries["Assimp_Debug"] = "%{wks.location}XYZEngine/vendor/assimp/bin/Debug/assimp-vc142-mtd.dll"
Binaries["Assimp_Release"] = "%{wks.location}XYZEngine/vendor/assimp/bin/Release/assimp-vc142-mt.dll"
