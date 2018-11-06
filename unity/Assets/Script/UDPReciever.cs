using UnityEngine;
using System.Collections;
using System.Net;
using System.Net.Sockets;
using System.Threading;

public class UDPReciever : MonoBehaviour {

    static UdpClient udp;
    Thread thread;

	// Use this for initialization
	void Start () {
        string localIpString = "127.0.0.1";
        IPAddress localAddress = IPAddress.Parse(localIpString);
        int localPort = 9999;

        //UdpClientを作成し、ローカルエンドポイントにバインドする
        IPEndPoint localEP = new IPEndPoint(localAddress, localPort);
        udp = new UdpClient(localEP);
        //udp.Client.ReceiveTimeout = 1000;
        thread = new Thread(new ThreadStart(ThreadMethod));
        thread.Start();
	}
	
	// Update is called once per frame
	void Update () {
		
	}

    private void OnApplicationQuit()
    {
        udp.Close();
        thread.Abort();
    }

    private static void ThreadMethod()
    {
        while (true)
        {
            IPEndPoint remoteEP = null;
            byte[] data = udp.Receive(ref remoteEP);
            Debug.Log(data);
        }
    }

}
