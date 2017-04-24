﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Windows.Forms;

namespace CSharpClient
{
    public partial class mainWindow : Form
    {
        public class StateObject
        {
            public Socket WorkSocket = null;
            public const int BufferSize = 256;
            // Receive buffer.  
            public byte[] Buffer = new byte[BufferSize];
            // Received data string.  
            public StringBuilder Sb = new StringBuilder();
        }

        private static string RESPONSE = string.Empty;
        private static readonly ManualResetEvent receiveDone = new ManualResetEvent(false);
        private IPAddress serverAddress;
        private int serverPort;
        private IPEndPoint endPoint;
        private Socket client;
        private StateObject state;

        public string Usernames = "UserNames";

        public mainWindow()
        {
            InitializeComponent();
           

        }
        //192.168.56.1
        private void connectToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var connectForm = new connectWindow(); //Create a var for a new connect window
            var errorForm = new ServerNotFoundWindow(); //create a var for a new error window

            if (connectForm.ShowDialog() != DialogResult.OK)
                return;

            var validAddress = connectForm.GetAddress(out serverAddress, out serverPort);

            if (!validAddress) // if the address is enetered incorrectly 
            {
                errorForm.ShowDialog(); // show the error window
            
            }


            endPoint = new IPEndPoint(serverAddress, serverPort);
            client = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            client.Connect(endPoint);

            if (!client.Connected) return;

            state = new StateObject { WorkSocket = client };
            disconnectToolStripMenuItem.Enabled = true;

            // Begin receiving the data from the remote device.  
            client.BeginReceive(state.Buffer, 0, StateObject.BufferSize, 0,
                new AsyncCallback(ReceiveCallback), state);
        }

        public void UpdateUsers() 
        {
            Usernames = "Test";

           // userList.Items.Add(Usernames);
        }

        public delegate void UpdateMessageBox();
        public void UpdateMessages()
        {
            if (messageText.InvokeRequired)
            {
                messageText.Invoke((MethodInvoker)delegate{messageText.AppendText(DateTime.Now.ToString("hh:mm:ss tt : ", System.Globalization.DateTimeFormatInfo.InvariantInfo) + state.Sb.ToString() + "\n"); });
                state.Sb.Clear();
            }
        }

        private void ReceiveCallback(IAsyncResult ar)
        {
            var stateObject = (StateObject) ar.AsyncState;
            var workSocket = stateObject.WorkSocket;

            // Read data from the remote device.  
            var bytesRead = client.EndReceive(ar);

            if (bytesRead > 0)
            {
                // There might be more data, so store the data received so far.  
                stateObject.Sb.Append(Encoding.ASCII.GetString(stateObject.Buffer, 0, bytesRead));
                UpdateMessages();
               
            }

            // Get the rest of the data.  
            client.BeginReceive(stateObject.Buffer, 0, StateObject.BufferSize, 0,
                new AsyncCallback(ReceiveCallback), stateObject);
        }

        private void Disconnect()
        {
            if (client == null || !client.Connected)
                return;

            client.Shutdown(SocketShutdown.Both);
            client.Disconnect(true);
            disconnectToolStripMenuItem.Enabled = false;
        }

        private void disconnectToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Disconnect();
        }


    
        private void sendBtn_Click(object sender, EventArgs e)
        {
            if (!client.Connected)
                return;

            UpdateUsers();

            var msg = messageBox.Text;

            var msgBytes = Encoding.ASCII.GetBytes(msg);

            client.Send(msgBytes);

            messageBox.Clear();
        }
    }
}



