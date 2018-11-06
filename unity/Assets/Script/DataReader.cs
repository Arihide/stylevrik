
using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Threading;
using System.Net;
using System.Net.Sockets;
using UnityEngine;

/// <summary>
/// Header format of BVH data
/// </summary>
[StructLayout(LayoutKind.Sequential, Pack = 1)]
// public struct BvhDataHeader
// {
//     public ushort Token1;               // Package start token: 0xDDFF
//     public DataVersion DataVersion;     // Version of community data format: 1.1.0.0
//     public ushort DataCount;            // Values count
//     public byte bWithDisp;              // With/out displacement
//     public byte bWithReference;         // With/out reference bone data at first
//     public uint AvatarIndex;            // Avatar index
//     [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
//     public string AvatarName;           // Avatar name
//     public uint FrameIndex;             // Frame data index
//     public uint Reserved;               // Reserved, only enable this package has 64bytes length
//     public uint Reserved1;              // Reserved, only enable this package has 64bytes length
//     public uint Reserved2;              // Reserved, only enable this package has 64bytes length
//     public ushort Token2;               // Package end token: 0xEEFF
// };

class UDPSession
{
    Thread receivingThread = null;
    bool exitFlag = false;
    byte[] recvBuffer = null;
    byte[] reserveBuffer = null;
    int reserveSize = 0;
    int recvBufferSize = 2048;
    int reserveBufferSize = 4096;
    int headerSize = 64;
    IntPtr pinnedHeader = IntPtr.Zero;
    IntPtr pinnedData = IntPtr.Zero;
    int dataSize = 0;
    Socket socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
    IPEndPoint sourceAddress = new IPEndPoint(IPAddress.Any, 0);
    public UDPSession(int recvBufferSize)
    {
        this.recvBufferSize = recvBufferSize;
        recvBuffer = new byte[recvBufferSize];
    }

    public void Start()
    {
        receivingThread = new Thread(new ThreadStart(DoReceive));
        receivingThread.IsBackground = true;
        receivingThread.Start();
    }
    public void Stop()
    {
        // ReleaseHeader();
        ReleaseData();

        exitFlag = true;
        while (receivingThread.IsAlive) ;
    }

    public void Recieve(byte[] buffer, int size, out int receivedSize, int recvWaitTime = 0)
    {
        {
            EndPoint remote = new IPEndPoint(IPAddress.Any, 0);
            try
            {
                receivedSize = socket.ReceiveFrom(buffer, size, SocketFlags.None, ref remote);
                if (sourceAddress.Address != IPAddress.Any && sourceAddress.Address != ((IPEndPoint)remote).Address)
                {
                    receivedSize = 0;
                }
            }
            catch (Exception e)
            {
                receivedSize = 0;
                Debug.LogException(e);
            }
        }

    }
    public void DoReceive()
    {

        while (!exitFlag)
        {
            int recvSize = 0;
            Recieve(recvBuffer, recvBufferSize, out recvSize, 0);

            if (reserveBuffer == null)
            {
                reserveBuffer = new byte[reserveBufferSize];
            }

            Buffer.BlockCopy(recvBuffer, 0, reserveBuffer, reserveSize, recvSize);
            reserveSize += recvSize;

            // int packetSize = headerSize + 32 * sizeof(float);
            int packetSize = 32 * sizeof(float);
            // Debug.LogFormat( "reserveSize {0} >= {1} packetSize", reserveSize, packetSize );
            // if (reserveSize >= packetSize)
            {
                //Debug.LogWarningFormat( "received packet received size = {0}", reserveSize );
                // if (NeuronDataReader.frameDataReceivedCallback != null)
                {
                    int requiredSize = 32 * sizeof(float);
                    if (pinnedHeader == IntPtr.Zero || dataSize < requiredSize)
                    {
                        ReleaseData();
                        pinnedData = Marshal.AllocHGlobal(requiredSize);
                        dataSize = requiredSize;
                    }

                    Marshal.Copy(reserveBuffer, 0, pinnedData, requiredSize);

                    float[] f_data = new float[1000];

                    Marshal.Copy(pinnedData, f_data, 0, 640);
                    Debug.Log(f_data[0]);
                    Debug.Log(f_data[1]);
                    Debug.Log(f_data[2]);
                    // NeuronDataReader.frameDataReceivedCallback(IntPtr.Zero, GetHashValue(), pinnedHeader, pinnedData);
                }

                // clean reserveBuffer
                Buffer.BlockCopy(reserveBuffer, packetSize, reserveBuffer, 0, reserveSize - packetSize);
                reserveSize -= packetSize;
            }
        }

    }

    // void AcquireHeader(byte[] buffer)
    // {
    //     if (pinnedHeader == IntPtr.Zero)
    //     {
    //         pinnedHeader = Marshal.AllocHGlobal(headerSize);
    //     }

    //     Marshal.Copy(buffer, 0, pinnedHeader, headerSize);
    //     return (BvhDataHeader)Marshal.PtrToStructure(pinnedHeader, typeof(BvhDataHeader));
    // }

    // void AcquireData(byte[] buffer, BvhDataHeader header)
    // {
    //     int requiredSize = (int)header.DataCount * sizeof(float);
    //     if (pinnedHeader == IntPtr.Zero || dataSize < requiredSize)
    //     {
    //         ReleaseData();
    //         pinnedData = Marshal.AllocHGlobal(requiredSize);
    //         dataSize = requiredSize;
    //     }

    //     Marshal.Copy(buffer, 64, pinnedData, requiredSize);
    // }
    void ReleaseData()
    {
        if (pinnedData != IntPtr.Zero)
        {
            Marshal.FreeHGlobal(pinnedData);
            pinnedData = IntPtr.Zero;
        }
    }

    public bool Connect(int port)
    {

        socket.Bind(new IPEndPoint(IPAddress.Any, port));
        // hashValue = GetHashValue("0.0.0.0", port);
        Start();
        return true;
    }

    public void Disconnect()
    {
        socket.Close();
        Stop();
    }

}

public class DataReader
{
    static int MaxPacketSize = 64 + 60 * 6 * sizeof(float);

    static UDPSession session;

    static Dictionary<IntPtr, UDPSession> sessions = new Dictionary<IntPtr, UDPSession>();
    // public static FrameDataReceived frameDataReceivedCallback;
    // public static SocketStatusChanged socketStatusChangedCallback;

    // public static void BRRegisterFrameDataCallback(IntPtr customedObj, FrameDataReceived handle)
    // {
    //     frameDataReceivedCallback = handle;
    // }

    // public static void BRRegisterSocketStatusCallback(IntPtr customedObj, SocketStatusChanged handle)
    // {
    //     socketStatusChangedCallback = handle;
    // }

    public static IntPtr BRStartUDPServiceAt(int nPort)
    {
        session = new UDPSession(MaxPacketSize);
        if (session.Connect(nPort))
        {
            // sessions.Add(Session.GetHashValue("0.0.0.0", nPort), newSession);
            // return newSession.GetHashValue();
        }

        return IntPtr.Zero;
    }

    public static void BRCloseSocket()
    {
        // UDPSession session = null;
        // if (sessions.TryGetValue(sockRef, out session))
        {
            session.Disconnect();
        }

        // sessions.Remove(sockRef);
    }

}