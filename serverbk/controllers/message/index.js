var express = require('express')
var router = express.Router()
var mongoose = require('mongoose')
var models = reqlib('database').models
var moment = require('moment')
const { ObjectId } = require('mongoose').Types
const fs = require('fs');
const path = require('path');
const { v4: uuidv4 } = require('uuid');
const multer = require('multer')
const upload = multer({ storage: multer.memoryStorage() })
const currentDirectory = __dirname;
const parentDirectory = path.resolve(currentDirectory, '..', '..');
const savePathImage = `${parentDirectory}/images`;
const savePathFile = `${parentDirectory}/files`;
const authMiddleware = require('./../../middleware/index.js')
const { sendToKafka } = require('./../../kafkaClient');

module.exports = () => {
    router.get('/list-friend', async (req, res) => {
        try {
            const UserID = req.UserID
            console.log('list-friend UserID:', UserID)
            let user = await models.Users.findOne({ _id: new ObjectId(UserID) }).exec()
            console.log('user found:', user)  
            console.log('error message sẽ là gì:',
            user == null ? 'User not found' : 'OK') 
            if (user == null) {
                return res.status(400).json({ status: 0, data: null, message: 'User not found' })
            }
            const listUser = await models.Users.find({ _id: { $ne: user._id } }).sort({ created_at: -1 }).exec()
            let listCustomFriend = []
            await Promise.all(listUser.map(async (value) => {
                const queryConditions = [
                    {
                        $or: [
                            { UserID: user.user_id, FriendID: value.user_id },
                            { UserID: value.user_id, FriendID: user.user_id }
                        ]
                    }
                ];
                const response = await models.Message.find({ $and: queryConditions }).sort({ CreatedAt: -1 }).limit(1);
                const unreadCount = await models.Message.countDocuments({
                UserID: value.user_id,
                FriendID: user.user_id,
                isSend: {$lt: 2}
                });
                listCustomFriend.push({
                    Content: response.length > 0 ? response[0]?.Content : '',
                    Files: response.length > 0 ? response[0]?.Files : null,
                    Images: response.length > 0 ? response[0]?.Images : null,
                    isSend: response.length > 0 ? response[0]?.isSend : 0,
                    FriendID: value.user_id,
                    FullName: value.display_name,
                    Username: value.username,
                    Avatar: value.avatar_path,
                    isOnline: moment(value.created_at).isSameOrAfter(moment().subtract(10, 'minutes')),
                    UnreadCount: unreadCount,                                          
                    LastMsgTime: response.length > 0 ? response[0].CreatedAt : null
                })
            }))

            listCustomFriend.sort((a, b) => {
            if (!a.LastMsgTime) return 1;
            if (!b.LastMsgTime) return -1;
            return new Date(b.LastMsgTime) - new Date(a.LastMsgTime);
            });

            await models.Users.updateOne({ _id: user._id }, { created_at: moment().toDate() })
            return res.status(200).json({ status: 1, data: listCustomFriend, message: "success" })

        } catch (error) {
            console.log('list-friend ERROR:', error.message) // 
            console.log('list-friend STACK:', error.stack)   //
            return res.status(400).json({ status: 0, data: null, message: error.message })
        }
    })

    router.post('/send-message',upload.any(), async (req, res) => {
        try {
            const UserID = req.UserID
            const { FriendID, Content } = req.body
            console.log('send-message UserID:', UserID, 'FriendID:', FriendID)
            let listImages = []
            let listFiles = []
            let user = await models.Users.findOne({ _id: new ObjectId(UserID) }).exec()
            if (user == null) {
                return res.status(400).json({ status: 0, data: null, message: 'User not found' })
            }

            let Friend = await models.Users.findOne({ user_id: parseInt(FriendID)}).exec()
            if(Friend == null){
                return res.status(400).json({status: 0, data: null, message: 'Friend not found at send-message'})
            }
            console.log('user.user_id:', user.user_id)
            console.log('Friend found:', Friend)
            if (Friend == null) {
                return res.status(400).json({ status: 0, data: null, message: 'Friend not found' })
            }
            for (const file of (req.files || []) /*req.files*/) {
                console.log('req.files:', req.files)
                if (file.fieldname === 'files') {
                    const extension = file.originalname.split('.').pop();
                    const nameFile = uuidv4();
                    if (!file.mimetype.startsWith('image/')) {
                        const fullPath = path.join(savePathFile, `${nameFile}.${extension}`);
                        fs.writeFileSync(fullPath, file.buffer);
                        const Link = `/files/${nameFile}.${extension}`;
                        listFiles.push({
                            urlFile: Link,
                            FileName: file.originalname
                        })
                    }
                    else {
                        const fullPath = path.join(savePathImage, `${nameFile}.${extension}`);
                        fs.writeFileSync(fullPath, file.buffer);
                        const Link = `/images/${nameFile}.${extension}`;
                        listImages.push({
                            urlImage: Link,
                            FileName: file.originalname
                        })
                    }

                }
            }

            const response = await models.Message({
                UserID: user.user_id,
                FriendID: Friend.user_id,
                Content: Content,
                Files: listFiles,
                Images: listImages,
                CreatedAt: moment().toDate(),
                UpdateAt: moment().toDate(),
                isSend: 0
            }).save()
            await models.Users.updateOne({ _id: user._id }, { created_at: moment().toDate() })
            await sendToKafka('chat_messages', {
                id: response._id,
                from: user.user_id,
                to: Friend.user_id,
                content: Content,
                files: listFiles,
                images: listImages
            });
            const resMessage = await models.Message.find({ FriendID: Friend.user_id, isSend: 0 }, { _id: 1, content: 1 }).sort({ CreatedAt: 1 });
            await Promise.all(resMessage.map(async (value) => {
                await models.Message.updateOne({ _id: value._id }, { isSend: 1 });
            }));
            return res.status(200).json({
                status: 1, data: {
                    id: response?._id,
                    Content: response?.Content,
                    Files: response?.Files,
                    Images: response?.Images,
                    isSend: response?.isSend,
                    CreatedAt: response?.CreatedAt,
                    MessageType: 1
                }, message: ""
            })

        } catch (error) {
            return res.status(400).json({ status: 0, data: null, message: error.message })
        }
    })


    router.get('/get-message', async (req, res) => {
        try {
            const UserID = req.UserID
            const { FriendID, LastTime } = req.query
            console.log('get-message UserID:', UserID, 'FriendID:', FriendID)
            let user = await models.Users.findOne({ _id: new ObjectId(UserID) }).exec()
            if (user == null) {
                return res.status(400).json({ status: 0, data: null, message: 'User not found' })
            }

            let Friend = await models.Users.findOne({ user_id: parseInt(FriendID)}).exec()
            if(Friend == null){
                return res.status(400).json({status: 0, data: null, message: 'Friend not found at get-message'})
            }
            console.log('user.user_id:', user.user_id)
            console.log('Friend.user_id:', Friend.user_id)
            if (Friend == null) {
                return res.status(400).json({ status: 0, data: null, message: 'Friend not found' })
            }
            const queryConditions = [
                {
                    $or: [
                        { UserID: user.user_id, FriendID: Friend.user_id },
                        { UserID: Friend.user_id, FriendID: user.user_id }
                    ]
                }
            ];

            if (LastTime) {
                queryConditions.push({ CreatedAt: { $gt: LastTime } });
            }
            const response = await models.Message.find({ $and: queryConditions }).sort({ CreatedAt: 1 });
            console.log('messages found:', response.length)
            const data = await Promise.all(response?.map(async (value) => {
                if (value.UserID === user.user_id) {
                    return ({
                        id: value._id,
                        Content: value?.Content,
                        Files: value?.Files,
                        Images: value?.Images,
                        isSend: value?.isSend,
                        CreatedAt: value?.CreatedAt,
                        MessageType: 1
                    })
                }
                else {
                    if (value?.isSend === 0) {
                        await models.Message.updateOne({ _id: value._id }, { isSend: 1 });
                        const senderWs = global.wsClients.get(value.UserID.toString());
                        if(senderWs && senderWs.readyState === 1){
                            senderWs.send(JSON.stringify({
                                type: 'message_delivered',
                                messageId: value._id,
                                to: user.user_id
                            }));
                        }
                    }
                    return ({
                        id: value._id,
                        Content: value?.Content,
                        Files: value?.Files,
                        Images: value?.Images,
                        isSend: value?.isSend === 0 ? 1 : values?.isSend,
                        CreatedAt: value?.CreatedAt,
                        MessageType: 0
                    })
                }

            }));
            await models.Users.updateOne({ _id: user._id }, { created_at: moment().toDate() })
            return res.status(200).json({ status: 1, data: data, message: "" })
        } catch (error) {
            console.log('get-message error:', error.message)
            return res.status(400).json({ status: 0, data: null, message: error.message })
        }
    })

    return router
}