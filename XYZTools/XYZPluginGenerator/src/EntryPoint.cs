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


        static void Main(string[] args)
        {
            //Debug.Assert(args.Length == 1);
            //
            //string codeToCompile = File.ReadAllText(args[0]);

            Assembly assembly = CreateAssemblyFromScript("C:\\Users\\Gido\\Projects\\XYZEngine\\XYZPlugin\\XYZPlugin.cs", out var compileErrors);
            foreach (var error in compileErrors)
            {
                Console.WriteLine(error);
                File.AppendAllText(logFile, error.ToString());
            }
            if (compileErrors.Count > 0) { return; }

  
            var type = assembly.GetTypes()[0]; 
            var project = Activator.CreateInstance(type);

            ProjectInfo projectInfo = new ProjectInfo(project, type);
            var validationErrors = projectInfo.Validate();
            foreach (var error in validationErrors)
            {
                Console.WriteLine(error);
                File.AppendAllText(logFile, error);
            }
            if (validationErrors.Count > 0) { return; }


            string premake = Premake.GeneratePremake(projectInfo, type.Name);
            File.WriteAllText("premake5.lua", premake);
        }
    }
}
