using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

namespace NSPEdit
{
	static class Program
	{
		static public NSPEditForm MainForm;
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main(string[] args)
		{
			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);
			MainForm = new NSPEditForm();
			if (args.Length > 0)
			{
				bool getfilename = false;
				foreach (string arg in args)
				{
					if (getfilename)
					{
						getfilename = false;
						MainForm.loadfile = arg;
					}
					if (arg == "-f")
					{
						getfilename = true;
						continue;
					}
					else if (!arg.StartsWith("-") && MainForm.loadfile == "")
					{
						MainForm.loadfile = arg;
					}
				}
			}
			Application.Run(MainForm);
		}
	}
}
