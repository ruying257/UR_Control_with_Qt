import socket
import time

# 模拟 UR 机械臂的 IP (本机) 和端口
HOST = '127.0.0.1'
PORT = 30003

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind((HOST, PORT))
server.listen(1)

print(f"🤖 假机械臂已启动，正在监听 {HOST}:{PORT} ...")

while True:
    try:
        conn, addr = server.accept()
        print(f"✅ Qt程序已连接: {addr}")
        
        # 保持连接，直到 Qt 断开
        while True:
            data = conn.recv(1024)
            if not data:
                break
        print("❌ Qt程序已断开")
        conn.close()
    except Exception as e:
        print(e)