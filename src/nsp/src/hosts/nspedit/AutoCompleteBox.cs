using System;
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;

namespace NSPEdit
{
	public class AutoCompleteBox : ComboBox
	{
		RichCodeBox richCodeBox1;
		public string nsnamespace;

		public AutoCompleteBox(RichCodeBox richCodeBox1)
		{
			this.richCodeBox1 = richCodeBox1;
			this.DropDownStyle = ComboBoxStyle.DropDown;
			this.AutoCompleteSource = AutoCompleteSource.ListItems;
			//this.AutoCompleteMode = AutoCompleteMode.SuggestAppend;
			this.FlatStyle = FlatStyle.Flat;
			this.Font = new Font("Courier New", 10, FontStyle.Regular);
			//this.ForeColor = Color.DarkCyan;
			this.ForeColor = Color.Black;
			this.BackColor = Color.FromArgb(0xCC, 0xFF, 0xFF);
			this.Sorted = false;
			this.Visible = false;
			this.LostFocus += CB_LostFocus;
			this.KeyDown += CB_KeyDown;
			this.KeyPress += CB_KeyPress;
			this.TextChanged += CB_TextChanged;
			this.Name = "AutoCompleteBox";
		}

		void CB_KeyPress(object sender, KeyPressEventArgs e)
		{
			if (char.IsControl(e.KeyChar))
			{
			}
			else if (Char.IsLetterOrDigit(e.KeyChar))
			{
			}
			else
			{
				if (this.SelectedIndex < 0) this.SelectedIndex = 0;
				richCodeBox1.SelectedText = this.Text;
				richCodeBox1.SelectedText = e.KeyChar.ToString();
				this.Visible = false;
				Program.MainForm.CodeTip1.Hide(Program.MainForm.richCodeBox1);
				richCodeBox1.Focus();
				if (e.KeyChar == '.')
				{
					KeyPressEventArgs x = new KeyPressEventArgs('.');
					Program.MainForm.richCodeBox1_KeyPress(null, x);
				}
			}
		}

		void CB_TextChanged(object sender, EventArgs e)
		{
			if (this.Text != "") this.Items[0] = this.Text;
			Program.MainForm.CodeTip1.Hide(Program.MainForm.richCodeBox1);
			string ns = nsnamespace + (nsnamespace == "" ? "" : ".") + this.Text;

			Color color;
			Font font;
			richCodeBox1.GetFontFromPosition(richCodeBox1.SelectionStart - 1, out color, out font);

			string t = Program.MainForm.CodeTip1.FormatToolTip(ns, color);
			Point cursorPt = richCodeBox1.GetPositionFromCharIndex(richCodeBox1.SelectionStart);
			cursorPt.X += (int)richCodeBox1.Font.SizeInPoints;
			cursorPt.Y += richCodeBox1.Location.Y;
			if (t != "" && !ns.EndsWith(".")) Program.MainForm.CodeTip1.Show(t, Program.MainForm.richCodeBox1, cursorPt.X + this.Width, cursorPt.Y, 5000);
		}

		void CB_KeyDown(object sender, KeyEventArgs e)
		{
			if (e.KeyCode == Keys.Enter)
			{
				if (this.SelectedIndex < 0) this.SelectedIndex = 0;
				richCodeBox1.SelectedText = this.Text;
				this.Visible = false;
				Program.MainForm.CodeTip1.Hide(Program.MainForm.richCodeBox1);
				richCodeBox1.Focus();
			}
			else if (e.KeyCode == Keys.Escape)
			{
				this.Text = "";
				this.Visible = false;
				richCodeBox1.Focus();
			}
		}

		void CB_LostFocus(object sender, EventArgs e)
		{
			this.Visible = false;
		}

		public void MakeActive()
		{
			this.Text = "";
			this.Items.Clear();
			this.Items.Add("");
			int maxchar = 0;

			Color color;
			Font font;
			richCodeBox1.GetFontFromPosition(richCodeBox1.SelectionStart - 1, out color, out font);

			if (color != Color.DarkCyan)
			{
				//Program.Log("probably not a good place for autocomplete {0}", color);
				return;
			}


			string label = richCodeBox1.getlabel(richCodeBox1.SelectionStart, false);
			this.nsnamespace = label;

			List<string> ACL = this.fillautocomplete(label);

			foreach (string s in ACL)
			{
				this.Items.Add(s);
				if (maxchar < s.Length) maxchar = s.Length;
			}
			this.Visible = true;
			Point cursorPt = richCodeBox1.GetPositionFromCharIndex(richCodeBox1.SelectionStart);
			cursorPt.X += (int)richCodeBox1.Font.SizeInPoints;
			cursorPt.Y += richCodeBox1.Location.Y;
			this.Size = new System.Drawing.Size(100, 1);
			this.DropDownHeight = (richCodeBox1.Font.Height + 1) * 5;
			this.Width = (maxchar + 1) * (int)richCodeBox1.Font.SizeInPoints;
			this.Location = cursorPt;
			this.BringToFront();
			this.Show();
			this.Focus();
			this.DroppedDown = true;
			this.Text = "";
		}

		public List<string> fillautocomplete(string parentnamespace)
		{
			List<string> ACL = new List<string>();
			foreach (XmlHelp.XmlHelpEntry entry in XmlHelp.getlist(parentnamespace))
			{
				if (!ACL.Contains(entry.name)) ACL.Add(entry.name);
			}
			if (!ACL.Contains("gettype")) ACL.Add("gettype");
			if (!ACL.Contains("length")) ACL.Add("length");
			if (!ACL.Contains("tostring")) ACL.Add("tostring");
			ACL.Sort();
			return ACL;
		}
	}
}
