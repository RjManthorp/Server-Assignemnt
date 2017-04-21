using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace CSharpClient
{
    public partial class connectWindow : Form
    {
        public connectWindow()
        {
            InitializeComponent();
        }

        public bool GetAddress(out IPAddress ip, out int port)
        {
            var ipAddress = ipAddressText.Text;
            var validPort = int.TryParse(portText.Text, out port);
            return IPAddress.TryParse(ipAddress, out ip) && validPort;
        }

        private void CancelBtn_Click(object sender, EventArgs e)
        {

        }

        private void ConnectBtn_Click(object sender, EventArgs e)
        {

        }
    }
}
