require('dotenv').config()
var express = require('express');
var cookieParser = require('cookie-parser');
var bodyParser = require('body-parser')
var cors = require('cors')
const http = require('http');
const { WebSocketServer } = require('ws');
const { initKafka, sendToKafka, consumer } = require('./kafkaClient');


var app = express();
const port = process.env.PORT || 3000;

app.use(cors());
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));
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

function heartbeat() {
    this.isAlive = true;
}
 
const heartbeatInterval = setInterval(() => {
    wss.clients.forEach((ws) => {
        if (ws.isAlive === false) {
            console.log('Terminating dead connection for user:', ws.userId);
            return ws.terminate(); // trigger 'close' -> dọn clients Map + báo offline
        }
        ws.isAlive = false;
        ws.ping();
    });
}, 30000); // kiểm tra mỗi 30 giây
 
wss.on('close', () => {
    clearInterval(heartbeatInterval);
});

wss.on('connection', (ws) => {
    console.log('New WS connection');
     ws.isAlive = true;
    ws.on('pong', heartbeat);
    ws.on('message', async (data) => {
        try {
            const msg = JSON.parse(data);

            // Đăng ký user
            if (msg.type === 'register') {
                clients.set(msg.userId.toString(), ws);
                ws.userId = msg.userId;
                console.log('User registered:', msg.userId);

                await sendToKafka('user_presence', {
                    userId: msg.userId,
                    isOnline: true
                });
                 await sendToKafka('user_presence_sync', {
                requesterId: msg.userId  // ai cần nhận thông tin
                });
                return;
            }
            if (msg.type === 'unregister') {
                clients.delete(msg.userId.toString());
                console.log('User unregistered:', msg.userId);

                await sendToKafka('user_presence', {
                    userId: msg.userId,
                    isOnline: false
                });
                return;
            }
            if(msg.type === 'mark_seen'){
                const models = reqlib('database').models;
                await models.Message.updateMany(
                    {UserID: msg.friendId, FriendID: msg.userId, isSend: {$lt: 2}},
                    {isSend: 2}
                );

                await sendToKafka('message_seen', {
                    to: msg.friendId,
                    by: msg.userId
                    });
                return;
            }
        } catch (e) {
            console.log('WS error:', e.message);
        }
    });

    ws.on('close', async () => {
        if (ws.userId) {
            clients.delete(ws.userId.toString());
            console.log('User disconnected:', ws.userId);

            try{
                await sendToKafka('user_presence', {
                    userId: ws.userId,
                    isOnline: false
                });
            }catch(error){
                console.error('Kafka presence update failed on disconnect (non-fatal):', error.message);
            }
        }
    });
});
async function startConsumer() {
    await initKafka();

    await consumer.run({
        eachMessage: async ({ topic, message }) => {
            const data = JSON.parse(message.value.toString());

            if (topic === 'user_presence') {
                const models = reqlib('database').models;
                const allUsers = await models.Users.find(
                    { user_id: { $ne: data.userId } },
                    { user_id: 1 }
                );

                clients.forEach((clientWs, clientId) => {
                if (clientId !== data.userId.toString() && clientWs.readyState === 1) {
                    clientWs.send(JSON.stringify({
                        type: 'presence',
                        userId: data.userId,
                        isOnline: data.isOnline
                    }));
        }
    });
            }

            if (topic === 'user_presence_sync') {
            // Mỗi server instance báo cáo clients của mình
            // về cho requester
            const requesterWs = clients.get(data.requesterId.toString());
            if (!requesterWs || requesterWs.readyState !== 1) return;
            const models = reqlib('database').models;

            // Cập nhật persistent state trong DB
            // await models.Users.updateOne(
            //     { user_id: data.userId },
            //     { $set: { isOnline: data.isOnline } }
            // );
            // Gửi từng user đang online trên instance này
            clients.forEach((clientWs, clientId) => {
                if (clientId !== data.requesterId.toString() 
                    && clientWs.readyState === 1) {
                    requesterWs.send(JSON.stringify({
                        type: 'presence',
                        userId: parseInt(clientId),
                        isOnline: true
                    }));
                }
            });
        }

            if (topic === 'chat_messages') {
                const targetWs = clients.get(data.to.toString());
                if (targetWs && targetWs.readyState === 1) {
                    targetWs.send(JSON.stringify({
                        id: data.id,
                        from: data.from,
                        to: data.to,
                        content: data.content,
                        files: data.files,
                        images: data.images,
                        createAt: data.createAt
                    }));
                    const models = reqlib('database').models;
                    await models.Message.updateOne({_id: data.id, isSend: 0}, {isSend: 1});

                    const senderWs = clients.get(data.from.toString());
                    if(senderWs && senderWs.readyState === 1){
                        senderWs.send(JSON.stringify({
                            type: 'message_delivered',
                            messageId: data.id,
                            to: data.to
                        }));
                    }
                }
            }
            if (topic === 'message_seen') {
                const targetWs = clients.get(data.to.toString());
                if (targetWs && targetWs.readyState === 1) {
                    targetWs.send(JSON.stringify({
                        type: 'message_seen',
                        by: data.by
                    }));
                }
            }
        }
    });
}

startConsumer().catch(console.error);
module.exports = app;
