README


- descrição do projeto:
	O projeto consiste na construção de um barco de controle remoto com funcionalidades baseadas nos conteúdos aprendidos nas aulas de microcontroladores. O barco flutuará sobre a água e deverá se movimentar obedecendo aos comandos enviados por um controle sem fio, o qual contará com uma pequena interface de interação com a internet.
	Foi incluído no projeto final um manche de brinquedo como joystick de controle remoto do barco — que conta com uma tela digital para monitoramento dos comandos —, além de uma câmera de monitoramento e uma interface web que monitora, em tempo real, informações do barco, como velocidade e direção.

- arquitetura:
	
	joystick -> Arduino -> ESP -> MQTT -> ESP -> Arduino -> motores
	
	I- O joystick conectado ao Arduino manda comandos via comunicação serial ao ESP, que são também lidos pelo Arduino que apresenta esses dados de velocidade, direção, estado da câmera/luz e um gráfico do histórico da velocidade numa tela do Shield TFT.
	II- O ESP conectado ao joystick serve apenas como comunicador e passa para o servidor Wifi MQTT todos os dados que vão servir para mover o barco.
	III- O ESP que recebe os comandos, interpreta e envia para o Arduino do barco.
	IV - O Arduino conectado aos motores recebe via serial do ESP e escreve os dados lidos nos 2 motores.

