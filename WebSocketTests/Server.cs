using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace WebSocketTests
{
   internal class Server
   {
      public static void Main()
      {
         var server = new TcpListener(IPAddress.Parse("127.0.0.1"), 80);

         server.Start();
         Console.WriteLine("Server has started on 127.0.0.1:80.{0}Waiting for a connection…", Environment.NewLine);

         var client = server.AcceptTcpClient();

         Console.WriteLine("A client connected.");

         var stream = client.GetStream();

         //enter to an infinite cycle to be able to handle every change in stream
         while (true) {
            while (!stream.DataAvailable);

            var bytes = new byte[client.Available];

            stream.Read(bytes, 0, bytes.Length);
         }
      }
   }
}
