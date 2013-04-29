using System;
using System.Collections.Generic;
using System.Collections.Concurrent;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Diagnostics;
using System.Threading;


namespace MakiWatch
{
    public partial class WatchSettings : Form
    {
        private NotifyIcon trayIcon;
        private ContextMenu trayMenu;

        private Socket sock;
        private IPEndPoint endpoint;
        
        private const int PacketFileChanged = 0;
        private string assetsDir = "";

        private Dictionary<string, DateTime> scheduledCompilations = new Dictionary<string, DateTime>();
        private Dictionary<string, DateTime> scheduledAnnouncements = new Dictionary<string, DateTime>();

        protected TaskScheduler uiScheduler;

        public WatchSettings()
        {
            InitializeComponent();

            sock = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
            endpoint = new IPEndPoint(IPAddress.Loopback, 11001);

            trayMenu = new ContextMenu();
            trayMenu.MenuItems.Add("Settings", (Object o, EventArgs a) => {
                Visible = true; ShowInTaskbar = true;
                log.ScrollToCaret();
            });
            trayMenu.MenuItems.Add("Exit", (Object o, EventArgs a) => {
                Application.Exit();
            });
            trayIcon = new NotifyIcon();
            trayIcon.Text = "MakiWatch";
            trayIcon.Icon = new Icon(SystemIcons.Exclamation, 40, 40);
            trayIcon.ContextMenu = trayMenu;
            trayIcon.Visible = true;
            trayIcon.DoubleClick += (Object o, EventArgs a) => {
                Visible = true; ShowInTaskbar = true;
                Show();
                Activate();
                BringToFront();
                log.ScrollToCaret();
            };

            clearLogButton.Click += (Object o, EventArgs a) => {
                log.Text = "";
            };

            if (Properties.Settings.Default.MillisecondWindow != 0)
            {
                millisecondWindow.Value = Properties.Settings.Default.MillisecondWindow;
            }
            millisecondWindow.ValueChanged += (Object o, EventArgs a) => {
                Properties.Settings.Default.MillisecondWindow = (int)millisecondWindow.Value;
                Properties.Settings.Default.Save();
            };

            assetsBinWatcher.Changed += BinaryAssetChanged;
            assetsWatcher.Changed += AssetChanged;

            timer.Enabled = true;
            timer.Tick += TickSchedule;
            timer.Interval = 500;
        }

        protected override void OnLoad(EventArgs e)
        {
            uiScheduler = TaskScheduler.FromCurrentSynchronizationContext();

            Visible = false;
            ShowInTaskbar = false;
            if (Properties.Settings.Default.WatchDir != null)
            {
                SetPaths(Properties.Settings.Default.WatchDir);
            }
            base.OnLoad(e);
        }

        protected override void OnClosing(CancelEventArgs e)
        {
            e.Cancel = true;
            Visible = false;
            ShowInTaskbar = false;
            base.OnClosing(e);
        }

        private void browseButton_Click(object sender, EventArgs e)
        {
            folderBrowser.SelectedPath = assetsBinDir.Text;
            DialogResult result = folderBrowser.ShowDialog();
            if (result == DialogResult.OK)
            {
                SetPaths(folderBrowser.SelectedPath);
            }
        }

        private void SetPaths(string p)
        {
            assetsBinDir.Text = p;
            
            if (Directory.Exists(assetsBinDir.Text))
            {
                log.AppendText("Assets bin dir is: " + assetsBinDir.Text + "\n");
                assetsBinWatcher.Path = assetsBinDir.Text;
                assetsBinWatcher.EnableRaisingEvents = true;

                assetsDir = Path.Combine(Path.GetDirectoryName(assetsBinDir.Text), "assets");
            }
            else
            {
                log.AppendText("Assets bin directory does not exist\n");
            }

            if (Directory.Exists(assetsDir))
            {
                log.AppendText("Assets dir is: " + assetsDir + "\n");
                assetsWatcher.Path = assetsDir;
                assetsWatcher.EnableRaisingEvents = true;
            }
            else
            {
                log.AppendText("Assets directory does not exist\n");
            }

            log.ScrollToCaret();

            Properties.Settings.Default.WatchDir = p;
            Properties.Settings.Default.Save();
        }

        protected void AssetChanged(Object o, FileSystemEventArgs a)
        {
            if(!File.Exists(a.FullPath) || Path.GetExtension(a.FullPath).Equals(".tmp"))
            {
                return;
            }

            string rootDir = Path.Combine(assetsDir, "..\\");
            scheduledCompilations[a.FullPath] = DateTime.Now;
        }

        protected void BinaryAssetChanged(Object o, FileSystemEventArgs a)
        {
            if (!File.Exists(a.FullPath) || Path.GetExtension(a.FullPath).Equals(".tmp"))
            {
                return;
            }

            Uri rootDir = new Uri(assetsBinDir.Text);
            Uri fullPath = new Uri(a.FullPath);
            Uri relative = rootDir.MakeRelativeUri(fullPath);

            string enginePath = Uri.UnescapeDataString(relative.ToString());
            enginePath = enginePath.Substring(enginePath.IndexOf(Path.AltDirectorySeparatorChar) + 1);

            scheduledAnnouncements[enginePath] = DateTime.Now;
        }

        private void TickSchedule(Object o, EventArgs a)
        {
            int lineCount = log.Lines.Count();
            if (lineCount > 3000)
            {
                log.Lines = log.Lines.ToArray().Skip(500).ToArray();
            }
            int cutoff = (int)millisecondWindow.Value;

            if (scheduledAnnouncements.Count != 0)
            {
                var removals = new List<string>();
                DateTime now = DateTime.Now;

                foreach (var entry in scheduledAnnouncements)
                {
                    if ((int)(now - entry.Value).TotalMilliseconds > cutoff)
                    {
                        removals.Add(entry.Key);
                    }
                }
                foreach (var key in removals)
                {
                    AnnounceChange(key);
                    scheduledAnnouncements.Remove(key);
                }
            }

            if (scheduledCompilations.Count != 0)
            {
                var removals = new List<string>();
                DateTime now = DateTime.Now;

                foreach (var entry in scheduledCompilations)
                {
                    if ((int)(now - entry.Value).TotalMilliseconds > cutoff)
                    {
                        removals.Add(entry.Key);
                    }
                }
                foreach (var key in removals)
                {
                    CompileChange(key);
                    scheduledCompilations.Remove(key);
                }
            }
        }

        private void AnnounceChange(string path)
        {
            log.AppendText("Announcing: " + path + "\n");
            log.ScrollToCaret();

            byte[] pathBuffer = Encoding.ASCII.GetBytes(path);
            byte[] packet = new byte[pathBuffer.Length+4];
            Array.Copy(BitConverter.GetBytes(PacketFileChanged), packet, 4);
            Array.Copy(pathBuffer, 0, packet, 4, pathBuffer.Length);

            sock.SendTo(packet, endpoint);
        }

        private void CompileChange(string target)
        {
            string manageScript = Path.Combine(System.Environment.GetEnvironmentVariable("MAKI_DIR"), "tools\\manage.py");
            string rootDir = Path.Combine(assetsDir, "..\\");

            log.AppendText("Compiling: " + target + "\n");
            log.ScrollToCaret();

            System.Diagnostics.Process process = new System.Diagnostics.Process();
            System.Diagnostics.ProcessStartInfo startInfo = new System.Diagnostics.ProcessStartInfo();

            process.StartInfo.WorkingDirectory = rootDir;
            process.StartInfo.FileName = "python.exe";
            process.StartInfo.Arguments = manageScript+" build \"" + target + "\"";
            process.StartInfo.CreateNoWindow = true;
            process.StartInfo.RedirectStandardOutput = true;
            process.StartInfo.RedirectStandardError = true;
            process.StartInfo.UseShellExecute = false;
            process.EnableRaisingEvents = true;

            var sb = new StringBuilder();
            process.OutputDataReceived += (Object o, DataReceivedEventArgs e) => {
                if (e.Data != null)
                {
                    if (sb.Length > 0)
                    {
                        sb.Append('\n');
                    }
                    sb.Append(e.Data.TrimEnd(new char[] {'\0'}).Trim());
                }
            };

            process.Start();
            process.BeginOutputReadLine();
            string err = process.StandardError.ReadToEnd();
            process.WaitForExit();

            if (sb.Length > 0)
            {
                sb.Append('\n');
            }

            int len = sb.Length;
            sb.Append(err.TrimEnd(new char[] { '\0' }).Trim());
            if (sb.Length > len)
            {
                sb.Append('\n');
            }

            Task.Run(() => { }).ContinueWith((prevResult) =>
            {
                log.AppendText(sb.ToString());
                log.ScrollToCaret();
            }, uiScheduler);
        }

    }
}
