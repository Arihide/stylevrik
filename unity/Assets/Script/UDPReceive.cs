//using UnityEngine;
//using System.Collections;
//using System.Net;
//using System.Net.Sockets;
//using System.Text;
//using System.Threading;  
//using System.Collections.Generic;

//public struct motionData
//{
//    public int ID;
//    public Vector3 acceleration;
//    public Vector3 magne;
//    public Vector3 gyro;
//    public float pressure;
//    public float timeStamp;
//}

//public class UDPReceive : MonoBehaviour
//{
//    public Queue<motionData> dataList;

//    /*--書き込み時の動機を取るためのもの-------------*/
//    public object syncHead = new object();
//    public object syncRightArm = new object();
//    public object syncLeftArm = new object();

//    /*--各モーションデータはここに格納する-----------*/
//    /*--他のスクリプトはこの変数からデータを取得する--*/
//    public motionData headMotionData;
//    public motionData rightArmMotionData;
//    public motionData leftArmMotionData;

//    private int count = 0;

//    private int LOCA_LPORT = 4444;
//    private UdpClient udp;
//    Thread thread;
//    bool threadEnd;

//    bool threadFlag = true;
//    bool receiveFlag = true;
//    bool sendEndFlag = false;
//    bool receiveStart = false;

//    private string host;
//    private UdpClient tmpUdp;
//    private byte[] tmpSendMsg;

//    private DataRecScript dataRec;

//    void Start()
//    {
//        dataList = new Queue<motionData>();
//        udp = new UdpClient(LOCA_LPORT);
//        udp.Client.ReceiveTimeout = 300;
//        thread = new Thread(new ThreadStart(ThreadMethod));
//        thread.Start();
//        threadFlag = true;
//        threadEnd = false;
//        receiveFlag = true;
//        sendEndFlag = false;
//        receiveStart = false;
//        count = 0;

//        //      host = "192.168.1.70";
//        //      tmpUdp = new UdpClient();
//        //      tmpUdp.Connect(host, LOCA_LPORT);
//        //tmpSendMsg = Encoding.UTF8.GetBytes("0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0");    

//        receiveStart = true;

//        dataRec = GameObject.Find("DataPlayer").GetComponent<DataRecScript>();

//    }

//    void Update()
//    {
        
//        if (!receiveFlag && !sendEndFlag)
//        {
//            Debug.Log("send stop signal");
//            tmpUdp.Send(tmpSendMsg, tmpSendMsg.Length);
//            Debug.Log("closing");
//        }
//    }

//    void OnApplicationQuit()
//    {
//        udp.Close();
//        //tmpUdp.Close();
//        threadFlag = false;

//        while (!threadEnd) ;
//        thread.Abort();
//    }

//    private void ThreadMethod()
//    {
//        Debug.Log("thread start");
//        while (threadFlag)
//        {
//            if (receiveStart)
//            {
//                count++;
//                IPEndPoint remoteEP = null;
//                string text = null; ;
//                if (receiveFlag)
//                {
//					try{
//                    byte[] data = udp.Receive(ref remoteEP);
//					text = Encoding.ASCII.GetString(data);
//                    //Debug.Log("data: "+text);
//					}
//					catch(System.Exception e ){
//						Debug.Log(e.ToString());
//					}
//				}
//				else
//                {
//                    sendEndFlag = true;
//                }
//                if (text != null && receiveFlag)
//                {
//                    transMotionData(text);
//                    dataRec.saveData.Add(dataRec.tRec.ElapsedMilliseconds.ToString() + " " + text + "\n");
//                }
//                else
//                {
//                    Debug.Log("receive fault");
//                }
//            }
//        }
//        threadEnd = true;
//    }

//    float changeFloat(string num)
//    {
//        if (string.Compare(num, "nan") == 0) 
//        {
//            return 0.0f;
//        }
//        else
//        {
//            return float.Parse(num);
//        }
//    }

//    public void transMotionData(string text)
//    {
//        motionData tmpData = new motionData();
//        // カンマ区切りで分割して配列に格納する
//        string[] splitData = text.Split(' ');
//        if (splitData.Length > 2)
//        {
//            tmpData.ID = -1;
//            if (splitData[0].Equals("LAPIS_RAW0"))
//            {
//                tmpData.ID = 0;
//            }
//            else if (splitData[0].Equals("LAPIS_RAW1"))
//            {
//                tmpData.ID = 1;
//            }
//            else if (splitData[0].Equals("LAPIS_RAW2"))
//            {
//                tmpData.ID = 2;
//            }
//            else
//            {
//                Debug.Log("irregal id" + splitData[0]);
//            }
//            tmpData.timeStamp = changeFloat(splitData[1]);
//            tmpData.acceleration.x = changeFloat(splitData[2]);
//            tmpData.acceleration.y = changeFloat(splitData[3]);
//            tmpData.acceleration.z = changeFloat(splitData[4]);
//            tmpData.magne.x = changeFloat(splitData[5]);
//            tmpData.magne.y = changeFloat(splitData[6]);
//            tmpData.magne.z = changeFloat(splitData[7]);
//            tmpData.gyro.x = changeFloat(splitData[8]);
//            tmpData.gyro.y = changeFloat(splitData[9]);
//            tmpData.gyro.z = changeFloat(splitData[10]);
//            tmpData.pressure = changeFloat(splitData[11]);
//            switch (tmpData.ID)
//            {
//                case 0:
//                    lock (syncHead)
//                    {
//                        headMotionData = tmpData;
//                    }
//                    break;
//                case 1:
//                    lock (syncRightArm)
//                    {
//                        rightArmMotionData = tmpData;
//                    }
//                    break;
//                case 2:
//                    lock (syncLeftArm)
//                    {
//                        leftArmMotionData = tmpData;
//                    }
//                    break;
//                default:
//                    Debug.Log("irregal id");
//                    break;

//            }
//            /*
//            Debug.Log("data receive:" + "ID:" + tmpData.ID +
//                ":accel:" + tmpData.acceleration +
//                ":magne:" + tmpData.magne +
//                ":gyro:" + tmpData.gyro +
//                ":pres:" + tmpData.pressure +
//                ":time:" + tmpData.timeStamp);
//             */
//        }
//        else
//        {
//            Debug.Log("irregal text" + text);
//        }
//    }
//    public void initData(int ID)
//    {
//        motionData tmpData = new motionData();
//        tmpData.ID = ID;
//        tmpData.timeStamp = 0;
//        tmpData.acceleration.x = 0;
//        tmpData.acceleration.y = 0;
//        tmpData.acceleration.z = 0;
//        tmpData.magne.x = 0;
//        tmpData.magne.y = 0;
//        tmpData.magne.z = 0;
//        tmpData.gyro.x = 0;
//        tmpData.gyro.y = 0;
//        tmpData.gyro.z = 0;
//        tmpData.pressure = 0;

//        switch (ID)
//        {
//            case 0:
//                headMotionData = tmpData;
//                break;
//            case 1:
//                rightArmMotionData = tmpData;
//                break;
//            case 2:
//                leftArmMotionData = tmpData;
//                break;
//            default:
//                break;
//        }
//    }

//}


