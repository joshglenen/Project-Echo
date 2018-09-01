using CSCore.CoreAudioAPI;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Timers;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.ComponentModel;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using AudioSwitcher.AudioApi.CoreAudio;

namespace DRWatchdogV2
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        //reads media, could condense and clean
        private static AudioSessionManager2 GetDefaultAudioSessionManager2(DataFlow dataFlow)
        {

            using (var enumerator = new MMDeviceEnumerator())
            {
                using (var device = enumerator.GetDefaultAudioEndpoint(dataFlow, Role.Multimedia))
                {
                    //Debug.WriteLine("DefaultDevice: " + device.FriendlyName);
                    var sessionManager = AudioSessionManager2.FromMMDevice(device);
                    return sessionManager;
                }
            }
        }
        private void SetMediaManager()
        {
            try
            {

                using (var sessionManager = GetDefaultAudioSessionManager2(DataFlow.Render))
                {
                    using (var sessionEnumerator = sessionManager.GetSessionEnumerator())
                    {
                        foreach (var session in sessionEnumerator)
                        {
                            if (session.QueryInterface<AudioMeterInformation>() != null)
                            {
                                using (var myMedia = session.QueryInterface<AudioMeterInformation>())
                                {
                                    buffer = myMedia.GetPeakValue();
                                    //Console.WriteLine(buffer);
                                    try
                                    {
                                        this.Dispatcher.Invoke(() =>
                                        {

                                            Emu.Value = buffer;
                                        });
                                    }
                                    catch (Exception e)
                                    {
                                        Debug.WriteLine(e.ToString());
                                    }

                                }
                            }
                        }
                    }
                }
            }
            catch(Exception e)
            {
                Debug.WriteLine(e.ToString());
            }
        }
        private void RunMTAThread()
        {
            Thread t = new Thread(SetMediaManager);
            t.SetApartmentState(ApartmentState.MTA);
            t.Start();
        }

        //checks and changes volume
        private void checkVol(float val)
        {
            if ((!volLowered) && (val >= UpThresh)) { volLowered = !volLowered; changeVol = !changeVol;
            }
            else if ((volLowered) && (val <= DownThresh)) { volLowered = !volLowered; changeVol = !changeVol;
            }
        }
        private void raiseVol()
        {
            VolumeMixer.SetVol(Convert.ToInt32(defaultVol));
        }
        private void lowerVol()
        {
            VolumeMixer.SetVol(Convert.ToInt32(defaultVol * percentMod));
        }

        //static and private variables
        private float buffer = 0;
        private bool changeVol = false;
        private bool volLowered = false;
        private System.Timers.Timer myTimer;
        public int timerInterval = 50;

        //dynamic variables
        public double UpThresh = 0.80;
        public double DownThresh = 0.40;
        public double percentMod = 0.50;
        public double attackVal = 10;
        public double releaseVal = 10;
        public static int defaultVol = 2;

        //configure fundamental classes
        private void SetProgressBar()
        {
            Emu.Minimum = 0;
            Emu.Maximum = 1;
        }
        private void SetTimer()
        {
            myTimer = new System.Timers.Timer(timerInterval);
            myTimer.Elapsed += OnTimedEvent;
            myTimer.AutoReset = true;

        }
        private void OnTimedEvent(Object source, ElapsedEventArgs e)
        {
            RunMTAThread();
            Dispatcher.BeginInvoke(
            new ThreadStart(() => checkVol(buffer)));
            if (changeVol)
            {
                myTimer.Enabled = false;
                changeVol = !changeVol;
                //note: volLowered has already been toggled
                if (!volLowered) { raiseVol(); myTimer.Interval = releaseVal;
                }
                else { lowerVol(); myTimer.Interval = attackVal;
                }
                myTimer.Enabled = true;
            }
            else  myTimer.Interval = timerInterval;
        }

        //GUI
        public void RunProg_Click(object sender, RoutedEventArgs e)
        {
            Reset();
            //TODO
            myTimer.Enabled = true;
        }
        private void Button_Click(object sender, RoutedEventArgs e)
        {
            Reset();
            //TODO
        }
        private void Reset()
        {
            DownThresh = Convert.ToDouble(LO.Text);
            UpThresh = Convert.ToDouble(UP.Text);
            defaultVol = Convert.ToInt32(DV.Text);
            percentMod = Convert.ToDouble(PR.Text);
            attackVal = Convert.ToDouble(AT.Text);
            releaseVal = Convert.ToDouble(RE.Text);
            attackVal = Convert.ToDouble(AT.Text);
        }

        public MainWindow()
        {
            InitializeComponent();
            SetTimer();
            SetProgressBar();
            VolumeMixer.SetVol(defaultVol);
        }

    }
}
