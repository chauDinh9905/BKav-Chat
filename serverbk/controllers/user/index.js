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
const multer = require('multer');
const upload = multer({
    storage: multer.memoryStorage(),
    limits: { fileSize: 5 * 1024 * 1024 }, // giới hạn 5MB
    fileFilter: (req, file, cb) => {
        if (!file.mimetype.startsWith('image/')) {
            return cb(new Error('Only image files are allowed'));
        }
        cb(null, true);
    }
});
function convertObjectIdToInt(objectId) {
    const hex = objectId.toString().slice(-8); // Lấy 8 ký tự cuối của ObjectId (đảm bảo tính duy nhất cao)
    return parseInt(hex, 16); // Chuyển từ hệ cơ số 16 sang số nguyên int
}
module.exports = () => {
    router.post('/update', (req, res, next) => {
    upload.single('avatar')(req, res, (err) => {
        if (err) {
            return res.status(400).json({ status: 0, data: null, message: err.message });
        }
        next();
    });
}, async (req, res) => {
    try {
        console.log(req.UserID)
        const UserID = req.UserID
        const { FullName } = req.body
        let user = await models.Users.findOne({ _id: new ObjectId(UserID) }).exec()
        if (user == null) {
            return res.status(400).json({ status: 0, data: null, message: 'User not found' })
        }

        const allowedExtensions = ['png', 'jpg', 'jpeg'];
        let avatar = null
        let oldAvatarPath = user.avatar_path

        if (req.file) {
            const extension = req.file.originalname.split('.').pop().toLowerCase();
            if (!allowedExtensions.includes(extension)) {
                return res.status(400).json({ status: 0, data: null, message: 'Invalid file extension' })
            }
            const nameFile = uuidv4();
            const fullPath = path.join(savePathImageAvatar, `${nameFile}.${extension}`);
            await fs.promises.writeFile(fullPath, req.file.buffer);
            avatar = `/avatar/${nameFile}.${extension}`;
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

        // Xóa avatar cũ sau khi update thành công, tránh rác tích lũy
        if (avatar && oldAvatarPath) {
            const oldFullPath = path.join(parentDirectory, 'images', oldAvatarPath.replace('/avatar/', 'avatar/'));
            fs.promises.unlink(oldFullPath).catch(() => {
                // Bỏ qua nếu file cũ không tồn tại hoặc đã bị xóa trước đó
            });
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
        return res.status(200).json({
            status: 1,
            data: users.map(user => ({
                user_id: user?.user_id,
                display_name: user.display_name,
                //nickname: nicknameMap[user.user_id] || "",
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