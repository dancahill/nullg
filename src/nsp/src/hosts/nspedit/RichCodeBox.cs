using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace NSPEdit
{
	public class RichCodeBox : RichTextBox
	{
		[DllImport("user32")]
		private static extern int LockWindowUpdate(IntPtr hWnd);
		[DllImport("user32.dll")]
		private static extern int SendMessage(IntPtr hWnd, Int32 wMsg, Int32 wParam, Int32 lParam);
		[DllImport("user32.dll")]
		private static extern IntPtr SendMessage(IntPtr hWnd, Int32 wMsg, Int32 wParam, IntPtr lParam);
		[DllImport("user32.dll")]
		private static extern int SendMessage(IntPtr hWnd, Int32 wMsg, Int32 wParam, ref Point pt);

		[DllImport("user32.dll", CharSet = CharSet.Auto)]
		private static extern IntPtr SendMessage(IntPtr hWnd, int msg, IntPtr wParam, IntPtr lParam);

		const int WM_USER = 0x400;
		const int EM_HIDESELECTION = WM_USER + 63;
		const int EM_GETSCROLLPOS = WM_USER + 221;
		const int EM_SETSCROLLPOS = WM_USER + 222;





		//		const int WM_USER = 0x400;
		const int WM_SETREDRAW = 0x000B;
		const int EM_GETEVENTMASK = WM_USER + 59;
		const int EM_SETEVENTMASK = WM_USER + 69;
		//		const int EM_GETSCROLLPOS = WM_USER + 221;
		//		const int EM_SETSCROLLPOS = WM_USER + 222;


		//private IntPtr Handle;

		//private const int WM_USER = 0x0400;
		//		private const int EM_SETEVENTMASK = (WM_USER + 69);
		//		private const int WM_SETREDRAW = 0x0b;
		//private IntPtr OldEventMask;

		Point pt;
		int ss;

		int tabwidth = 64;
		DateTime LastUpdate;
		Stack<UndoStep> undoList = new Stack<UndoStep>(100);
		RichTextBox richTextBox2;
		public ScriptThread scriptThread = null;

		public string LastSavedText = "";

		public RichCodeBox()
		{
			this.richTextBox2 = Program.MainForm.richTextBox2;
			LastUpdate = DateTime.Now;
			// richTextBox1
			this.AcceptsTab = true;
			this.DetectUrls = false;
			this.Dock = System.Windows.Forms.DockStyle.Fill;
			this.Location = new System.Drawing.Point(0, 0);
			this.Name = "RichCodeBox1";
			//richTextBox1.Size = new System.Drawing.Size(795, 364);
			//richTextBox1.TabIndex = 0;
			this.TabStop = false;
			this.Text = "";
			this.WordWrap = false;

			this.SelectionTabs = new int[] { tabwidth * 1, tabwidth * 2, tabwidth * 3, tabwidth * 4, tabwidth * 5, tabwidth * 6 };
			this.BackColor = Color.WhiteSmoke;
			this.ForeColor = Color.Black;
			this.Font = new Font("Courier New", 10, FontStyle.Regular);
			this.Text = "";
			this.AllowDrop = true;
			this.SetEvents();
		}

		public void BeginUpdate()
		{
			SendMessage(this.Handle, WM_SETREDRAW, IntPtr.Zero, IntPtr.Zero);
			//OldEventMask = (IntPtr)SendMessage(RTB.Handle, EM_SETEVENTMASK, IntPtr.Zero, IntPtr.Zero);

			LockWindowUpdate(this.Handle);
			SendMessage(this.Handle, EM_HIDESELECTION, 1, 0);

			//this.DoubleBuffered = true;
			//RTB.SuspendLayout();
			//RTB.Visible = false;
			//RTB.Enabled = false;

			//https://msdn.microsoft.com/en-us/library/aa970779%28v=vs.110%29.aspx
			//Usually the TextChanged event should be used to detect whenever the text in a TextBox or RichTextBox changes rather then KeyDown as you might expect.
			//See How to: Detect When Text in a TextBox Has Changed for an example.
			ss = this.SelectionStart;
			pt = new Point();
			SendMessage(this.Handle, EM_GETSCROLLPOS, 0, ref pt);
		}

		public void RefreshRTF()
		{
			this.TextChanged -= RichCodeBox_TextChanged;
			this.BeginUpdate();
			this.Rtf = CodeFormat.AddColour(this, "RTF");
			this.EndUpdate();
			this.TextChanged += RichCodeBox_TextChanged;
		}

		public void EndUpdate()
		{
			this.SelectionLength = 0;
			this.SelectionStart = ss;
			//RTB.DeselectAll();
			SendMessage(this.Handle, EM_SETSCROLLPOS, 0, ref pt);









			SendMessage(this.Handle, WM_SETREDRAW, (IntPtr)1, IntPtr.Zero);
			//SendMessage(RTB.Handle, EM_SETEVENTMASK, IntPtr.Zero, OldEventMask);

			LockWindowUpdate((IntPtr)0);
			SendMessage(this.Handle, EM_HIDESELECTION, 0, 0);

			//RTB.Enabled = true;
			//RTB.ResumeLayout();
			//RTB.Invalidate();
			//RTB.Visible = true;
		}

		//public void ClearOutput()
		//{
		//	richTextBox2.Text = "";
		//}

		//public void AppendOutput(string OutString)
		//{
		//	richTextBox2.Text += OutString;
		//	Application.DoEvents();
		//}


		public void LoadScript(string filetoread)
		{
			this.TextChanged -= RichCodeBox_TextChanged;
			try
			{
				this.Text = "";
				undoList.Clear();
				if (filetoread != "")
				{
					Program.MainForm.ClearOutput();
					Program.MainForm.AppendOutput(string.Format("Loading {0}\r\n", filetoread));
					this.Text = File.ReadAllText(filetoread);
					DateTime t1 = DateTime.Now;
					this.BeginUpdate();
					this.Rtf = CodeFormat.AddColour(this, "RTF");
					this.EndUpdate();
					Program.MainForm.ClearOutput();
					Program.MainForm.AppendOutput(string.Format("Loaded {0} ({1} seconds)\r\n", filetoread, (DateTime.Now - t1).TotalSeconds));
					Program.MainForm.SetStatus("Loaded " + filetoread);
					Program.MainForm.SetScriptFileName(filetoread);
				}
				else
				{
					//this.Text = "// New script\r\nprintf(\"Hello.\");\r\n";
					this.Text = Program.GetResource("example1.ns");
					this.BeginUpdate();
					this.Rtf = CodeFormat.AddColour(this, "RTF");
					this.EndUpdate();
				}
				this.Select(0, 0);
				this.SelectionTabs = new int[] { tabwidth * 1, tabwidth * 2, tabwidth * 3, tabwidth * 4, tabwidth * 5, tabwidth * 6 };
				undoList.Push(new UndoStep { RTF = this.Rtf, SelectionStart = 0 });
				this.LastSavedText = this.Text;
			}
			catch (Exception ex)
			{
				MessageBox.Show(ex.Message, "LoadScript()");
			}
			this.TextChanged += RichCodeBox_TextChanged;
		}

		public void RunScript(string filetoread)
		{
			try
			{
				if (scriptThread != null && scriptThread.IsAlive())
				{
					scriptThread.Resume();
					return;
				}
				Program.MainForm.ClearOutput();
				if (this.TextLength == 0) throw new Exception("script is empty");
				string srcfile = Program.MainForm.GetScriptFileName();
				if (srcfile != "") Directory.SetCurrentDirectory(Path.GetDirectoryName(srcfile));

				richTextBox2 = Program.MainForm.richTextBox2;
				scriptThread = new ScriptThread(this.Text, srcfile)
				{
					WriteBuffer = WriteBuffer
				};
				scriptThread.Run();
			}
			catch (Exception ex)
			{
				Program.MainForm.AppendOutput(ex.Message);
				MessageBox.Show(ex.Message, "RunScript()");
			}
		}

		public void ContinueScript()
		{
			if (scriptThread == null) return;
			scriptThread.Resume();
		}

		private void WriteBuffer(string outstr)
		{
			//this.richTextBox2.Text += outstr;
			Invoke(new Action(() => this.richTextBox2.AppendText(outstr)));
			Application.DoEvents();
		}

		private void SetEvents()
		{
			this.KeyUp += RichCodeBox_KeyUp;
			this.KeyDown += RichCodeBox_KeyDown;
			this.KeyPress += RichCodeBox_KeyPress;
			this.MouseClick += RichCodeBox_MouseClick;
			this.DragDrop += RichCodeBox_DragDrop;
		}

		private void UnsetEvents()
		{
			this.KeyUp -= RichCodeBox_KeyUp;
			this.KeyDown -= RichCodeBox_KeyDown;
			this.KeyPress -= RichCodeBox_KeyPress;
			this.MouseClick -= RichCodeBox_MouseClick;
			this.DragDrop -= RichCodeBox_DragDrop;
		}

		private void RichCodeBox_KeyUp(object sender, KeyEventArgs e)
		{
		}

		private void RichCodeBox_KeyDown(object sender, KeyEventArgs e)
		{
			if (e.KeyCode == Keys.Space || e.KeyCode == Keys.Enter)
			{
				undoList.Push(new UndoStep { RTF = this.Rtf, SelectionStart = this.SelectionStart });
			}
			if (e.Control && e.KeyCode == Keys.Z)
			{
				if (undoList.Count < 1) return;
				UndoStep s = undoList.Pop();
				this.TextChanged -= RichCodeBox_TextChanged;
				this.BeginUpdate();
				this.Rtf = s.RTF;
				this.EndUpdate();
				this.Select(s.SelectionStart, 0);
				this.TextChanged += RichCodeBox_TextChanged;
				Program.MainForm.RestartFormatTimer();
				return;
			}
			if (e.Control && e.KeyCode == Keys.Y)
			{
			}
		}

		private void RichCodeBox_KeyPress(object sender, KeyPressEventArgs e)
		{
		}

		private void RichCodeBox_MouseClick(object sender, MouseEventArgs e)
		{
		}

		private void RichCodeBox_DragDrop(object sender, DragEventArgs e)
		{
		}

		private void RichCodeBox_TextChanged(object sender, EventArgs e)
		{
			//undoList.Push(this.Rtf);
			//if ((DateTime.Now - LastUpdate).TotalSeconds < 1)
			//{
			Program.MainForm.RestartFormatTimer();
			return;
			//}

			//this.TextChanged -= RichCodeBox_TextChanged;
			//LastUpdate = DateTime.Now;
			//int charline = this.GetFirstCharIndexOfCurrentLine();
			//int line = this.GetLineFromCharIndex(charline);
			//int charlen = (this.Lines.Length > 0) ? this.Lines[line].Length : 0;
			//this.BeginUpdate();
			////CodeFormat.AddColour(this, charline, charline + charlen);
			//this.Rtf = CodeFormat.AddColour(this, "RTF");
			//this.EndUpdate();
			//this.TextChanged += RichCodeBox_TextChanged;
		}

		protected override CreateParams CreateParams
		{
			get
			{
				CreateParams i_Params = base.CreateParams;
				try
				{
					Win32.LoadLibrary("MsftEdit.dll");
					//i_Params.ClassName = "RichEdit20W";
					i_Params.ClassName = "RichEdit50W";
				}
				catch
				{ /* Windows XP without any Service Pack. */
				}
				return i_Params;
			}
		}

		private class Win32
		{
			[DllImport("kernel32.dll", EntryPoint = "LoadLibraryW", CharSet = CharSet.Unicode, SetLastError = true)]
			private static extern IntPtr LoadLibraryW(string s_File);

			public static IntPtr LoadLibrary(string s_File)
			{
				IntPtr h_Module = LoadLibraryW(s_File);
				if (h_Module != IntPtr.Zero) return h_Module;
				int s32_Error = Marshal.GetLastWin32Error();
				throw new System.ComponentModel.Win32Exception(s32_Error);
			}
		}

		private struct UndoStep
		{
			public string RTF;
			public int SelectionStart;
		}


		//public string getlabel(RichCodeBox rcb, int charindex, bool readpastcursor)
		public string getlabel(int charindex, bool readpastcursor)
		{
			if (this.Lines.Length == 0) return "";
			int linenum = this.GetLineFromCharIndex(charindex);
			int linestartindex = this.GetFirstCharIndexFromLine(linenum);
			string lineText = this.Lines[linenum];
			if (linestartindex >= charindex) return "";
			string subline = this.Text.Substring(linestartindex, charindex - linestartindex);
			string sub = "";
			//Program.Log("line='{0}'\r\n", lineText);
			//Program.Log("subline='{0}'\r\n", subline);
			for (int cur = 0; cur < subline.Length; cur++)
			{
				char p = subline[cur];

				if (sub == "" && (p == '_' || p == '$' || char.IsLetter(p)))
				{
					sub += p;
				}
				else if (p != '_' && !char.IsLetterOrDigit(p) && p != '.')
				{
					sub = "";
				}
				else
				{
					sub += p;
				}
			}
			if (readpastcursor)
			{
				for (int cur = subline.Length; cur < lineText.Length; cur++)
				{
					char p = lineText[cur];

					if (sub == "" && (p == '_' || p == '$' || char.IsLetter(p)))
					{
						sub += p;
					}
					//else if (p != '_' && !char.IsLetterOrDigit(p) && p != '.')
					else if (p != '_' && !char.IsLetterOrDigit(p))
					{
						break;
					}
					else
					{
						sub += p;
					}
				}

			}
			if (sub.EndsWith(".")) sub = sub.Substring(0, sub.Length - 1);
			//Program.MainForm.AppendOutput(string.Format("sub='{0}'\r\n", sub));
			return sub;
		}

		public void GetFontFromPosition(int charindex, out Color color, out Font font)
		{
			//// start ugly hack
			////RichTextBox rtbx = new RichTextBox();
			////rtbx.Rtf = rcb.Rtf;
			////int oldindex = rcb.SelectionStart;
			////rtbx.Select(charindex, 0);
			////Color color = rtbx.SelectionColor;
			////Font font = rtbx.SelectionFont;
			////rtbx.Dispose();

			//int ss = rcb.SelectionStart;
			//int sl = rcb.SelectionLength;
			//Point p = rcb.AutoScrollOffset;
			//rcb.Select(charindex, 0);
			//Color color = rcb.SelectionColor;
			//Font font = rcb.SelectionFont;
			//rcb.Select(ss, sl);
			//rcb.AutoScrollOffset = p;
			//// end ugly hack

			Point _ScrollPoint = new Point();
			IntPtr _EventMask;

			SendMessage(this.Handle, EM_GETSCROLLPOS, 0, ref _ScrollPoint);
			SendMessage(this.Handle, WM_SETREDRAW, 0, IntPtr.Zero);
			_EventMask = SendMessage(this.Handle, EM_GETEVENTMASK, 0, IntPtr.Zero);

			int ss = SelectionStart;
			int sl = SelectionLength;
			//			Point p = AutoScrollOffset;
			Select(charindex, 0);
			color = SelectionColor;
			font = SelectionFont;
			Select(ss, sl);
			//AutoScrollOffset = p;

			SendMessage(this.Handle, EM_SETSCROLLPOS, 0, ref _ScrollPoint);
			SendMessage(this.Handle, EM_SETEVENTMASK, 0, _EventMask);
			SendMessage(this.Handle, WM_SETREDRAW, 1, IntPtr.Zero);
			//this.Invalidate();
		}
	}
}
