var express = require('express')
var router = express.Router()
var mongoose = require('mongoose')
var models = reqlib('database').models
var moment = require('moment')
const { generateToken } = require('../../utils/jwtUtils')
const crypto = require('crypto');

function convertObjectIdToInt(objectId) {
    const hex = objectId.toString().slice(-8); // Lấy 8 ký tự cuối của ObjectId (đảm bảo tính duy nhất cao)
    return parseInt(hex, 16); // Chuyển từ hệ cơ số 16 sang số nguyên int
}

module.exports = () => {
    router.post('/register', async (req, res) => {
    try {
        
        const { display_name, username, password: Password } = req.body;
        
        //  Kiểm tra username tồn tại
        let info = await models.Users.findOne({ username: username }).exec();
        if (info != null) {
            return res.status(400).json({ status: 0, data: null, message: 'Username already exists' });
        }

        //  Băm SHA-256 thẳng (Đồng bộ, không cần callback lồng nhau nữa)
        const hash = crypto.createHash('sha256').update(Password).digest('hex');

        // Tạo và lưu User mới vào DB
        const user = await models.Users({
            display_name: display_name,
            username: username,
            password_hash: hash, // Lưu chuỗi hash SHA-256 thẳng ở đây
            avatar_path: null,
            created_at: moment().toDate()
        }).save();

        //  Tạo token và xử lý ID cho Qt
        const numericUserId = convertObjectIdToInt(user._id);
        const token = await generateToken({
            uuid: user._id,
            display_name: display_name
        });

        //  Trả kết quả về cho Client (Qt)
        return res.status(200).json({
            status: 1, 
            data: {
                token: token,
                user_id: numericUserId, // Trả về dạng số int cho Qt
                username: username,
                display_name: display_name,
                avatar_path: null
            }, 
            message: 'success register'
        });

    } catch (error) {
        // Tất cả lỗi từ DB, băm chuỗi hay token đều lọt vào đây
        return res.status(400).json({ status: 0, data: null, message: error.message });
    }
});

    router.post('/login', async (req, res) => {
    try {
        
        const { username, password: Password } = req.body;
        
        // Tìm user theo username
        let info = await models.Users.findOne({ username: username }).exec();
        if (info == null) {
            return res.status(400).json({
                status: 0, data: null, message: 'Username not found'
            });
        }

        // 2. Băm mật khẩu người dùng vừa nhập bằng SHA-256 thẳng
        const inputHash = crypto.createHash('sha256').update(Password).digest('hex');

        // 3. So sánh trực tiếp chuỗi vừa băm với password_hash trong database (info.password_hash)
        // Lưu ý: Bạn kiểm tra lại tên trường trong DB của bạn là info.password_hash hay info.Password nhé
        if (inputHash === info.password_hash) {
            
            // Cập nhật thông tin user (nếu cần)
            await models.Users.updateOne({ _id: info._id });
            
            // Tạo token
            const token = await generateToken({
                uuid: info._id,
                display_name: info.display_name // Sửa lại theo trường display_name ở API register của bạn
            });

            // Trả về dữ liệu đăng nhập thành công
            return res.status(200).json({
                status: 1, 
                data: {
                    token: token,
                    username: info.username,
                    display_name: info.display_name,
                    avatar_path: info.avatar_path || null
                }, 
                message: 'success login' // Sửa chữ register thành login cho đúng ngữ cảnh
            });
        } else {
            // Mật khẩu không trùng khớp
            return res.status(401).json({
                status: 0, data: null, message: "Incorrect password"
            });
        }

    } catch (error) {
        // Bắt toàn bộ lỗi hệ thống hoặc DB lọt vào đây
        return res.status(400).json({ status: 0, data: null, message: error.message });
    }
});

    return router
}