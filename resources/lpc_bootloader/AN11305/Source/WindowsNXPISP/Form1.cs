/****************************************************************************
 *   $Id:: Form1.cs 4528 2010-08-24 23:32:22Z nxp21346                        $
 *   Project: NXP LPC134x USB ISP programmer example for Windows
 *
 *   Description:
 *     This file contains code to recognize LPC134x devices attached to a
 *     PC in ISP mode and reprogram them with new firmware. It depends on the
 *     UsbManager class from the iTuner open source project.
 *
 ****************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
****************************************************************************/
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using iTuner;

namespace NXPISP
{
    public partial class Form1 : Form
    {
        Firmware newFirmware;   // Used to read and store our new firmware
        private UsbManager manager; // UsbManager object from iTuner is used to discover disk information
        UsbStateChangedEventHandler diskCallback; // our UsbManager will call this function when a USB disk is inserted

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        // Handler for Select button. Upon click, give the user a chance to select a file and then validate it
        private void buttonSelect_Click(object sender, EventArgs e)
        {
            OpenFileDialog fileDialog = new OpenFileDialog();

            fileDialog.Filter = "firmware files (*.bin)|*.bin|All files (*.*)|*.*";
            // We always start in the Windows home directory. An enhancement would be to save the
            // last directory path and start there.
            fileDialog.InitialDirectory = System.Environment.GetFolderPath(Environment.SpecialFolder.Personal);
            fileDialog.ShowDialog();

            textBoxPath.Text = fileDialog.FileName;

            // Checks out the file the user selected and sees if it meets the "Criteria for valid user code"
            CheckPath();
        }

        private void CheckPath()
        {
            // Reset the UI. Disable "Update" button, don't show any red X or green check boxes
            labelPathCheck.Visible = false;
            labelPathX.Visible = false;
            labelStatusCheck.Visible = false;
            labelStatusX.Visible = false;
            checkBoxUpdate.Checked = false;
            checkBoxUpdate.Enabled = false;

            if (textBoxPath.Text.Length > 0)
            {
                newFirmware = new Firmware();
                try
                {
                    newFirmware.ReadFirmwareFromFile(textBoxPath.Text);
                    if (newFirmware.IsFirmwareVectorSumGood() == true)
                        // && newFirmware.IsFirmwareMod1024() == true)
                        // You may optionally check that the file length is mod 1024...
                    {
                        // The file checks out. Show the green checkmark and enable the Update button.
                        labelPathCheck.Visible = true;
                        checkBoxUpdate.Enabled = true;
                    }
                    else
                    {
                        MessageBox.Show(this, "Selected file does not appear to contain valid firmware.",
                            "Error Reading Firmware", MessageBoxButtons.OK, MessageBoxIcon.Stop);
                        throw new Exception();
                    }
                } 
                catch (Exception ex)
                {
                    // A file that does not look like firmware, or any random error will land us here.
                    // Show the red X
                    labelPathX.Visible = true;
                }
            }
        }

        // This member takes a UsbDisk class (from iTuner) with information about a
        // USB disk and updates the firmware on it.
        private void UpdateFirmwareFromDisk(UsbDisk disk)
        {
            // If there is no disk name it was a removal, not an insertion so we return
            if(disk.Name == null || disk.Name.Length == 0)
                return;
            // Create firmware.bin full path name
            string firmwareBinPath = disk.Name + "\\firmware.bin";

            // Check the disk model. Windows returns a processed string
            // with the vendor and the word "Device" in it
            if (disk.Model == "NXP LPC134X IFLASH USB Device" || disk.Model == "NXP LPC1XXX IFLASH USB Device")
            {
                // Check out the disk volume name and handle code protection
                if (disk.Volume == "CRP3 ENABLD")
                {
                    MessageBox.Show(this, "An NXP ISP device was found, but it cannot be reprogrammed due to its code protect settings.",
                        "Error Writing Firmware", MessageBoxButtons.OK, MessageBoxIcon.Stop);
                }

                if (disk.Volume == "CRP1 ENABLD"
                    || disk.Volume == "CRP2 ENABLD")
                {
                    System.IO.File.Delete(firmwareBinPath);
                    MessageBox.Show(this, "A code protected NXP ISP device was found. This device has been erased and it must be reset and reconnected to complete programming.",
                        "Error Writing Firmware", MessageBoxButtons.OK, MessageBoxIcon.Stop);
                }
                if (disk.Volume == "CRP DISABLD")
                {
                    UpdateFirmwareFromPath(firmwareBinPath);
                    // An enhancement here would be to unmount the disk for the user
                    MessageBox.Show(this, "Firmware has been updated. Please safely eject device " + disk.Name + ".",
                        "Successfully Updated Firmware", MessageBoxButtons.OK, MessageBoxIcon.Information);
                }
            }
        }

        private void UpdateFirmwareFromPath(string firmwareBinPath)
        {
            try
            {
                newFirmware.WriteFirmwareToFile(firmwareBinPath);

                labelStatusCheck.Visible = true;
                labelStatusX.Visible = false;
            }
            catch (Exception ex)
            {
                labelStatusCheck.Visible = false;
                labelStatusX.Visible = true;
            }
            checkBoxUpdate.Checked = false;
            if (diskCallback != null)
            {
                manager.StateChanged -= diskCallback;
                diskCallback = null;
            }
        }

        // Registered to be called by UsbManager when a new disk is inserted or removed
        private void DoStateChanged(UsbStateChangedEventArgs e)
        {
            UpdateFirmwareFromDisk(e.Disk);
        }

        // The update button is implemented as a checkbox, so that it can stay
        // visually depressed while the program is waiting for a USB device to be
        // plugged in.
        private void checkBoxUpdate_CheckedChanged(object sender, EventArgs e)
        {
            if(checkBoxUpdate.Checked == true)
            {
                // Clear X and Check for Update button
                labelStatusX.Visible = false;
                labelStatusCheck.Visible = false;

                // Make a UsbManager, used to get information on USB disks
                manager = new UsbManager();
                UsbDiskCollection disks = manager.GetAvailableDisks();

                // Scan existing USB disks
                foreach (UsbDisk disk in disks)
                    UpdateFirmwareFromDisk(disk);

                // If we did not update firmware yet then set up event handler and wait
                // for insertion
                if (checkBoxUpdate.Checked == true)
                {
                    if (diskCallback == null)
                        diskCallback = new UsbStateChangedEventHandler(DoStateChanged);
                    manager.StateChanged += diskCallback;
                }
            } else
            {
                if(diskCallback != null)
                {
                    manager.StateChanged -= diskCallback;
                    diskCallback = null;
                }
            }
        }

        private void textBoxPath_TextChanged(object sender, EventArgs e)
        {
            // validate path and set red/green X/check
            CheckPath();
        }
    }

    // Class to read firmware and check "Criteria for Valid User Code"
    public class Firmware
    {
        byte[] firmwareData;

        public void ReadFirmwareFromFile(string filePath)
        {
            firmwareData = System.IO.File.ReadAllBytes(filePath);
        }
        public void WriteFirmwareToFile(string filePath)
        {
            System.IO.File.WriteAllBytes(filePath, firmwareData);
        }
        public byte[] GetFirmware()
        {
            return firmwareData;
        }
        public void SetFirmware(byte[] newFirmwareData)
        {
            firmwareData = newFirmwareData;
        }
        public int GetFirmwareSize()
        {
            return firmwareData.Length;
        }
        public UInt32 BytesToLong(byte b0, byte b1, byte b2, byte b3)
        {
            return (UInt32)b0 | (UInt32)b1 << 8 | (UInt32)b2 << 16 | (UInt32)b3 << 24;
        }
        public bool IsFirmwareVectorSumGood()
        {
            UInt32[] vectors = new UInt32[8];
            UInt32 vectorSum = 0;
            int i;

            for (i = 0; i < 8; i++)
            {
                vectorSum += BytesToLong(
                    firmwareData[i * 4],
                    firmwareData[i * 4 + 1],
                    firmwareData[i * 4 + 2],
                    firmwareData[i * 4 + 3]);
            }

            if (vectorSum == 0)
                return true;
            return false;
        }
        public bool IsFirmwareMod1024()
        {
            if ((firmwareData.Length % 1024) == 0)
                return true;
            return false;
        }
    }
}
