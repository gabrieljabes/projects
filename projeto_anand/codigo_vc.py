import cv2
import socket

# Configuração UDP
UDP_IP = "172.20.10.2"   
UDP_PORT = 4210         
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# URL do stream MJPEG do ESP32-CAM
STREAM_URL = "http://172.20.10.2:81/stream"

# Inicializa VideoCapture
cap = cv2.VideoCapture(STREAM_URL)
if not cap.isOpened():
    print("Erro ao conectar ao stream da câmera")
    exit()

while True:
    try:
        ret, frame = cap.read()

        if not ret:
            continue

        # reduz resolução
        frame_resized = cv2.resize(frame, (0, 0), fx=0.5, fy=0.5)
        height, width, _ = frame_resized.shape
        region_width = width // 5  # 5 regiões

        # processa imagem
        gray = cv2.cvtColor(frame_resized, cv2.COLOR_BGR2GRAY)
        blurred = cv2.GaussianBlur(gray, (5, 5), 0)
        _, thresh = cv2.threshold(blurred, 220, 255, cv2.THRESH_BINARY)

        contorno, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

        # regiões: começam "acesas"
        regions = {
            "FARLEFT": True,
            "LEFT": True,
            "CENTER": True,
            "RIGHT": True,
            "FARRIGHT": True
        }

        for cnt in contorno:
            x, y, w, h = cv2.boundingRect(cnt)
            if w * h > 30:  # descarta ruído
                cx = x + w // 2
                if cx < region_width:
                    regions["FARLEFT"] = False
                elif cx < 2 * region_width:
                    regions["LEFT"] = False
                elif cx < 3 * region_width:
                    regions["CENTER"] = False
                elif cx < 4 * region_width:
                    regions["RIGHT"] = False
                else:
                    regions["FARRIGHT"] = False

        # envia estado pro ESP via UDP + log
        msgs_enviadas = []
        for regiao, estado in regions.items():
            msg = f"{regiao}{'ON' if estado else 'OFF'}"
            sock.sendto(msg.encode(), (UDP_IP, UDP_PORT))
            msgs_enviadas.append(msg)

        print("Enviado:", msgs_enviadas)

        # desenha divisões
        for i in range(1, 5):
            cv2.line(frame_resized, (i * region_width, 0), (i * region_width, height), (255, 0, 0), 2)

        cv2.imshow("Frame", frame_resized)
        cv2.imshow("Bright Spots", thresh)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    except Exception as e:
        print("Erro:", e)

cap.release()
cv2.destroyAllWindows()
