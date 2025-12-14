import tkinter as tk
from PIL import Image, ImageTk
import paho.mqtt.client 
import base64
import io
from random import randrange
import math

hex_buffer = ""
recebendo_foto = False
ponto_anterior = (0,0)


class DashboardBarco:
    def __init__(self, root):
        self.root = root
        root.title("Dashboard Barco RC - Projeto Final")
        root.geometry("1200x700")  # aumentei um pouco a janela pra caber tudo melhor
        root.configure(bg="#e6f2ff")

        # ------------------- TOP -------------------
        top = tk.Frame(root, bg="#e6f2ff")
        top.pack(side="top", anchor="nw", padx=10, pady=10, fill="x")

        tk.Label(top, text="Dashboard do Barco (Projeto Final)",
                 font=("Segoe UI", 20, "bold"),
                 fg="#004c99", bg="#e6f2ff").pack(side="left")


        self.fotos = ["foto_01.jpg", "foto_02.jpg"]
        self.foto_index = 0

        # ------------------- BODY -------------------
        body = tk.Frame(root, bg="#e6f2ff")
        body.pack(fill="both", expand=True, padx=10, pady=5)

        # =================== COLUNA ESQUERDA ===================
        # Foto + cards de Velocidade / Distância / Lanterna
        left = tk.Frame(body, bg="#e6f2ff")
        left.pack(side="left", fill="y", expand=False, padx=(0, 10))


        # --- FRAME COM OS CARDS EMBAIXO DA FOTO ---
        cards_frame = tk.Frame(left, bg="#e6f2ff")
        cards_frame.pack(fill="x")


        self.lbl_head = tk.Label(cards_frame, text="Distância: 0 cm",
                                 font=("Segoe UI", 14, "bold"),
                                 bg="white", fg="#004c99",
                                 bd=1, relief="solid",
                                 padx=10, pady=10)
        self.lbl_head.pack(fill="x", pady=(0, 8))

        self.lbl_velocidade = tk.Label(cards_frame, text="Velocidade",
                                 font=("Segoe UI", 14, "bold"),
                                 bg="white", fg="#990085",
                                 bd=1, relief="solid",
                                 padx=10, pady=10)
        self.lbl_velocidade.pack(fill="x", pady=(0, 8))

        self.lbl_angulo = tk.Label(cards_frame, text="Velocidade",
                                 font=("Segoe UI", 14, "bold"),
                                 bg="white", fg="#35CD0F",
                                 bd=1, relief="solid",
                                 padx=10, pady=10)
        self.lbl_angulo.pack(fill="x", pady=(0, 8))
    

        self.lbl_lanterna = tk.Label(cards_frame, text="Lanterna: Apagada",
                                     font=("Segoe UI", 14, "bold"),
                                     bg="white", fg="#aa0000",   # vermelho = apagada
                                     bd=1, relief="solid",
                                     padx=10, pady=10)
        self.lbl_lanterna.pack(fill="x")
        mapa_frame = tk.Frame(left, bg="#e6f2ff")
        mapa_frame.pack(fill="both", expand=True, pady=(10, 0))

        tk.Label(mapa_frame, text="Mapa do barco",
                 font=("Segoe UI", 11, "bold"),
                 bg="white", fg="#004c99",
                 anchor="w", padx=5).pack(fill="x")

        # Canvas simples para desenhar o caminho
        self.mapa_canvas = tk.Canvas(mapa_frame, bg="white", width=380, height=200)
        self.mapa_canvas.pack(fill="both", expand=True)

        # ponto inicial no centro do canvas
        self.mapa_prev_x = 10
        self.mapa_prev_y = 150

        # =================== COLUNA DIREITA ===================
        # Frame do Ao Vivo do Barco
        mqtt_frame = tk.Frame(body, bg="#e6f2ff")
        mqtt_frame.pack(side="left", fill="both", expand=True, padx=(0, 0))

        tk.Label(mqtt_frame, text="Ao Vivo do Barco",
                 font=("Segoe UI", 11, "bold"),
                 bg="white", fg="#004c99",
                 anchor="w", padx=5).pack(fill="x")

        # Label que vai receber a imagem e ocupar o frame inteiro
        self.lbl_mqtt_foto = tk.Label(mqtt_frame,
                                      text="(nenhuma foto recebida ainda)",
                                      bg="#d9ffee", fg="black",
                                      bd=1, relief="solid")
        self.lbl_mqtt_foto.pack(fill="both", expand=True, pady=(0, 15))

                # ponto inicial no centro do canvas
        self.mapa_prev_x = 10
        self.mapa_prev_y = 150

        # ângulo atual (em graus) para o mapa – começa apontando para a direita (0°)
        self.mapa_heading = 0.0



    # ------------------- LOOP -------------------
    def loop(self):
        # agora o loop só mexe na velocidade;
        self.root.after(800, self.loop)


    def atualizar_foto(self):
        self.foto_index = (self.foto_index + 1) % len(self.fotos)
        img = Image.open(self.fotos[self.foto_index]).resize((400, 200))
        self.foto_barco = ImageTk.PhotoImage(img)
        self.lbl_foto.config(image=self.foto_barco)


# ---------- FUNÇÕES AUXILIARES PARA ATUALIZAR UI ----------

def atualizar_mqtt_foto_bytes(img_bytes):
    """
    Redimensiona a imagem recebida via MQTT para ocupar 100% do label lbl_mqtt_foto.
    """
    # Garante que o Tk já calculou o layout antes de medir o tamanho
    app.root.update_idletasks()

    # Tamanho atual do label onde a imagem vai ficar
    w = app.lbl_mqtt_foto.winfo_width()
    h = app.lbl_mqtt_foto.winfo_height()

    # fallback se ainda estiver muito pequeno no primeiro draw
    if w < 50 or h < 50:
        w, h = 800, 400

    img = Image.open(io.BytesIO(img_bytes))
    img = img.convert("RGB")

    # Se estiver em pé (mais alto que largo), gira 90º
    if img.height > img.width:
        img = img.rotate(90, expand=True)

    # Aqui a gente força exatamente o tamanho do frame (pode distorcer um pouco, mas ocupa 100%)
    img = img.resize((w, h))

    foto = ImageTk.PhotoImage(img)
    app.mqtt_foto_img = foto   # segura a referência
    app.lbl_mqtt_foto.config(image=foto, text="")

def atualizar_heading(dist_texto):
    """Atualiza a distância na interface a partir do valor recebido no tópico 'distancia'."""
    try:
        valor = float(dist_texto)
    except ValueError:
        print("Valor inválido recebido em 'distancia':", dist_texto)
        return
    app.lbl_head.config(text=f"Distância: {valor:.1f} cm")

def atualizar_lanterna(valor_texto):
    """Atualiza o estado da lanterna (0 = apagada, 1 = acesa)."""
    v = valor_texto.strip()
    if v == "1":
        app.lbl_lanterna.config(text="Lanterna: Acesa", fg="#008800")
    elif v == "0":
        app.lbl_lanterna.config(text="Lanterna: Apagada", fg="#aa0000")
    else:
        print("Valor inválido recebido em 'lanterna':", valor_texto)

def atualiza_velocidade_ang(valor_vel, valor_ang):
    valor = float(valor_vel)
    if valor <= 3:
        valor = 0
    ang = float(valor_ang)
    app.lbl_velocidade.config(text=f"Velocidade: {valor}")
    app.lbl_angulo.config(text=f"Ângulo: {valor_ang}° ")


# ------------------- MQTT -------------------

def on_connect(mitt, dados_usuario, flags, codigo): 
    # Assina fotos, distancia, lanterna e dados_vel_ang
    mqtt.subscribe("fotos")
    mqtt.subscribe("distancia")
    mqtt.subscribe("lanterna")
    mqtt.subscribe("dados_vel_ang")
    print("Conectado. Inscrito em 'fotos', 'distancia', 'lanterna' e 'dados_vel_ang'.")



def on_message(cliente, dados_usuario, mensagem):
    global hex_buffer, recebendo_foto

    print("Recebido:", mensagem.topic)
    print("Tamanho payload:", len(mensagem.payload))

    # 1) Tópico 'distancia' → atualiza distância (card azul)
    if mensagem.topic == "distancia":
        try:
            texto = mensagem.payload.decode("utf-8").strip()
            print("Distância recebida:", texto)
            app.root.after(0, atualizar_heading, texto)
        except Exception as e:
            print("Erro ao tratar mensagem de 'distancia':", e)
        return
    # 0) Tópico 'dados_vel_ang' → recebe x e y e desenha no mapa
    if mensagem.topic == "dados_vel_ang":
        try:
            texto = mensagem.payload.decode("utf-8").strip()
            print("dados_vel_ang recebido:", texto)

            # remove espaços
            t = texto.replace(" ", "")

            # agora fica assim: "A:91.96I:16"
            if "A:" not in t or "I:" not in t:
                print("Formato inválido:", texto)
                return

            parteA, parteI = t.split("I:")

            ang = float(parteA.split("A:")[1])
            vel = float(parteI)

            # chama atualização do mapa na thread do Tkinter
            app.root.after(0, atualizar_mapa, ang, vel)
            app.root.after(0, atualiza_velocidade_ang, vel, ang)

        except Exception as e:
            print("Erro ao tratar 'dados_vel_ang':", e)
        return


    # 2) Tópico 'lanterna' → atualiza estado da lanterna
    if mensagem.topic == "lanterna":
        try:
            texto = mensagem.payload.decode("utf-8").strip()
            print("Lanterna recebida:", texto)
            app.root.after(0, atualizar_lanterna, texto)
        except Exception as e:
            print("Erro ao tratar mensagem de 'lanterna':", e)
        return

    # 3) Tópico 'fotos' → mesma lógica de antes (binário ou HEX)
    # Primeiro tentamos tratar como TEXTO (protocolo NOVA_FOTO / FIM_FOTO / HEX)
    try:
        texto = mensagem.payload.decode("utf-8").strip()
    except UnicodeDecodeError:
        # Se não é texto, assumimos que é JPEG binário direto
        try:
            img_bytes = mensagem.payload
            print("Payload binário, primeiros bytes:", img_bytes[:20])

            app.root.after(0, atualizar_mqtt_foto_bytes, img_bytes)
            print("Imagem binária exibida com sucesso.")
        except Exception as e:
            print("Erro ao abrir imagem binária:", e)
        return  # não continua tentando tratar como texto

    # Se chegamos aqui, o payload de 'fotos' é texto
    print("Conteúdo (até 80 chars):", texto[:80])

    # Mensagem de INÍCIO (HEX)
    if texto.startswith("NOVA_FOTO"):
        print("Iniciando recepção de nova foto (HEX)...")
        hex_buffer = ""
        recebendo_foto = True
        return

    # Mensagem de FIM (HEX)
    if texto == "FIM_FOTO":
        print("Final da foto HEX. Tamanho do HEX acumulado:", len(hex_buffer))
        recebendo_foto = False

        try:
            img_bytes = bytes.fromhex(hex_buffer)
            print("Bytes de imagem (a partir do HEX):", len(img_bytes))

            app.root.after(0, atualizar_mqtt_foto_bytes, img_bytes)
            print("Imagem HEX exibida com sucesso.")
        except Exception as e:
            print("Erro ao montar imagem a partir do HEX:", e)

        return

    # No meio da recepção: acumula HEX
    if recebendo_foto:
        hex_buffer += texto

def atualizar_mapa(angulo_delta, velocidade):
    # converte do sistema do barco para o sistema do canvas
    ang_conv = (90 - angulo_delta) % 360

    # heading acumulado
    app.mapa_heading = (app.mapa_heading + ang_conv) % 360

    # direção em radianos
    rad = math.radians(app.mapa_heading)

    # deslocamento
    dx = math.cos(rad) * velocidade
    dy = math.sin(rad) * velocidade

    # ponto anterior
    x0 = app.mapa_prev_x
    y0 = app.mapa_prev_y

    # novo ponto proposto
    x1 = x0 + dx
    y1 = y0 - dy

    w = app.mapa_canvas.winfo_width()
    h = app.mapa_canvas.winfo_height()

    # clamping: limita dentro do retângulo
    x1 = max(0, min(x1, w))
    y1 = max(0, min(y1, h))

    # desenha linha até a borda (ou até onde conseguir)
    app.mapa_canvas.create_line(x0, y0, x1, y1, fill="red", width=2)

    # atualiza o ponto
    app.mapa_prev_x = x1
    app.mapa_prev_y = y1

    print(f"[MAPA] ΔAng={angulo_delta:.1f}°, heading={app.mapa_heading:.1f}°, "
          f"vel={velocidade:.1f} -> ({x1:.1f}, {y1:.1f})")



mqtt = paho.mqtt.client.Client() 
#mqtt.tls_set(certifi.where()) 
mqtt.username_pw_set(username="aula", password="zowmad-tavQez") 
mqtt.on_connect = on_connect 
mqtt.on_message = on_message 
mqtt.connect("mqtt.janks.dev.br", port=1883, keepalive=10) 

mqtt.loop_start()
print("Cliente iniciado, aguardando mensagens...\n")

# ------------------- RUN -------------------
if __name__ == "__main__":
    root = tk.Tk()
    global app
    app = DashboardBarco(root)
    app.loop()  # loop só da velocidade agora
    root.mainloop()
