using System;
using System.IO;
using System.Reflection;
using System.Windows.Forms;
using Microsoft.VisualBasic.ApplicationServices;

namespace NSPEdit
{
	public class SingleInstanceController : WindowsFormsApplicationBase
	{
		public SingleInstanceController()
		{
			IsSingleInstance = true;
			StartupNextInstance += this_StartupNextInstance;
		}

		void this_StartupNextInstance(object sender, StartupNextInstanceEventArgs e)
		{
			Program.MainForm.LoadFile(e.CommandLine[1]);
		}

		protected override void OnCreateMainForm()
		{
			Program.MainForm = new NSPEditForm();
			MainForm = Program.MainForm;
		}
	}

	static class Program
	{
		static public NSPEditForm MainForm;
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main()
		{
			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);
			//Environment.SetEnvironmentVariable("PATH", @"C:\NullLogic\utils\VS2019\lib;" + Environment.GetEnvironmentVariable("PATH"));
			string[] args = Environment.GetCommandLineArgs();
			SingleInstanceController controller = new SingleInstanceController();
			controller.Run(args);
		}

		static public void Log(string format, params object[] list)
		{
			string methname = new System.Diagnostics.StackFrame(1, true).GetMethod().Name;
			string o = string.Format("{0} {1}(): {2}", DateTime.Now, methname, string.Format(format, list));
			if (Program.MainForm != null)
			{
				MainForm.richTextBox2.Text += "\r\n" + o.Substring(o.IndexOf(' ') + 1).Trim();
				MainForm.richTextBox2.SelectionStart = MainForm.richTextBox2.Text.Length;
				MainForm.richTextBox2.ScrollToCaret();
			}
			Application.DoEvents();
		}

		static public string GetResource(string name)
		{
			string fullname = string.Format("NSPEdit.Resources.{0}", name);
			string x = "";
			Assembly a = Assembly.GetExecutingAssembly();
			//foreach (string resource in a.GetManifestResourceNames()) Program.Log("'{0}'", resource);
			using (Stream stream = a.GetManifestResourceStream(fullname))
			using (StreamReader reader = new StreamReader(stream))
			{
				x = reader.ReadToEnd();
			}
			//if (x == "") Program.Log("resource '{0}' not found", fullname);
			return x;
		}
	}
}
