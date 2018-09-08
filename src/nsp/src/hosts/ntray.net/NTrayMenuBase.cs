using System;
using System.Windows.Forms;

namespace NTray_NET
{
	public class NTrayMenuBase
	{
		string Title = "NSP C# Script Host";

		public static void Main()
		{
		}

		public NTrayMenuBase()
		{
			//MessageBox.Show("NTrayMenuBase() constructor", Title);
			SysTrayApp.trayMenu.MenuItems.Clear();
		}

		public void SetMenu()
		{
			SysTrayApp.trayMenu = new ContextMenu(new MenuItem[] {
				new MenuItem("Test Script 1", Test1),
				new MenuItem("submenu", new MenuItem[] {
					new MenuItem("Test Script 2", Test1)
				}),
				new MenuItem("a", SysTrayApp.Exit)
			});
		}

		private void Test1(object sender, EventArgs e)
		{
			MessageBox.Show("NTrayMenuBase.Test1()", Title);
		}
	}
}
