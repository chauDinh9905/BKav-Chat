FROM node:18.19.1-alpine

WORKDIR /app

# Copy các file cần thiết
COPY package*.json ./
RUN npm install

# Copy toàn bộ code (server.js, etc) vào /app
COPY . .

EXPOSE 8888

CMD ["node", "server.js"]