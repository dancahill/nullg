//https://alanbondo.wordpress.com/2008/06/22/creating-a-system-tray-app-with-c/
//https://social.msdn.microsoft.com/Forums/vstudio/en-US/0913ae1a-7efc-4d7f-a7f7-58f112c69f66/c-application-system-tray-icon?forum=csharpgeneral

using System;
using System.Drawing;
using System.Windows.Forms;

namespace NTray_NET
{
	public class SysTrayApp : Form
	{
		[STAThread]
		public static void Main()
		{
			Application.Run(new SysTrayApp());
		}

		private NotifyIcon trayIcon;
		public static ContextMenu trayMenu;

		public SysTrayApp()
		{
			try
			{
				trayMenu = new ContextMenu();
				trayIcon = new NotifyIcon();
				trayIcon.Text = "NTray.NET\nPowered by wishful thinking";
				trayIcon.Icon = new Icon(System.Reflection.Assembly.GetExecutingAssembly().GetManifestResourceStream("NTray.NET.NTray.NET.ico"));
				trayIcon.ContextMenu = trayMenu;
				trayIcon.Visible = true;
				trayIcon.MouseDoubleClick += TrayIcon_MouseDoubleClick;
				CSScripter c = new CSScripter();
				c.ScriptLoad();

				trayIcon.ContextMenu = trayMenu;
				trayMenu.Popup += TrayMenu_Popup;
				trayMenu.Collapse += TrayMenu_Collapse;
			}
			catch (Exception ex)
			{
				MessageBox.Show(ex.Message);
				Application.Exit();
				this.Close();
			}
		}

		private void TrayMenu_Collapse(object sender, EventArgs e)
		{
			//throw new NotImplementedException();
			Console.WriteLine("TrayMenu_Collapse");
		}

		private void TrayMenu_Popup(object sender, EventArgs e)
		{
			//throw new NotImplementedException();
			Console.WriteLine("TrayMenu_Popup");
		}

		private void TrayIcon_MouseDoubleClick(object sender, MouseEventArgs e)
		{
			trayIcon.ShowBalloonTip(10, "stuff", "stop clicking on me!", ToolTipIcon.Info);
		}

		protected override void OnLoad(EventArgs e)
		{
			Visible = false;
			ShowInTaskbar = false;
			base.OnLoad(e);
		}

		static public void Exit(object sender, EventArgs e)
		{
			Application.Exit();
		}

		//private void OnExit(object sender, EventArgs e)
		//{
		//	Application.Exit();
		//}

		protected override void Dispose(bool isDisposing)
		{
			if (isDisposing)
			{
				trayIcon.Dispose();
			}
			base.Dispose(isDisposing);
		}
	}
}
