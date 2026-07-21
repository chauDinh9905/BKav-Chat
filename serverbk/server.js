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

function addClient(userId, ws) {
    const key = userId.toString();
    if (!clients.has(key)) {
        clients.set(key, new Set());
    }
    clients.get(key).add(ws);
}

function removeClient(userId, ws) {
    const key = userId.toString();
    const set = clients.get(key);
    if (!set) return false;
    set.delete(ws);
    if (set.size === 0) {
        clients.delete(key);
        return true; // user thực sự offline (không còn socket nào)
    }
    return false; // vẫn còn cửa sổ khác đang mở
}

function sendToUser(userId, payload) {
    const set = clients.get(userId.toString());
    if (!set) return;
    for (const ws of set) {
        if (ws.readyState === 1) {
            ws.send(JSON.stringify(payload));
        }
    }
}

global.sendToUser = sendToUser; // để routes/nickname.js (và các route khác) gọi được

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
                const wasOffline = !clients.has(msg.userId.toString());
                addClient(msg.userId, ws);
                ws.userId = msg.userId;
                console.log('User registered:', msg.userId);

                if (wasOffline) {
                    // chỉ báo "vừa online" nếu đây là cửa sổ đầu tiên của user này
                    await sendToKafka('user_presence', {
                        userId: msg.userId,
                        isOnline: true
                    });
                }
                await sendToKafka('user_presence_sync', {
                    requesterId: msg.userId
                });
                return;
            }
            if (msg.type === 'unregister') {
                const trulyOffline = removeClient(msg.userId, ws);
                console.log('User unregistered:', msg.userId);

                if (trulyOffline) {
                    await sendToKafka('user_presence', {
                        userId: msg.userId,
                        isOnline: false
                    });
                }
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
        const trulyOffline = removeClient(ws.userId, ws);
        console.log('User disconnected:', ws.userId);

        if (trulyOffline) {
            try{
                await sendToKafka('user_presence', {
                    userId: ws.userId,
                    isOnline: false
                });
            }catch(error){
                console.error('Kafka presence update failed on disconnect (non-fatal):', error.message);
            }
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
            const requesterSet = clients.get(data.requesterId.toString());
            if (!requesterSet || requesterSet.size === 0) return;
            const models = reqlib('database').models;

            clients.forEach((clientSet, clientId) => {
                if (clientId !== data.requesterId.toString() && clientSet.size > 0) {
                    for (const requesterWs of requesterSet) {
                        if (requesterWs.readyState === 1) {
                            requesterWs.send(JSON.stringify({
                                type: 'presence',
                                userId: parseInt(clientId),
                                isOnline: true
                            }));
                        }
                    }
                }
            });
        }
            
            if (topic === 'user_presence') {
            const models = reqlib('database').models;
            const allUsers = await models.Users.find(
                { user_id: { $ne: data.userId } },
                { user_id: 1 }
            );

            clients.forEach((clientSet, clientId) => {
                if (clientId !== data.userId.toString()) {
                    for (const clientWs of clientSet) {
                        if (clientWs.readyState === 1) {
                            clientWs.send(JSON.stringify({
                                type: 'presence',
                                userId: data.userId,
                                isOnline: data.isOnline
                            }));
                        }
                    }
                }
            });
        }

            if (topic === 'chat_messages') {
                const targetSet = clients.get(data.to.toString());
                const payload = JSON.stringify({
                    id: data.id,
                    from: data.from,
                    to: data.to,
                    content: data.content,
                    files: data.files,
                    images: data.images,
                    createAt: data.createAt
                });

                if (targetSet && targetSet.size > 0) {
                    for (const targetWs of targetSet) {
                        if (targetWs.readyState === 1) targetWs.send(payload);
                    }
                    const models = reqlib('database').models;
                    await models.Message.updateOne({_id: data.id, isSend: 0}, {isSend: 1});

                    const senderSet = clients.get(data.from.toString());
                    if (senderSet) {
                        const deliveredPayload = JSON.stringify({
                            type: 'message_delivered',
                            messageId: data.id,
                            to: data.to
                        });
                        for (const senderWs of senderSet) {
                            if (senderWs.readyState === 1) senderWs.send(deliveredPayload);
                        }
                    }
                }

                // đồng bộ đa cửa sổ cho chính người gửi — không phân biệt window nào đã POST REST
                const senderSet = clients.get(data.from.toString());
                if (senderSet) {
                    const syncPayload = JSON.stringify({
                        type: 'message_sync',
                        id: data.id,
                        from: data.from,
                        to: data.to,
                        content: data.content,
                        files: data.files,
                        images: data.images,
                        createAt: data.createAt
                    });
                    for (const senderWs of senderSet) {
                        if (senderWs.readyState === 1) senderWs.send(syncPayload);
                    }
                }
            }
            if (topic === 'message_seen') {
                const targetSet = clients.get(data.to.toString());
                if (targetSet) {
                    const payload = JSON.stringify({
                        type: 'message_seen',
                        by: data.by
                    });
                    for (const targetWs of targetSet) {
                        if (targetWs.readyState === 1) targetWs.send(payload);
                    }
                }
            }
        }
    });
}

startConsumer().catch(console.error);
module.exports = app;
