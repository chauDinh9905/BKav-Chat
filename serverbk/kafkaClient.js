const { Kafka } = require('kafkajs');

const kafka = new Kafka({
    clientId: 'chat-app',
    brokers: [process.env.KAFKA_BROKER || 'kafka:9092']
});

const producer = kafka.producer();
const consumer = kafka.consumer({ groupId: 'chat-group-id' });

async function initKafka() {
    await producer.connect();
    await consumer.connect();
    // Subscribe vào topic 'chat-messages' để nhận tin từ các server khác
    await consumer.subscribe({
    topic: 'user_presence',
    fromBeginning: false,
    });

    await consumer.subscribe({
        topic: 'chat_messages',
        fromBeginning: false,
    });
    await consumer.subscribe({ 
        topic: 'user_presence_sync',
        fromBeginning: false 
    });
    console.log("Kafka Client initialized and connected.");

}

async function sendToKafka(topic, data) {
    console.log("Sending to topic:", JSON.stringify(topic));
    await producer.send({
        topic: topic,
        messages: [{ value: JSON.stringify(data) }],
    });
}

module.exports = { initKafka, sendToKafka, consumer };