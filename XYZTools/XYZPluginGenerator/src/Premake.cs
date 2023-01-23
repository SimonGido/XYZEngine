using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace XYZPluginGenerator
{
    internal class Premake
    {
        private static string cppDialect = "C++17";

        private static string outputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}";

        private static string targetDir = "bin";

        private static string objDir = "bin-int";

        private static string engineSourceDirectory = "../../../XYZEngine";

        private static string engineLibDirectory = "../XYZEngine";

        private static string engineScriptLibDirectory = "../XYZScriptCore";

        private static string engineSourceFullDirectory;

        private static string engineLibFullDirectory;

        private static string engineScriptLibFullDirectory;


        private static Dictionary<string, string> modules;

        static Premake()
        {
            engineSourceFullDirectory = Path.GetFullPath(engineSourceDirectory);
            engineLibFullDirectory = Path.GetFullPath(engineLibDirectory);
            engineScriptLibFullDirectory = Path.GetFullPath(engineScriptLibDirectory);

            engineSourceFullDirectory = engineSourceFullDirectory.Replace('\\', '/');
            engineLibFullDirectory = engineLibFullDirectory.Replace("\\", "/");
            engineScriptLibFullDirectory = engineScriptLibFullDirectory.Replace("\\", "/");

            RegisterModules();
        }
        
        
        public static string GeneratePremake(ProjectInfo info, string projectName)
        {
            StringBuilder builder = new StringBuilder();
            builder.AppendLine(GenerateWorkSpace(projectName));
            builder.AppendLine(GenerateProjectSetup(info, projectName));
            builder.AppendLine(GenerateIncludedFiles());
            builder.AppendLine(GenerateIncludedDirs(info));
            builder.AppendLine(GenerateLinks(info));

            return builder.ToString();
        }

        private static void RegisterModules()
        {
            modules = new Dictionary<string, string>();
            modules.Add("spdlog", engineSourceFullDirectory + "/vendor/spdlog/include");
            modules.Add("ImGui", engineSourceFullDirectory + "/vendor/imgui");
            modules.Add("ImGuizmo", engineSourceFullDirectory + "/vendor/ImGuizmo");
            modules.Add("ImGuiNode", engineSourceFullDirectory + "/vendor/imgui-node-editor");
            modules.Add("glm", engineSourceFullDirectory + "/vendor/glm");
            modules.Add("ozz_animation", engineSourceFullDirectory + "/vendor/ozz-animation/include");
            modules.Add("box2d", engineSourceFullDirectory + "/vendor/box2d/include");
            modules.Add("entt", engineSourceFullDirectory + "/vendor/entt/include");
            modules.Add("OpenXR", engineSourceFullDirectory + "/vendor/OpenXR-SDK/include");
            modules.Add("optick", engineSourceFullDirectory + "/vendor/optick/src");
            modules.Add("yaml", engineSourceFullDirectory + "/vendor/yaml-cpp/include");
            modules.Add("Asio", engineSourceFullDirectory + "/vendor/asio/include");
        }

        private static string GenerateWorkSpace(string projectName)
        {
            StringBuilder builder = new StringBuilder();
            builder.AppendLine(KeyValue("workspace", projectName));
            AddList(builder, "configurations", "Debug", "Release");
            builder.AppendLine(KeyValue("architecture", "x64"));

            return builder.ToString();
        }

        private static string GenerateProjectSetup(ProjectInfo info, string projectName)
        {
            StringBuilder builder = new StringBuilder();
            builder.AppendLine(KeyValue("project", projectName));
            builder.AppendLine(KeyValue("\tkind", info.ProjectKind));
            builder.AppendLine(KeyValue("\tlanguage", info.Language));

            if (info.Language == "C++")
            {
                builder.AppendLine(KeyValue("\tcppdialect", cppDialect));
                builder.AppendLine(KeyValue("\tstaticruntime", "off"));
            }

            builder.AppendLine(String.Format("\ttargetdir(\"{0}\")", targetDir + "/" + outputDir));
            builder.AppendLine(String.Format("\tobjdir(\"{0}\")", objDir + "/" + outputDir));

            return builder.ToString();
        }

        private static string GenerateIncludedFiles()
        {
            StringBuilder builder = new StringBuilder();


            AddList(builder,
                "files",
                "**.h",
                "**.cpp",
                "**.cs"
            );
     
            return builder.ToString();
        }

        private static string GenerateIncludedDirs(ProjectInfo info)
        {
            StringBuilder builder = new StringBuilder();

            List<string> includedDirs = new List<string>();
            includedDirs.Add(engineSourceFullDirectory + "/vendor");
            includedDirs.Add(engineSourceFullDirectory + "/src");
            foreach (var module in modules)
            {
                includedDirs.Add(module.Value);
            }

            AddList(builder, "includedirs", includedDirs.ToArray());

            // TODO: include modules

            return builder.ToString();
        }

        private static string GenerateLinks(ProjectInfo info)
        {
            StringBuilder builder = new StringBuilder();

            if (info.Language == "C++")
            {
                AddList(builder, 
                    "links",
                    engineLibFullDirectory + "/XYZEngine.lib"
                    );

            }
            else if (info.Language == "C#")
            {
                AddList(builder, 
                    "links",
                    engineScriptLibFullDirectory + "/XYZScriptCore.dll"
                    );
            }
            return builder.ToString();
        }

        private static void AddList(StringBuilder builder, string name, params string[] strings)
        {
            builder.AppendLine(name);
            builder.AppendLine("{");
            for (int i = 0; i < strings.Length - 1; i++)
            {
                builder.Append(String.Format("\"{0}\"", strings[i]));
                builder.AppendLine(",");
            }
            builder.AppendLine(String.Format("\"{0}\"", strings.Last()));
            builder.AppendLine("}");
        }

        private static string KeyValue(string key, string value)
        {
            return String.Format("{0} \"{1}\"", key, value);
        }
    }
}
