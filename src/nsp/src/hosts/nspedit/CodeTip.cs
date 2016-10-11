using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace NSPEdit
{
	public class CodeTip: ToolTip
	{
		public string FormatToolTip(string name, Color color)
		{
			XmlHelp.XmlHelpEntry xhelp = XmlHelp.findnode(name);
			this.ToolTipTitle = "";
			string t = name;
			if (xhelp != null)
			{
				this.ToolTipTitle = string.Format("({0}) {1}", xhelp.type, xhelp.name);
				t = "";
				if (xhelp.desc != "") t = string.Format("{0}", xhelp.desc);
				if (xhelp.parameters != "" || xhelp.returns != "")
				{
					if (xhelp.desc != "") t += "\r\n";
					if (xhelp.parameters != "") t += string.Format("\r\nParameters: {0}", xhelp.parameters);
					if (xhelp.returns != "") t += string.Format("\r\nReturns: {0}", xhelp.returns);
				}
				if (xhelp.type == "table")
				{
					List<XmlHelp.XmlHelpEntry> entries = XmlHelp.getlist(name);
					if (entries.Count > 0)
					{
						if (xhelp.desc != "") t += "\r\n";
						t += "\r\nMembers:";
						foreach (XmlHelp.XmlHelpEntry entry in entries)
						{
							t += string.Format("\r\n    ({0}){1}", entry.type, entry.name);
						}
					}

				}
				//tt.SetToolTip(, t);
			}
			if (color != Color.White)
			{
				if (color != Color.DarkCyan) this.ToolTipTitle = "";
				if (color == Color.Green) t = ""; // comment
				else if (color == Color.Red) t = ""; // misc extra punctuation
				else if (color == Color.Black) t = "";//keyword
				else if (color == Color.DarkCyan) t += "";// string.Format("\r\nColor: {0}", color);
				else if (color == Color.Maroon) t = ""; // punctuation
				else if (color == Color.Blue && name != "true" && name != "false" && name != "null" && name != "this") { t = "string data"; }
				else if (color == Color.Navy) t = "numeric data";
			}
			return t;
		}
	}
}
