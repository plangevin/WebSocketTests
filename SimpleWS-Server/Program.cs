using System.Net;
using System.Net.WebSockets;
using System.Text;
using System.Threading.Tasks.Dataflow;

var builder = WebApplication.CreateBuilder(args);
builder.WebHost.UseUrls("http://localhost:6969");
var app = builder.Build();
app.UseWebSockets();

var connections = new List<WebSocket>();

app.Map("/ws", async context =>
{
   if (context.WebSockets.IsWebSocketRequest)
   {
      var curName = context.Request.Query["name"];
      Console.WriteLine($"{curName} incoming connection");

      using var ws = await context.WebSockets.AcceptWebSocketAsync();
      connections.Add(ws);

      await Broadcast($"{curName} joined the chat room");
      await Broadcast($"{connections.Count} users connected");

      await ReceiveMessage(ws, async (result, buffer) =>
      {
         if (result.MessageType == WebSocketMessageType.Text)
         {
            var message = Encoding.UTF8.GetString(buffer, 0, buffer.Length);
            await Broadcast($"{curName}: {message}");
         }
         else if (result.MessageType == WebSocketMessageType.Close || ws.State == WebSocketState.Aborted)
         {
            connections.Remove(ws);
            await Broadcast($"{curName} has left the chat");
            await Broadcast($"{connections.Count} users connected");
            if (result.CloseStatus != null)
               await ws.CloseAsync(result.CloseStatus.Value, result.CloseStatusDescription, CancellationToken.None);
         }
      });
   }
   else
   {
      context.Response.StatusCode = (int)HttpStatusCode.BadRequest;
   }
});

async Task ReceiveMessage(WebSocket socket, Action<WebSocketReceiveResult, byte[]> handleMessage)
{
   var buffer = new byte[1024*4];
   while (socket.State == WebSocketState.Open)
   {
      var result = await socket.ReceiveAsync(new ArraySegment<byte>(buffer), CancellationToken.None);
      handleMessage(result, buffer);
   }
}

async Task Broadcast(string message)
{
   var bytes = Encoding.UTF8.GetBytes(message);
   foreach (var connection in connections)
   {
      if (connection.State == WebSocketState.Open)
      {
         var arraySegment = new ArraySegment<byte>(bytes, 0, bytes.Length);
         await connection.SendAsync(arraySegment, WebSocketMessageType.Text, true, CancellationToken.None);
      }
   }
}

await app.RunAsync();
