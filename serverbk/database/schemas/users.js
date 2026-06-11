const mongoose = require('mongoose')

let User = new mongoose.Schema({
   user_id: { type: Number, required: false },
   username: {
        type: String, required: true, default: null,
        validate: {
            validator: function (value) {
                return /^[a-zA-Z0-9_-]+$/.test(value);
            },
            message: 'Tên đăng nhập chỉ được bao gồm chữ cái, số và dấu gạch dưới.'
        },
        unique: true
    },
    display_name: { type: String, required: false, default: null },
    password_hash: { type: String, required: true, default: null },
    created_at: { type: Date, required: false, default: Date.now },
    avatar_path: { type: String, required: false, default: null }
})

let UserModel = mongoose.model('User', User)
module.exports = UserModel