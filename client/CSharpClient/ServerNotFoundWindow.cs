﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace CSharpClient
{
    public partial class ServerNotFoundWindow : Form
    {
        public ServerNotFoundWindow()
        {
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e) 
        {
         
            var connectForm = new connectWindow();

            this.Dispose(); // close the error window
            connectForm.ShowDialog(); // show the connection window
            

        }
    }
}
