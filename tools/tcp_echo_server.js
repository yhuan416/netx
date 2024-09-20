const net = require('net');

// 创建一个 TCP 服务器
const server = net.createServer((socket) => {
    console.log('客户端已连接');

    // 当服务器接收到数据时，将其回显给客户端
    socket.on('data', (data) => {
        console.log('接收到数据：', data.toString());
        socket.write(data);
    });

    // 当服务器与客户端断开连接时
    socket.on('end', () => {
        console.log('客户端已断开连接');
    });

    // 监听错误事件
    socket.on('error', (err) => {
        console.error('发生错误：', err.message);
    });
});

// 指定服务器监听的端口号
const port = 8080;

server.listen(port, () => {
    console.log(`服务器正在监听端口：${port}`);
});
