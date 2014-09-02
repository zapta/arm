//************************************************************************************************
// Copyright © 2010 Steven M. Cohn. All Rights Reserved.
//
//************************************************************************************************

namespace iTuner
{
	using System;
	using System.Management;
	using System.Runtime.InteropServices;
	using System.Windows.Forms;


	/// <summary>
	/// Discover USB disk devices and monitor for device state changes.
	/// </summary>

	public class UsbManager : IDisposable
	{

		#region DriverWindow

		/// <summary>
		/// A native window used to monitor all device activity.
		/// </summary>

		private class DriverWindow : NativeWindow, IDisposable
		{
			// Contains information about a logical volume.
			[StructLayout(LayoutKind.Sequential)]
			public struct DEV_BROADCAST_VOLUME
			{
				public int dbcv_size;			// size of the struct
				public int dbcv_devicetype;		// DBT_DEVTYP_VOLUME
				public int dbcv_reserved;		// reserved; do not use
				public int dbcv_unitmask;		// Bit 0=A, bit 1=B, and so on (bitmask)
				public short dbcv_flags;		// DBTF_MEDIA=0x01, DBTF_NET=0x02 (bitmask)
			}


			private const int WM_DEVICECHANGE = 0x0219;				// device state change
			private const int DBT_DEVICEARRIVAL = 0x8000;			// detected a new device
			private const int DBT_DEVICEQUERYREMOVE = 0x8001;		// preparing to remove
			private const int DBT_DEVICEREMOVECOMPLETE = 0x8004;	// removed 
			private const int DBT_DEVTYP_VOLUME = 0x00000002;		// logical volume


			public DriverWindow ()
			{
				// create a generic window with no class name
				base.CreateHandle(new CreateParams());
			}


			public void Dispose ()
			{
				base.DestroyHandle();
				GC.SuppressFinalize(this);
			}


			public event UsbStateChangedEventHandler StateChanged;


			protected override void WndProc (ref Message message)
			{
				base.WndProc(ref message);

				if ((message.Msg == WM_DEVICECHANGE) && (message.LParam != IntPtr.Zero))
				{
					DEV_BROADCAST_VOLUME volume = (DEV_BROADCAST_VOLUME)Marshal.PtrToStructure(
						message.LParam, typeof(DEV_BROADCAST_VOLUME));

					if (volume.dbcv_devicetype == DBT_DEVTYP_VOLUME)
					{
						switch (message.WParam.ToInt32())
						{
							case DBT_DEVICEARRIVAL:
								SignalDeviceChange(UsbStateChange.Added, volume);
								break;

							case DBT_DEVICEQUERYREMOVE:
								// can intercept
								break;

							case DBT_DEVICEREMOVECOMPLETE:
								SignalDeviceChange(UsbStateChange.Removed, volume);
								break;
						}
					}
				}
			}


			private void SignalDeviceChange (UsbStateChange state, DEV_BROADCAST_VOLUME volume)
			{
				string name = ToUnitName(volume.dbcv_unitmask);

				if (StateChanged != null)
				{
					UsbDisk disk = new UsbDisk(name);
					StateChanged(new UsbStateChangedEventArgs(state, disk));
				}
			}


			/// <summary>
			/// Translate the dbcv_unitmask bitmask to a drive letter by finding the first
			/// enabled low-order bit; its offset equals the letter where offset 0 is 'A'.
			/// </summary>
			/// <param name="mask"></param>
			/// <returns></returns>

			private string ToUnitName (int mask)
			{
				int offset = 0;
				while ((offset < 26) && ((mask & 0x00000001) == 0))
				{
					mask = mask >> 1;
					offset++;
				}

				if (offset < 26)
				{
					return String.Format("{0}:", Convert.ToChar(Convert.ToInt32('A') + offset));
				}

				return "?:";
			}
		}

		#endregion WndProc Driver


		private delegate void GetDiskInformationDelegate (UsbDisk disk);


		private DriverWindow window;
		private UsbStateChangedEventHandler handler;
		private bool isDisposed;


		//========================================================================================
		// Constructor
		//========================================================================================

		/// <summary>
		/// Initialize a new instance.
		/// </summary>

		public UsbManager ()
		{
			this.window = null;
			this.handler = null;
			this.isDisposed = false;
		}


		#region Lifecycle

		/// <summary>
		/// Destructor.
		/// </summary>

		~UsbManager ()
		{
			Dispose();
		}


		/// <summary>
		/// Must shutdown the driver window.
		/// </summary>

		public void Dispose ()
		{
			if (!isDisposed)
			{
				if (window != null)
				{
					window.StateChanged -= new UsbStateChangedEventHandler(DoStateChanged);
					window.Dispose();
					window = null;
				}

				isDisposed = true;

				GC.SuppressFinalize(this);
			}
		}

		#endregion Lifecycle


		//========================================================================================
		// Events/Properties
		//========================================================================================

		/// <summary>
		/// Add or remove a handler to listen for USB disk drive state changes.
		/// </summary>

		public event UsbStateChangedEventHandler StateChanged
		{
			add
			{
				if (window == null)
				{
					// create the driver window once a consumer registers for notifications
					window = new DriverWindow();
					window.StateChanged += new UsbStateChangedEventHandler(DoStateChanged);
				}

				handler = (UsbStateChangedEventHandler)Delegate.Combine(handler, value);
			}

			remove
			{
				handler = (UsbStateChangedEventHandler)Delegate.Remove(handler, value);

				if (handler == null)
				{
					// destroy the driver window once the consumer stops listening
					window.StateChanged -= new UsbStateChangedEventHandler(DoStateChanged);
					window.Dispose();
					window = null;
				}
			}
		}


		//========================================================================================
		// Methods
		//========================================================================================

		/// <summary>
		/// Gets a collection of all available USB disk drives currently mounted.
		/// </summary>
		/// <returns>
		/// A UsbDiskCollection containing the USB disk drives.
		/// </returns>

		public UsbDiskCollection GetAvailableDisks ()
		{
			UsbDiskCollection disks = new UsbDiskCollection();

			// browse all USB WMI physical disks
			foreach (ManagementObject drive in
				new ManagementObjectSearcher(
					"select DeviceID, Model from Win32_DiskDrive where InterfaceType='USB'").Get())
			{
				// associate physical disks with partitions
				ManagementObject partition = new ManagementObjectSearcher(String.Format(
					"associators of {{Win32_DiskDrive.DeviceID='{0}'}} where AssocClass = Win32_DiskDriveToDiskPartition",
					drive["DeviceID"])).First();

				if (partition != null)
				{
					// associate partitions with logical disks (drive letter volumes)
					ManagementObject logical = new ManagementObjectSearcher(String.Format(
						"associators of {{Win32_DiskPartition.DeviceID='{0}'}} where AssocClass = Win32_LogicalDiskToPartition",
						partition["DeviceID"])).First();

					if (logical != null)
					{
						// finally find the logical disk entry to determine the volume name
						ManagementObject volume = new ManagementObjectSearcher(String.Format(
							"select FreeSpace, Size, VolumeName from Win32_LogicalDisk where Name='{0}'",
							logical["Name"])).First();

						UsbDisk disk = new UsbDisk(logical["Name"].ToString());
						disk.Model = drive["Model"].ToString();
						disk.Volume = volume["VolumeName"].ToString();
						disk.FreeSpace = (ulong)volume["FreeSpace"];
						disk.Size = (ulong)volume["Size"];

						disks.Add(disk);
					}
				}
			}

			return disks;
		}


		/// <summary>
		/// Internally handle state changes and notify listeners.
		/// </summary>
		/// <param name="e"></param>

		private void DoStateChanged (UsbStateChangedEventArgs e)
		{
			if (handler != null)
			{
				UsbDisk disk = e.Disk;

				// we can only interrogate drives that are added...
				// cannot see something that is no longer there!

				if ((e.State == UsbStateChange.Added) && (e.Disk.Name[0] != '?'))
				{
					// the following Begin/End invokes looks strange but are required
					// to resolve a "DisconnectedContext was detected" exception which
					// occurs when the current thread terminates before the WMI queries
					// can complete.  I'm not exactly sure why that would happen...

					GetDiskInformationDelegate gdi = new GetDiskInformationDelegate(GetDiskInformation);
					IAsyncResult result = gdi.BeginInvoke(e.Disk, null, null);
					gdi.EndInvoke(result);
				}

				handler(e);
			}
		}


		/// <summary>
		/// Populate the missing properties of the given disk before sending to listeners
		/// </summary>
		/// <param name="disk"></param>

		private void GetDiskInformation (UsbDisk disk)
		{
			ManagementObject partition = new ManagementObjectSearcher(String.Format(
				"associators of {{Win32_LogicalDisk.DeviceID='{0}'}} where AssocClass = Win32_LogicalDiskToPartition",
				disk.Name)).First();

			if (partition != null)
			{
				ManagementObject drive = new ManagementObjectSearcher(String.Format(
					"associators of {{Win32_DiskPartition.DeviceID='{0}'}}  where resultClass = Win32_DiskDrive",
					partition["DeviceID"])).First();

				if (drive != null)
				{
					disk.Model = drive["Model"].ToString();
				}

				ManagementObject volume = new ManagementObjectSearcher(String.Format(
					"select FreeSpace, Size, VolumeName from Win32_LogicalDisk where Name='{0}'",
					disk.Name)).First();

				if (volume != null)
				{
					disk.Volume = volume["VolumeName"].ToString();
					disk.FreeSpace = (ulong)volume["FreeSpace"];
					disk.Size = (ulong)volume["Size"];
				}
			}
		}
	}
}
