using System;
using System.Collections;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using UnityEngine;

public class UDPTouchSender : MonoBehaviour
{
    public string host = "127.0.0.1";
    public int port = 9998;
    private UdpClient client;
    // Use this for initialization
    void Start()
    {
        client = new UdpClient();
        client.Connect(host, port);
    }

    // Update is called once per frame
    void Update()
    {
        // if (OVRInput.GetControllerPositionTracked(OVRInput.Controller.LTouch))
        // {
        //     Vector3 pos = 100f * OVRInput.GetLocalControllerPosition(OVRInput.Controller.LTouch);
        //     float[] array = { pos.x, pos.y, pos.z };
        //     byte[] b = new byte[12];
        //     Buffer.BlockCopy(array, 0, b, 0, 12);
        //     client.Send(b, 12);
        // }
    }

    void onDisable()
    {
        client.Close();
    }
}
