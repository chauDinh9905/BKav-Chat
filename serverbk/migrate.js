require('dotenv').config();
const mongoose = require('mongoose');
mongoose.Promise = require('bluebird');

const dbURI = `mongodb://${process.env.DB_HOST}:${process.env.DB_PORT}/${process.env.DB_NAME}`;

mongoose.connect(dbURI, {
    socketTimeoutMS: 30000,
    useNewUrlParser: true,
    useUnifiedTopology: true
});

const Users = require('./database/schemas/users');

async function migrate() {
    const users = await Users.find({ user_id: null }).sort({ _id: 1 }).exec();
    
    const lastUser = await Users.findOne({ user_id: { $ne: null } }).sort({ user_id: -1 }).exec();
    let nextId = lastUser ? lastUser.user_id + 1 : 1;

    for (const user of users) {
        await Users.updateOne({ _id: user._id }, { user_id: nextId });
        console.log(`Updated user ${user.username} → user_id: ${nextId}`);
        nextId++;
    }

    console.log('Migration done!');
    mongoose.disconnect();
}

mongoose.connection.once('open', () => {
    console.log('DB connected, starting migration...');
    migrate();
});
