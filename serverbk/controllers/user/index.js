var express = require('express')
var router = express.Router()
var mongoose = require('mongoose')
var models = reqlib('database').models
var moment = require('moment')
const { ObjectId } = require('mongoose').Types

const fs = require('fs');
const path = require('path');
const { v4: uuidv4 } = require('uuid');
const currentDirectory = __dirname;
const parentDirectory = path.resolve(currentDirectory, '..', '..');
const savePathImageAvatar = `${parentDirectory}/images/avatar`;

function convertObjectIdToInt(objectId) {
    const hex = objectId.toString().slice(-8); // Lấy 8 ký tự cuối của ObjectId (đảm bảo tính duy nhất cao)
    return parseInt(hex, 16); // Chuyển từ hệ cơ số 16 sang số nguyên int
}
module.exports = () => {
    router.post('/update', async (req, res) => {
        try {
            const UserID = req.UserID
            const { FullName } = req.body
            let user = await models.Users.findOne({ _id: new ObjectId(UserID) }).exec()
            if (user == null) {
                return res.status(400).json({ status: 0, data: null, message: 'User not found' })
            }
            let avatar = null
            for (const file of req.files) {
                if (file.fieldname === 'avatar') {
                    const extension = file.originalname.split('.').pop();
                    const nameFile = uuidv4();
                    const fullPath = path.join(savePathImageAvatar, `${nameFile}.${extension}`);
                    fs.writeFileSync(fullPath, file.buffer);
                    avatar = `/avatar/${nameFile}.${extension}`;
                }
            }
            const updateObject = {};

            if (FullName) {
                updateObject.display_name = FullName;
            }

            if (avatar) {
                updateObject.avatar_path = avatar;
            }
            updateObject.created_at = moment().toDate();
            if (Object.keys(updateObject).length > 0) {
                await models.Users.updateOne({ _id: user._id }, updateObject);
            }
            return res.status(200).json({ status: 1, data: null, message: "update success" })

        } catch (error) {
            return res.status(400).json({ status: 0, data: null, message: error.message })
        }
    })

    router.get('/info', async (req, res) => {
        console.log("Header nhận được:", req.headers.authorization); // Xem token có tới nơi không
    console.log("UserID từ middleware:", req.UserID);
        try {
            const UserID = req.UserID
            let user = await models.Users.findOne({ _id: new ObjectId(UserID) }).exec()
            if (user == null) {
                return res.status(400).json({ status: 0, data: null, message: 'User not found' })
            }
            return res.status(200).json({
                status: 1, data: {
                    Id: user?.user_id,
                    Username: user?.username,
                    FullName: user?.display_name,
                    Avatar: user?.avatar_path
                }, message: ''
            })
        } catch (error) {
            return res.status(400).json({ status: 0, data: null, message: error.message })
        }
    })

    router.get('/list', async (req, res) => {
    try {
        const UserID = req.UserID;

        if (!UserID) {
        return res.status(401).json({
        status: 0,
        message: "Unauthorized"
            });
        }

        const users = await models.Users.find({
            _id: { $ne: new ObjectId(UserID) }
        }).lean();

        return res.status(200).json({
            status: 1,
            data: users.map(user => ({
                user_id: user?.user_id,
                display_name: user.display_name,
                avatar_path: user.avatar_path
            })),
            message: ''
        });
    } catch(error) {
        return res.status(400).json({
            status: 0,
            data: null,
            message: error.message
        });
    }
})
    
    router.get('/search', async (req, res) => {
    try {
        const keyword = req.query.keyword || '';

        const users = await models.Users.find({
            display_name: {
                $regex: keyword,
                $options: 'i'
            }
        }).lean();

        return res.status(200).json({
            status: 1,
            data: users.map(user => ({
                user_id: user?.user_id,
                display_name: user.display_name,
                avatar_path: user.avatar_path
            }))
        });

    } catch(error) {
        return res.status(400).json({
            status: 0,
            data: null,
            message: error.message
        });
    }
});
    return router
}