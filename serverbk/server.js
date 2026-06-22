require('dotenv').config()
var express = require('express');
var cookieParser = require('cookie-parser');
var bodyParser = require('body-parser')
var cors = require('cors')
const multer = require('multer');
const http = require('http');
const { WebSocketServer } = require('ws');


var app = express();
const port = process.env.PORT || 3000;

app.use(cors());
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));
app.use(multer().any());
app.use(cookieParser());
global.reqlib = require('app-root-path').require;
app.use('/api', require('./controllers')());
app.use('/avatar',express.static('./images/avatar'));

const server = http.createServer(app);
const wss = new WebSocketServer({ server });

const clients = new Map();

global.wsClients = clients;

server.listen(port, () => {
    console.log(`Server listening on port ${port}`);
});

wss.on('connection', (ws) => {
    console.log('New WS connection');

    ws.on('message', (data) => {
        try {
            const msg = JSON.parse(data);

            // Đăng ký user
            if (msg.type === 'register') {
                clients.set(msg.userId.toString(), ws);
                ws.userId = msg.userId;
                console.log('User registered:', msg.userId);
                return;
            }

            // Gửi tin nhắn realtime
            if (msg.type === 'message') {
                const targetWs = clients.get(msg.to.toString());
                if (targetWs && targetWs.readyState === ws.OPEN) {
                    targetWs.send(JSON.stringify({
                        from: msg.from,
                        to: msg.to,
                        content: msg.content
                    }));
                    console.log('WS message received:', msg);
                }
            }
        } catch (e) {
            console.log('WS error:', e.message);
        }
    });

    ws.on('close', () => {
        if (ws.userId) {
            clients.delete(ws.userId.toString());
            console.log('User disconnected:', ws.userId);
        }
    });
});

module.exports = app;
