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

        console.log('Body nhận được:', JSON.stringify(req.body, null, 2));
        console.log(' Display name:', display_name);
        console.log(' Username:', username);
        console.log(' Password length:', Password ? Password.length : 0);
        
        //  Kiểm tra username tồn tại
        let info = await models.Users.findOne({ username: username }).exec();
        console.log(' Kiểm tra username:', username);
        console.log(' Kết quả:', info ? ' Đã tồn tại' : ' Chưa có');
        if (info != null) {
             console.log(' Trả về lỗi: Username already exists');
            return res.status(400).json({ status: 0, data: null, message: 'Username already exists' });
        }

        const lastUser = await models.Users.findOne().sort({ user_id: -1 }).exec();
        const newUserId = lastUser && lastUser.user_id ? lastUser.user_id + 1 : 1;
        // Tạo và lưu User mới vào DB
        const user = await new models.Users({
            user_id: newUserId,
            display_name: display_name,
            username: username,
            password_hash: Password, // Lưu chuỗi hash SHA-256 thẳng ở đây
            avatar_path: null,
            created_at: moment().toDate()
        }).save();
         console.log('Đã lưu user thành công, ID:', user._id);
        //  Tạo token và xử lý ID cho Qt
        const token = await generateToken({
            uuid: user._id,
            display_name: display_name
        });
        console.log('Tạo token thành công');
        console.log(' Đăng ký thành công cho user:', username);

        //  Trả kết quả về cho Client (Qt)
        return res.status(200).json({
            status: 1, 
            data: {
                token: token,
                user_id: user.user_id, // Trả về dạng số int cho Qt
                username: username,
                display_name: display_name,
                avatar_path: null
            }, 
            message: 'success register'
        });

    } catch (error) {
        console.error(' LỖI SERVER:', error);
        console.error(' Stack trace:', error.stack);

        // Tất cả lỗi từ DB, băm chuỗi hay token đều lọt vào đây
        return res.status(400).json({ status: 0, data: null, message: error.message });
    }
});

    router.post('/login', async (req, res) => {
        console.log('[LOGIN] Route reached!');
    try {
        
        const { username, password: password } = req.body;
         console.log('[LOGIN] Username:', username);
        if (!username || !password) {
            console.log('[LOGIN] Missing username or password');
            return res.status(400).json({ 
                status: 0, 
                message: 'Thiếu username hoặc password' 
            });
        }
        // Tìm user theo username
        let info = await models.Users.findOne({ username: username }).exec();
        console.log(' [LOGIN] User found:', info ? 'YES' : 'NO');
        if (info == null) {
            console.log(' [LOGIN] Username not found');
            return res.status(400).json({
                status: 0, data: null, message: 'Username not found'
            });
        }
        console.log(' [LOGIN] DB password hash:', info.password_hash);
        console.log(' [LOGIN] Password match:', password === info.password_hash ? 'YES' : 'NO');

        // So sánh trực tiếp chuỗi vừa băm với password_hash trong database (info.password_hash)
        // Lưu ý: Bạn kiểm tra lại tên trường trong DB của bạn là info.password_hash hay info.Password nhé
        if (info.password_hash === password) {
            console.log(' [LOGIN] Password correct');
            // Cập nhật thông tin user (nếu cần)
            await models.Users.updateOne({ _id: info._id }, { last_login: moment().toDate() });
            
            // Tạo token
            const token = await generateToken({
                uuid: info._id,
                display_name: info.display_name // Sửa lại theo trường display_name ở API register của bạn
            });

            
            console.log('[LOGIN] Login successful for user:', username);
            // Trả về dữ liệu đăng nhập thành công
            return res.status(200).json({
                status: 1, 
                data: {
                    token: token,
                    user_id: info.user_id,
                    username: info.username,
                    display_name: info.display_name,
                    avatar_path: info.avatar_path || null
                }, 
                message: 'success login' // Sửa chữ register thành login cho đúng ngữ cảnh
            });
        } else {
            console.log(' [LOGIN] Password incorrect');
            // Mật khẩu không trùng khớp
            return res.status(401).json({
                status: 0, data: null, message: "Incorrect password"
            });
        }

    } catch (error) {
        console.error(' [LOGIN] Error:', error);
        // Bắt toàn bộ lỗi hệ thống hoặc DB lọt vào đây
        return res.status(400).json({ status: 0, data: null, message: error.message });
    }
});

    return router
}