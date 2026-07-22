const { Kafka } = require('kafkajs');

const kafka = new Kafka({
    clientId: 'chat-app',
    brokers: [process.env.KAFKA_BROKER || 'kafka:9092']
});

const producer = kafka.producer();
const consumer = kafka.consumer({ groupId: 'chat-group-id' });
let producerConnected = false;

producer.on(producer.events.DISCONNECT, () => {
    console.warn('Kafka producer disconnected');
    producerConnected = false;
});
producer.on(producer.events.CONNECT, () => {
    producerConnected = true;
});
async function initKafka() {
    while(true){
        try{
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
            await consumer.subscribe({
                topic: 'message_seen',
                fromBeginning: false,
                });
            await consumer.subscribe({
                topic: 'nickname_updated',
                fromBeginning: false,
            })
            console.log("Kafka Client initialized and connected.");
            return;
        }catch (err) {
            console.log("Kafka chưa sẵn sàng:", err.message);
            await new Promise(r => setTimeout(r, 5000));
        }
    }

}

async function sendToKafka(topic, data) {
    try {
        if (!producerConnected) {
            await producer.connect();
        }
        await producer.send({
            topic,
            messages: [{ value: JSON.stringify(data) }],
        });
    } catch (err) {
        console.error(`Kafka send failed for topic "${topic}":`, err.message);
        // Không throw lại - caller không cần tự bọc try/catch nữa,
        // nhưng vẫn báo lỗi ra log để debug.
    }
}

module.exports = { initKafka, sendToKafka, consumer };