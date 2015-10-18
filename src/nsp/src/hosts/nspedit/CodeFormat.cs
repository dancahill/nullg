using System;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace NSPEdit
{
	class CodeFormat
	{
		private string[] flist = new string[] { "exit", "throw", "finally", "catch", "try", "default", "case", "switch", "while", "do", "foreach", "for", "else", "if", "var", "local", "global", "function", "class", "new", "delete", "return", "continue", "break", "in", "namespace" };
		private string[] rlist = new string[] { "true", "false", "null", "this" };
		private static string RTFHeader =
			@"{\rtf1\ansi\ansicpg1252\deff0{\fonttbl{\f0\fnil\fcharset0 Courier New;}}" + "\r\n" +
			@"{\colortbl ;\red0\green128\blue0;\red255\green0\blue0;\red0\green0\blue0;\red0\green139\blue139;\red128\green0\blue0;\red0\green0\blue255;\red0\green0\blue128;\red255\green192\blue203; }" + "\r\n" +
			@"\viewkind4\uc1\pard\tx960\tx1920\tx2880\tx3840\tx4800\tx5760\lang1033\f0\fs20 ";
		private StringBuilder sb;
		private string rtbtext;
		private int rtblen;
		private int sob;
		private int cur;
		private int eob;
		private string outformat = "RTF";
		private string output = "";

		public static string AddColour(RichTextBox richTextBox1, string outputtype)
		{
			CodeFormat CF = new CodeFormat();
			CF.rtbtext = richTextBox1.Text + " ";
			CF.rtblen = richTextBox1.TextLength;
			CF.outformat = outputtype;

			switch (CF.outformat)
			{
				case "HTML":
					CF.output = "<pre>\r\n";
					CF.AddColour(0, CF.rtblen);
					CF.output += "</pre>\r\n";
					return CF.output;
				case "RTF":
					CF.sb = new StringBuilder();
					CF.sb.Append(RTFHeader);
					CF.AddColour(0, CF.rtblen);
					CF.sb.Append("\\par\r\n}\r\n");
					return CF.sb.ToString();
				default:
					throw new Exception("invalid output type");
			}
		}

		//public static void AddColour(RichTextBox richTextBox1, int readptr, int endptr)
		//{
		//	CodeFormat CF = new CodeFormat();
		//	CF.outformat = "RTF";
		//	CF.sb = new StringBuilder();
		//	CF.sb.Append(@"{\rtf1 ");
		//	CF.rtbtext = richTextBox1.Text + " ";
		//	CF.rtblen = richTextBox1.TextLength;
		//	CF.AddColour(readptr, endptr);
		//	CF.sb.Append("}");
		//	richTextBox1.Select(readptr, endptr);
		//	richTextBox1.DeselectAll();
		//}

		private void AddColour(int _readptr, int _endptr)
		{
			bool pm = false;

			sob = _readptr;
			cur = _readptr;
			eob = _endptr;
			try
			{
				while (cur < eob)
				{
					char p = rtbtext[cur];
					if (p == '#')
					{
						int end = cur;
						while (rtbtext[end] != '\r' && rtbtext[end] != '\n' && end < eob) end++;
						SetRangeFont(cur, end - cur, Color.Green, FontStyle.Italic);
						cur = end;
						//continue;
					}
					else if (p == '/' && rtbtext[cur + 1] == '/')
					{
						int end = cur;
						while (rtbtext[end] != '\r' && rtbtext[end] != '\n' && end < eob) end++;
						SetRangeFont(cur, end - cur, Color.Green, FontStyle.Italic);
						cur = end;
						//continue;
					}
					else if (p == '/' && rtbtext[cur + 1] == '*')
					{
						int end = cur;
						bool term = false;
						while (end < eob)
						{
							if (rtbtext[end] == '*' && rtbtext[end + 1] == '/')
							{
								end += 2;
								SetRangeFont(cur, end - cur, Color.Green, FontStyle.Italic);
								term = true;
								break;
							}
							end++;
						}
						if (!term) SetRangeFont(cur, end - cur, Color.Green, FontStyle.Italic);
						cur = end;
						//continue;
					}
					else if (rtbtext[cur] == '"' || rtbtext[cur] == '\'')
					{
						int end = skipquote(rtbtext[cur]);
						SetRangeFont(cur, end - cur, Color.Blue, FontStyle.Regular);
						cur = end;
						//continue;
					}
					else if (p == '_' || p == '$' || char.IsLetter(p))
					{
						int end = cur + 1;
						string sub = "";
						while (end <= eob)
						{
							if (end >= rtblen)
							{
								end = rtblen;
								sub = rtbtext.Substring(cur, end - cur);
								break;
							}
							else if (rtbtext[end] != '_' && !char.IsLetterOrDigit(rtbtext[end]))
							{
								sub = rtbtext.Substring(cur, end - cur);
								break;
							}
							end++;
						}
						if (!pm && flist.Contains(sub))
							SetRangeFont(cur, sub.Length, Color.Black, FontStyle.Bold);
						else if (!pm && rlist.Contains(sub))
							SetRangeFont(cur, sub.Length, Color.Blue, FontStyle.Regular);
						else
							SetRangeFont(cur, sub.Length, Color.DarkCyan, FontStyle.Regular);
						cur = end;
						//continue;
					}
					else if (char.IsDigit(p))
					{
						int end = cur + 1;
						SetRangeFont(cur, end - cur, Color.Navy, FontStyle.Regular);
						cur = end;
						//continue;
					}
					else if ("=+-*/%&|^!<>:?".Contains(p) || "(),{};.[]".Contains(p))
					{
						int end = cur + 1;
						SetRangeFont(cur, end - cur, Color.Maroon, FontStyle.Bold);
						cur = end;
						if (p == '.')
						{
							pm = true;
							continue;
						}
						//continue;
					}
					else
					{
						int end = cur + 1;
						SetRangeFont(cur, end - cur, Color.Red, FontStyle.Bold);
						cur = end;
						//continue;
					}
					pm = false;
				}
			}
			catch (Exception ex)
			{
				//RTB.SelectAll();
				//SetFont(Color.Black, FontStyle.Regular);
				System.Diagnostics.Debug.WriteLine(string.Format("AddColour() Exception {0}", ex.Message));
			}
		}

		private int skipquote(char c)
		{
			int end = cur + 1;
			while (end < eob)
			{
				if (rtbtext[end] == '\\')
				{
					end++;
				}
				else if (rtbtext[end] == c)
				{
					end++;
					break;
				}
				end++;
			}
			if (end > eob) end = eob;
			return end;
		}

		private void SetRangeFont(int start, int length, Color color, FontStyle style)
		{
			if (outformat == "HTML")
			{
				string StyleB = "";
				string StyleE = "";
				if (style == FontStyle.Bold)
				{
					StyleB = "<b>";
					StyleE = "</b>";
				}
				else if (style == FontStyle.Italic)
				{
					StyleB = "<i>";
					StyleE = "</i>";
				}
				string c = string.Format("#{0:X2}{1:X2}{2:X2}", color.R, color.G, color.B);
				string st = rtbtext.Substring(start, length).Replace("&", "&amp;").Replace("<", "&lt;").Replace(">", "&gt;");
				if (string.IsNullOrWhiteSpace(st)) output += st;
				else output += "<font color='" + c + "'>" + StyleB + st + StyleE + "</font>";
			}
			else
			{
				string StyleB = "";
				string StyleE = "";
				string c = "";
				string st = rtbtext.Substring(start, length);

				if (!string.IsNullOrWhiteSpace(st))
				{
					if (style == FontStyle.Bold)
					{
						StyleB = @"\b";
						StyleE = @"\b0";
					}
					else if (style == FontStyle.Italic)
					{
						StyleB = @"\i";
						StyleE = @"\i0";
					}
					if (color == Color.Green) c = @"\cf1";
					else if (color == Color.Red) c = @"\cf2";
					else if (color == Color.Black) c = @"\cf3";
					else if (color == Color.DarkCyan) c = @"\cf4";
					else if (color == Color.Maroon) c = @"\cf5";
					else if (color == Color.Blue) c = @"\cf6";
					else if (color == Color.Navy) c = @"\cf7";
					else c = @"\cf3";
				}
				sb.Append(c + StyleB + (c == "" ? "" : " "));
				sb.Append(st.Replace("\\", "\\\\").Replace("{", "\\{").Replace("}", "\\}").Replace("\t", "\\tab ").Replace("\r", "").Replace("\n", "\\par\r\n"));
				sb.Append(StyleE + (StyleE == "" ? "" : " "));
			}
			//else
			//{
			//	RTB.ReadOnly = true;
			//	RTB.Select(start, length);
			//	RTB.SelectionColor = color;
			//	RTB.SelectionFont = new Font("Courier New", 10, style);
			//	RTB.DeselectAll();
			//	RTB.ReadOnly = false;
			//}
			//	Application.DoEvents();
		}

		private enum FormatType
		{
			WhiteSpace,
			Comment,
			Quote,
			Label,
			Number,
			Punctuation,
			Unknown
		}
	}
}
