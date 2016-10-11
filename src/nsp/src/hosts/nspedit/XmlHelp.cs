using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Windows.Forms;
using System.Xml;

namespace NSPEdit
{
	public class XmlHelp
	{
		public class XmlHelpEntry
		{
			public string fullname = "";
			public string name = "";
			public string type = "";
			public string desc = "";
			public string parameters = "";
			public string returns = "";
		}
		private static XmlDocument xdoc1 = null;
		private static XmlDocument xdoc2 = null;
		private static XmlDocument xdoc3 = null;
		private static DateTime lastload = DateTime.Now.AddMinutes(-1);

		private static void loadfiles()
		{
			//private static string filename1 = embedded resource "NSPEdit.Resources.NSPNameSpace.xml"
			string filename2 = Path.GetDirectoryName(Application.ExecutablePath) + @"\NSPNameSpace.xml";
			string filename3 = Directory.GetCurrentDirectory() + @"\NSPNameSpace.xml";

			if (DateTime.Now < lastload.AddMinutes(1)) return;
			lastload = DateTime.Now;
			if (xdoc1 == null)
			{
				//Assembly a = Assembly.GetExecutingAssembly();
				//foreach (string resource in a.GetManifestResourceNames()) Program.Log("'{0}'", resource);
				//Stream s = a.GetManifestResourceStream("NSPEdit.Resources.NSPNameSpace.xml");
				//xdoc1 = new XmlDocument();
				//xdoc1.Load(s);
				//s.Close();
				xdoc1 = new XmlDocument();
				xdoc1.LoadXml(Program.GetResource("NSPNameSpace.xml"));
			}
			if (DateTime.Now < lastload.AddMinutes(1)) return;
			lastload = DateTime.Now;
			if (xdoc2 == null && File.Exists(filename2))
			{
				xdoc2 = new XmlDocument();
				try
				{
					xdoc2.Load(filename2);
				}
				catch
				{
					xdoc2 = null;
					//Program.Log("{0} failed to load", filename2);
				}
			}
			if (xdoc3 == null && File.Exists(filename3))
			{
				xdoc3 = new XmlDocument();
				try
				{
					xdoc3.Load(filename3);
				}
				catch
				{
					xdoc3 = null;
					//Program.Log("{0} failed to load", filename2);
				}
			}
		}

		public static XmlHelpEntry findnode(string parentnamespace)
		{
			Func<XmlDocument, string, XmlNode> parseresult = (doc, ns) =>
			{
				if (doc == null) return null;
				XmlNode x = null;
				XmlNode NSPNameSpace = doc.DocumentElement.SelectSingleNode("/NSPNameSpace");
				try
				{
					x = (ns == "") ? NSPNameSpace : NSPNameSpace.SelectSingleNode(ns);
				}
				catch { }
				return x;
			};
			loadfiles();
			XmlNode xnode = null;
			if (parentnamespace != "")
			{
				string childnamespace = parentnamespace.Replace(".", "/");
				if (childnamespace.StartsWith("_GLOBALS")) childnamespace = childnamespace.Substring(8, childnamespace.Length - 8);
				if (childnamespace.StartsWith("/")) childnamespace = childnamespace.Substring(1, childnamespace.Length - 1);
				if (childnamespace.EndsWith("/")) childnamespace = childnamespace.Substring(0, childnamespace.Length - 1);
				xnode = parseresult(xdoc3, childnamespace);
				if (xnode == null) xnode = parseresult(xdoc2, childnamespace);
				if (xnode == null) xnode = parseresult(xdoc1, childnamespace);
			}
			if (xnode == null) return null;
			XmlHelpEntry he = new XmlHelpEntry();
			he.fullname = parentnamespace;
			he.name = parentnamespace;
			if (xnode != null)
			{
				he.type = xnode.Attributes["type"] != null ? xnode.Attributes["type"].Value : "";
				he.desc = xnode.Attributes["desc"] != null ? xnode.Attributes["desc"].Value : "";
				he.parameters = xnode.Attributes["params"] != null ? xnode.Attributes["params"].Value : "";
				he.returns = xnode.Attributes["returns"] != null ? xnode.Attributes["returns"].Value : "";
			}
			return he;
		}

		public static List<XmlHelpEntry> getlist(string parentnamespace)
		{
			List<XmlHelpEntry> entries = new List<XmlHelpEntry>();
			Func<XmlDocument, string, bool> parseresult = (xdoc, ns) =>
			{
				if (xdoc == null) return false;
				XmlNode NSPNameSpace = xdoc.DocumentElement.SelectSingleNode("/NSPNameSpace");
				XmlNode x = (ns == "") ? NSPNameSpace : NSPNameSpace.SelectSingleNode(ns);
				if (x == null) return false;
				foreach (XmlNode xnode in x.ChildNodes)
				{
					XmlHelpEntry he = new XmlHelpEntry();
					he.fullname = parentnamespace + (parentnamespace == "" ? "" : ".") + xnode.Name;
					he.name = xnode.Name;
					he.type = xnode.Attributes["type"] != null ? xnode.Attributes["type"].Value : "";
					he.desc = xnode.Attributes["desc"] != null ? xnode.Attributes["desc"].Value : "";
					he.parameters = xnode.Attributes["params"] != null ? xnode.Attributes["params"].Value : "";
					he.returns = xnode.Attributes["returns"] != null ? xnode.Attributes["returns"].Value : "";
					bool found = false;
					foreach (XmlHelpEntry entry in entries) if (entry.name == he.name) found = true;
					if (!found) entries.Add(he);
				}
				if (entries.Count != 0) return true;
				return false;
			};
			loadfiles();
			if (parentnamespace != "")
			{
				string childnamespace = parentnamespace.Replace(".", "/");
				if (childnamespace.StartsWith("_GLOBALS")) childnamespace = childnamespace.Substring(8, childnamespace.Length - 8);
				childnamespace = childnamespace.Trim('/');
				parseresult(xdoc3, childnamespace);
				parseresult(xdoc2, childnamespace);
				parseresult(xdoc1, childnamespace);
			}
			return entries;
		}
	}
}
