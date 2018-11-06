using System;
using System.Collections;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using UnityEngine;

public class TestUDPSender : MonoBehaviour
{
    public Transform Target;
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
            Vector3 pos = Target.position;
            float[] array = { pos.x, pos.y, pos.z };
            byte[] b = new byte[12];

            Buffer.BlockCopy(array, 0, b, 0, 12);
            client.Send(b, 12);
            print(b.Length);
    }

    void onDisable()
    {
        client.Close();
    }


}