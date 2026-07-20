var express = require('express')
var router = express.Router()
mongoose = require('mongoose')
models = reqlib('database').models

module.exports = () => {
    router.get('/', async (req, res) => {
        try {
            const owner_id = req.UserID
            const nicknames = await models.Nickname.find({ owner_id }).select('target_id nickname updated_at -_id')
            return res.status(200).json({ status: 1, data: nicknames, message: null })
        } catch (error) {
            return res.status(400).json({ status: 0, data: null, message: error.message })
        }
    })

    router.put('/:target_id', async (req, res) => {
        try {
            const owner_id = req.UserID
            const target_id = req.params.target_id
            const { nickname } = req.body

            if (!nickname || !nickname.trim()) {
                return res.status(400).json({ status: 0, data: null, message: 'Nickname không được để trống.' })
            }
            if (owner_id === target_id) {
                return res.status(400).json({ status: 0, data: null, message: 'Không thể tự đặt biệt danh cho chính mình.' })
            }

            const updated = await models.Nickname.findOneAndUpdate(
                { owner_id, target_id },
                { nickname: nickname.trim(), updated_at: new Date() },
                { upsert: true, new: true }
            )

            return res.status(200).json({ status: 1, data: updated, message: null })
        } catch (error) {
            return res.status(400).json({ status: 0, data: null, message: error.message })
        }
    })

    router.delete('/:target_id', async (req, res) => {
        try {
            const owner_id = req.UserID
            const target_id = req.params.target_id

            await models.Nickname.deleteOne({ owner_id, target_id })
            return res.status(200).json({ status: 1, data: null, message: 'Đã xoá biệt danh.' })
        } catch (error) {
            return res.status(400).json({ status: 0, data: null, message: error.message })
        }
    })

    return router
}