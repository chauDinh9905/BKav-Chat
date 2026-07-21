const mongoose = require('mongoose')

let Nickname = new mongoose.Schema({
    owner_id: { type: Number, required: true },
    target_id: { type: Number, required: true },
    nickname: { type: String, required: true }
})

Nickname.index({ owner_id: 1, target_id: 1 }, { unique: true })

let NicknameModel = mongoose.model('Nickname', Nickname)
module.exports = NicknameModel