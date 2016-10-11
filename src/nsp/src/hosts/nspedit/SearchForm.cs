using System;
using System.Drawing;
using System.Windows.Forms;

namespace NSPEdit
{
	public partial class SearchForm : Form
	{
		string lastsearch;

		public SearchForm()
		{
			InitializeComponent();
			this.Load += SearchForm_Load;
			this.Activated += SearchForm_Activated;
			this.Deactivate += SearchForm_Deactivate;
		}

		private void SearchForm_Load(object sender, EventArgs e)
		{
			this.DesktopLocation = new Point(Program.MainForm.DesktopLocation.X + Program.MainForm.Width - this.Width - 25, Program.MainForm.DesktopLocation.Y + 110);
		}

		private void SearchForm_Deactivate(object sender, EventArgs e)
		{
			this.Opacity = 0.7;
		}

		private void SearchForm_Activated(object sender, EventArgs e)
		{
			this.Opacity = 1;
		}

		public void buttonSearch_Click(object sender, EventArgs e)
		{
			RichCodeBox rcb = Program.MainForm.GetActiveCodeBox();
			try
			{
				if (SearchBox.Text == "")
				{
					this.Show();
					return;
				}
				if (SearchBox.Text != lastsearch)
				{
					lastsearch = SearchBox.Text;
				}
				if (string.IsNullOrEmpty(rcb.Text)) return;
				int index = rcb.Find(SearchBox.Text, rcb.SelectionStart + 1, rcb.TextLength, RichTextBoxFinds.None);
				if (index < 0) MessageBox.Show(string.Format("no matches for '{0}'", SearchBox.Text));
			}
			catch (Exception ex) { MessageBox.Show(ex.Message, "Error"); }
			//Program.Log("Program.MainForm.DesktopLocation = '{0}'", Program.MainForm.DesktopLocation);
			//Program.Log("Program.MainForm.Width = '{0}'", Program.MainForm.Width);
			//Program.Log("this.DesktopLocation = '{0}'", this.DesktopLocation);
		}

		private void buttonCancel_Click(object sender, EventArgs e)
		{
			this.Hide();
		}

		protected override void OnFormClosing(FormClosingEventArgs e)
		{
			e.Cancel = true;
			//base.OnFormClosing(e);
			this.Hide();
		}

		public void SetSearch(string s)
		{
			if (!string.IsNullOrEmpty(s)) SearchBox.Text = s;
		}
	}
}
