using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


using System.IO;
using System.Diagnostics;
using System.Reflection;
using System.Configuration;
using System.CodeDom.Compiler;

using Microsoft.CSharp;

using YamlDotNet.Serialization;

namespace XYZPluginGenerator
{

    class EntryPoint
    {
        static string logFile = "PluginGeneratorLog.log";

        static string buildFileEnd = ".Build.cs";

        static string pluginExtension = ".xyzplugin";

        private static Assembly CreateAssemblyFromScript(string scriptPath, out List<CompilerError> errors)
        {
            string codeToCompile = File.ReadAllText(scriptPath);

            var csc = new CSharpCodeProvider(new Dictionary<string, string>() { { "CompilerVersion", "v3.5" } });
            var parameters = new CompilerParameters(new[] { "mscorlib.dll", "System.Core.dll" }, "assembly", true);
            parameters.GenerateExecutable = false;
            parameters.GenerateInMemory = true;

            CompilerResults results = csc.CompileAssemblyFromSource(parameters, codeToCompile);
            errors = results.Errors.Cast<CompilerError>().ToList();

            return results.CompiledAssembly;
        }


        private static string FindBuildFile(string path)
        {
            var files = Directory.EnumerateFiles(path);
            foreach (var file in files)
            {
                if (file.EndsWith(buildFileEnd))
                    return file;
            }
            return null;
        }

        private static Assembly CreateAndValidateAssembly(string projectBuildFile)
        {
            Assembly assembly = CreateAssemblyFromScript(projectBuildFile, out var compileErrors);
            foreach (var error in compileErrors)
            {
                Console.WriteLine(error);
                File.AppendAllText(logFile, error.ToString());
            }
            if (compileErrors.Count > 0)
            {
                return null;
            }
            return assembly;
        }

        private static ProjectInfo CreateAndValidateProjectInfo(Assembly assembly, EngineInfo engineInfo)
        {
            var type = assembly.GetTypes()[0];
            var project = Activator.CreateInstance(type, engineInfo.EngineLibDirectory, engineInfo.EngineSourceDirectory);

            ProjectInfo projectInfo = new ProjectInfo(project, type);
            var validationErrors = projectInfo.Validate();
            foreach (var error in validationErrors)
            {
                Console.WriteLine(error);
                File.AppendAllText(logFile, error);
            }
            if (validationErrors.Count > 0)
            {
                return null;
            }
            return projectInfo;
        }

        private static void SerializeProjectInfo(string path, ProjectInfo projectInfo)
        {
            string saveFile = path + "/" + projectInfo.ProjectName + pluginExtension;
            Dictionary<string, string> yamlValues = projectInfo.ToDictionary();
            var writer = new StreamWriter(saveFile);
            Serializer serializer = new Serializer();
            serializer.Serialize(writer, yamlValues);
            writer.Dispose();
        }
        static void Main(string[] args)
        {
            try
            {
                string projectDirectory = args[0];

                EngineInfo engineInfo = new EngineInfo(args[1], args[2]);

                string projectBuildFile = FindBuildFile(projectDirectory);
                if (projectBuildFile == null)
                {
                    Console.WriteLine(projectDirectory + " does not contain build file");
                    return;
                }


                Assembly assembly = CreateAndValidateAssembly(projectBuildFile);
                if (assembly == null) 
                    return;

                ProjectInfo projectInfo = CreateAndValidateProjectInfo(assembly, engineInfo);
                if (projectInfo == null)
                    return;

                string premake = Premake.GeneratePremake(projectInfo, engineInfo);
                File.WriteAllText(projectDirectory + "\\premake5.lua", premake);
                SerializeProjectInfo(projectDirectory, projectInfo);
                Console.WriteLine("Successfully generated " + projectBuildFile);
            }
            catch(Exception ex)
            {
                Console.WriteLine(ex.Message); 
            }
        }
    }
}
