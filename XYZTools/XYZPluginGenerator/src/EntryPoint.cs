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

namespace XYZPluginGenerator
{

    class EntryPoint
    {
        static string logFile = "PluginGeneratorLog.log";

        static string buildFileEnd = ".Build.cs";

        static Assembly CreateAssemblyFromScript(string scriptPath, out List<CompilerError> errors)
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


        static string FindBuildFile(string path)
        {
            var files = Directory.EnumerateFiles(path);
            foreach (var file in files)
            {
                if (file.EndsWith(buildFileEnd))
                    return file;
            }
            return null;
        }


        static void Main(string[] args)
        {
            try
            {
                string projectDirectory = args[0];
                string engineDirectory = args[1];

                Console.WriteLine("Generating " + projectDirectory);
                Console.WriteLine("Engine directory " + engineDirectory);

                string projectBuildFile = FindBuildFile(projectDirectory);
                if (projectBuildFile == null)
                {
                    Console.WriteLine(projectDirectory + " does not contain build file");
                    Console.ReadKey();
                    return;
                }


                Assembly assembly = CreateAssemblyFromScript(projectBuildFile, out var compileErrors);
                foreach (var error in compileErrors)
                {
                    Console.WriteLine(error);
                    File.AppendAllText(logFile, error.ToString());
                }
                if (compileErrors.Count > 0)
                {
                    Console.ReadKey();
                    return;
                }


                var type = assembly.GetTypes()[0];
                var project = Activator.CreateInstance(type, engineDirectory);

                ProjectInfo projectInfo = new ProjectInfo(project, type);
                var validationErrors = projectInfo.Validate();
                foreach (var error in validationErrors)
                {
                    Console.WriteLine(error);
                    File.AppendAllText(logFile, error);
                }
                if (validationErrors.Count > 0)
                {
                    Console.ReadKey();
                    return;
                }


                string premake = Premake.GeneratePremake(projectInfo, type.Name);
                File.WriteAllText(projectDirectory + "\\premake5.lua", premake);

                Console.WriteLine("Successfully generated " + projectBuildFile);


                Console.ReadKey();
            }
            catch(Exception ex)
            {
                Console.WriteLine(ex.Message); 
                Console.ReadKey();

            }
        }
    }
}
