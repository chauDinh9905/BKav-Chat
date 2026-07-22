var express = require('express')
var router = express.Router()
var models = reqlib('database').models
const { ObjectId } = require('mongoose').Types
const authMiddleware = require('./../../middleware/index.js')

module.exports = () => {
    router.post('/set-nickname',authMiddleware, async (req, res) => {
        try {
            const UserID = req.UserID
            const targetId = Number(req.body.FriendID)
            const nickname = req.body.Nickname

            if (!targetId || !nickname) {
                return res.status(400).json({ status: 0, data: null, message: 'Thiếu dữ liệu' })
            }

            let user = await models.Users.findOne({ _id: new ObjectId(UserID) }).exec()
            if (user == null) {
                return res.status(400).json({ status: 0, data: null, message: 'User not found' })
            }
            const ownerId = user.user_id

            await models.Nickname.findOneAndUpdate(
                { owner_id: ownerId, target_id: targetId },
                { nickname },
                { upsert: true, new: true }
            )

            global.sendToUser(ownerId, {
                type: 'nickname_updated',
                friend_id: targetId,
                nickname: nickname
            })

            return res.status(200).json({ status: 1, data: { FriendID: targetId, Nickname: nickname }, message: '' })
        } catch (error) {
            return res.status(400).json({ status: 0, data: null, message: error.message })
        }
    })

    router.post('/remove-nickname',authMiddleware, async (req, res) => {
        try {
            const UserID = req.UserID
            const targetId = Number(req.body.FriendID)

            let user = await models.Users.findOne({ _id: new ObjectId(UserID) }).exec()
            if (user == null) {
                return res.status(400).json({ status: 0, data: null, message: 'User not found' })
            }
            const ownerId = user.user_id

            await models.Nickname.deleteOne({ owner_id: ownerId, target_id: targetId })

            const targetUser = await models.Users.findOne({ user_id: targetId }).exec()
            const originalName = targetUser ? targetUser.display_name : ''

            global.sendToUser(ownerId, {
                type: 'nickname_updated',
                friend_id: targetId,
                nickname: originalName
            })

            return res.status(200).json({ status: 1, data: { originalName }, message: '' })
        } catch (error) {
            return res.status(400).json({ status: 0, data: null, message: error.message })
        }
    })

    return router
}