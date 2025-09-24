import cv2
import socket

# configuração e porta udp
ip = "172.20.10.2"   
porta = 4210         
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) 

# url do esp
url = "http://172.20.10.2:81/stream"

# captura de vídeo
cap = cv2.VideoCapture(url)
if not cap.isOpened():
    print("erro ao conectar")
    exit()

while True:
    try:
        ret, frame = cap.read()
        #ret  indica se a leitura deu certo ou não do frame
        if not ret:
            continue

        # manipulação da resolução e obtenção das informações
        # definição da largura de cada regiao (são 5)
        frame_resized = cv2.resize(frame, (0, 0), fx=0.5, fy=0.5)   
        altura, largura, _ = frame_resized.shape  
        largura_regiao = largura // 5   

        # aplica os filtros
        gray = cv2.cvtColor(frame_resized, cv2.COLOR_BGR2GRAY)
        blurred = cv2.GaussianBlur(gray, (5, 5), 0)
        _, thresh = cv2.threshold(blurred, 220, 255, cv2.THRESH_BINARY)

        #define contorno 
        contorno, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

        # definiçao das regiões pra comunicar ao esp
        regioes = {
            "FARLEFT": True,
            "LEFT": True,
            "CENTER": True,
            "RIGHT": True,
            "FARRIGHT": True
        }
        # vai validar se a fonte de luz tá dentro das regiões 
        # com base no centro do contorno criado
        for cnt in contorno:
            x, y, w, h = cv2.boundingRect(cnt)
            if w * h > 30:  # descarta detecções muito pequenas
                cx = x + w // 2
                if cx < largura_regiao:
                    regioes["FARLEFT"] = False
                elif cx < 2 * largura_regiao:
                    regioes["LEFT"] = False
                elif cx < 3 * largura_regiao:
                    regioes["CENTER"] = False
                elif cx < 4 * largura_regiao:
                    regioes["RIGHT"] = False
                else:
                    regioes["FARRIGHT"] = False


        #log de pacotes enviados e comunicação com o esp
        msgs_enviadas = []
        for regiao, estado in regioes.items():
            msg = f"{regiao}{'ON' if estado else 'OFF'}"
            sock.sendto(msg.encode(), (ip, porta)) 
            msgs_enviadas.append(msg)
    
        print("enviado:", msgs_enviadas)

        # coloca linhas no frame mostrado pra ter uma ideia da divisao das regioes
        for i in range(1, 5):
            cv2.line(frame_resized, (i * largura_regiao, 0), (i * largura_regiao, altura), (255, 0, 0), 2)

        cv2.imshow("frame", frame_resized)
        cv2.imshow("luz", thresh)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    except Exception as e:
        print("erro:", e)

cap.release()
cv2.destroyAllWindows()
