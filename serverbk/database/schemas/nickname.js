const mongoose = require('mongoose')

let Nickname = new mongoose.Schema({
    owner_id: { type: String, required: true },
    target_id: { type: String, required: true },
    nickname: { type: String, required: true, trim: true, maxlength: 50 },
    updated_at: { type: Date, default: Date.now }
})

Nickname.index({ owner_id: 1, target_id: 1 }, { unique: true })
Nickname.index({ owner_id: 1 })

module.exports = mongoose.model('Nickname', Nickname)