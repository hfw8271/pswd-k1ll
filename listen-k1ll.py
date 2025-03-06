import socket

def start_listener():
    host = '0.0.0.0'
    port = 47022
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind((host, port))
    s.listen(5)
    print("Listener started...")

    while True:
        conn, addr = s.accept()
        print(f"Connection from {addr}")
        data = conn.recv(1024)
        print(data.decode('utf-8'))
        conn.close()

if __name__ == "__main__":
    start_listener()