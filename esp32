#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>

const char* ssid = "OfflineChat";
const char* password = "12345678";

WebServer server(80);
String chatMessages = "";

void setup() {
  Serial.begin(115200);
  
  if(!SPIFFS.begin(true)){
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  
  WiFi.softAP(ssid, password);
  
  Serial.println("Access Point Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/send", handleSend);
  server.on("/messages", handleMessages);
  
  server.begin();
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  const char* html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Offline Chat</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            font-family: 'Arial', sans-serif;
        }
        
        body {
            background-color: #f0f2f5;
            display: flex;
            flex-direction: column;
            height: 100vh;
            padding: 20px;
        }
        
        .chat-container {
            background: white;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            flex-grow: 1;
            display: flex;
            flex-direction: column;
            max-width: 800px;
            margin: 0 auto;
            width: 100%;
        }
        
        .chat-header {
            background: #075E54;
            color: white;
            padding: 15px;
            border-radius: 10px 10px 0 0;
        }
        
        #messages {
            flex-grow: 1;
            overflow-y: auto;
            padding: 20px;
            display: flex;
            flex-direction: column;
            gap: 10px;
            white-space: pre-wrap;
        }
        
        .message {
            background: #E8F5E9;
            padding: 10px 15px;
            border-radius: 15px;
            max-width: 80%;
            word-wrap: break-word;
            margin: 5px 0;
            align-self: flex-start;
        }
        
        .input-container {
            display: flex;
            padding: 15px;
            gap: 10px;
            background: white;
            border-top: 1px solid #eee;
            border-radius: 0 0 10px 10px;
        }
        
        #messageInput {
            flex-grow: 1;
            padding: 10px;
            border: 1px solid #ddd;
            border-radius: 20px;
            outline: none;
            font-size: 16px;
        }
        
        #sendButton {
            background: #128C7E;
            color: white;
            border: none;
            border-radius: 20px;
            padding: 10px 20px;
            cursor: pointer;
            font-size: 16px;
            transition: background 0.3s;
        }
        
        #sendButton:hover {
            background: #075E54;
        }
        
        @media (max-width: 480px) {
            body {
                padding: 10px;
            }
            .message {
                max-width: 90%;
            }
        }
    </style>
</head>
<body>
    <div class="chat-container">
        <div class="chat-header">
            <h2>Offline Chat</h2>
        </div>
        <div id="messages"></div>
        <div class="input-container">
            <input type="text" id="messageInput" placeholder="พิมพ์ข้อความ..." autocomplete="off">
            <button id="sendButton" onclick="sendMessage()">ส่ง</button>
        </div>
    </div>
    
    <script>
        const messagesDiv = document.getElementById('messages');
        const messageInput = document.getElementById('messageInput');
        
        function sendMessage() {
            const message = messageInput.value.trim();
            if(message) {
                fetch('/send?message=' + encodeURIComponent(message))
                    .then(response => response.text())
                    .then(data => {
                        messageInput.value = "";
                        updateMessages();
                    });
            }
        }
        
        messageInput.addEventListener('keypress', function(e) {
            if (e.key === 'Enter') {
                sendMessage();
            }
        });
        
        function updateMessages() {
            fetch('/messages')
                .then(response => response.text())
                .then(data => {
                    const messages = data.split('\\n').filter(msg => msg.trim());
                    const htmlMessages = messages.map(msg => 
                        `<div class="message">${msg}</div>`
                    ).join('\\n');
                    messagesDiv.innerHTML = htmlMessages;
                    messagesDiv.scrollTop = messagesDiv.scrollHeight;
                });
        }
        
        setInterval(updateMessages, 1000);
        updateMessages();
    </script>
</body>
</html>
)rawliteral";
  
  server.send(200, "text/html", html);
}

void handleSend() {
  if(server.hasArg("message")) {
    String message = server.arg("message");
    String clientIP = server.client().remoteIP().toString();
    chatMessages += clientIP + ": " + message + "\n\n";  // เพิ่มการขึ้นบรรทัดใหม่สองครั้ง
    server.send(200, "text/plain", "Message sent");
  }
}

void handleMessages() {
  server.send(200, "text/plain", chatMessages);
}
