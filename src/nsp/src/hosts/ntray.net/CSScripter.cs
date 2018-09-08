using System;
using System.CodeDom.Compiler;
using System.Reflection;
using System.Windows.Forms;

namespace NTray_NET
{
	public class CSScripter
	{
		public CSScripter()
		{
		}

		public void ScriptLoad()
		{
			try
			{
				CompilerParameters compilerparams = new CompilerParameters();
				compilerparams.ReferencedAssemblies.Add(Application.ExecutablePath);
				compilerparams.ReferencedAssemblies.Add("System.dll");
				compilerparams.ReferencedAssemblies.Add("System.Windows.Forms.dll");
				compilerparams.GenerateInMemory = true;
				compilerparams.GenerateExecutable = true;
				CodeDomProvider provider = new Microsoft.CSharp.CSharpCodeProvider();
				CompilerResults results = provider.CompileAssemblyFromFile(compilerparams, System.IO.Path.Combine(Application.StartupPath, "NTray.NET.cs"));
				if (results.Errors.HasErrors)
				{
					string s = "";
					for (int i = 0; i < 10 && i < results.Errors.Count; i++)
					{
						if (i > 0) s += "\r\n";
						s += string.Format("Line {0}: {1}", results.Errors[i].Line, results.Errors[i].ErrorText);
					}
					throw new Exception(s);
				}
				Assembly generatedAssembly = results.CompiledAssembly;
				//generatedAssembly.EntryPoint.Invoke(null, BindingFlags.Static, null, new object[] { new string[] { "a", "b" } }, null);
				//results.CompiledAssembly.EntryPoint.Invoke(null, BindingFlags.Static, null, new object[] { new string[] { "a", "b" } }, null);
				Type type = generatedAssembly.GetType("NTrayMenu");
				if (type != null)
				{
					object obj = Activator.CreateInstance(type);
					MethodInfo mi = type.GetMethod("SetMenu");
					mi.Invoke(obj, new object[] { });
					//type.GetMethod("Test1").Invoke(obj, new object[] { });
					//type.GetMethod("Test2").Invoke(obj, new object[] { new string[] { "a", "b" } });
					//type.GetMethod("Test3").Invoke(obj, new object[] { });
				}
				else
				{
					Console.WriteLine("ScriptLoad() warning, class not found");
				}
			}
			catch (Exception ex)
			{
				MessageBox.Show(ex.Message, "ScriptLoad() assembly exception", MessageBoxButtons.OK, MessageBoxIcon.Error);
			}
			SysTrayApp.trayMenu.MenuItems.Add("Exit", SysTrayApp.Exit);
		}
	}
}
