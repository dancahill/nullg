using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace NSPEdit
{
	public class AutoCompleteBox : ComboBox
	{
		RichCodeBox richCodeBox1;

		public AutoCompleteBox(RichCodeBox richCodeBox1)
		{
			this.richCodeBox1 = richCodeBox1;
			this.DropDownStyle = ComboBoxStyle.DropDown;
			this.AutoCompleteSource = AutoCompleteSource.ListItems;
			this.AutoCompleteMode = AutoCompleteMode.SuggestAppend;
			this.FlatStyle = FlatStyle.Flat;
			this.Font = new Font("Courier New", 10, FontStyle.Regular);
			this.ForeColor = Color.DarkCyan;
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
			if (Char.IsControl(e.KeyChar))
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
		}

		void CB_KeyDown(object sender, KeyEventArgs e)
		{
			if (e.KeyCode == Keys.Enter)
			{
				if (this.SelectedIndex < 0) this.SelectedIndex = 0;
				richCodeBox1.SelectedText = this.Text;
				this.Visible = false;
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
	}
}
