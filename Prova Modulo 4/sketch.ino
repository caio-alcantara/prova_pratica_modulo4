#include <WiFi.h>
#include <HTTPClient.h>

#define led_verde 41 // Pino utilizado para controle do led verda
#define led_vermelho 40 // Pino utilizado para controle do led vermelho
#define led_amarelo 9 // Pino utilizado para controle do led azul

class Timer {
private:
  unsigned long intervalo;  
  unsigned long ultimo_tempo;  
  bool ativado;            

public:
  Timer(unsigned long intervalo_millis) {
    intervalo = intervalo_millis;
    ultimo_tempo = 0;
    ativado = true;
  }

  // Configura o intervalo do timer
  void define_intervalo(unsigned long intervalo_millis) {
    intervalo = intervalo_millis;
  }

  // Inicia ou reinicia o timer
  void start() {
    ultimo_tempo = millis();
    ativado = true;
  }

  // Para o timer
  void stop() {
    ativado = false;
  }

  // Verifica se o tempo decorrido atingiu o intervalo
  bool esta_pronto() {
    if (!ativado) return false;

    unsigned long tempo_atual = millis();
    if (tempo_atual - ultimo_tempo >= intervalo) {
      ultimo_tempo = tempo_atual; // Reinicia o contador
      return true;
    }
    return false;
  }
};

bool estado_led_amarelo = false;
bool estado_led_verde = false;
bool estado_led_vermelho = false;


const int pino_botao = 18;  // pino do botao
int estado_botao = 0;  // variavel para ler o estado do botao
int ultimo_estado_botao = LOW;

unsigned long ultimo_debounce = 0;  
unsigned long delay_debounce = 50;

int contagem_apertos = 0;

const int pino_ldr = 4;  // pino do sensor LDR
int limiar = 600;

Timer timer_luz_verde(3000);
Timer timer_luz_amarela(2000);
Timer timer_luz_vermelha(5000);

void setup() {

  // Configuração inicial dos pinos para controle dos leds como OUTPUTs (saídas) do ESP32
  pinMode(led_verde,OUTPUT);
  pinMode(led_vermelho,OUTPUT);
  pinMode(led_amarelo,OUTPUT);
  pinMode(pino_ldr, INPUT);

  // Inicialização das entradas
  pinMode(pino_botao, INPUT); // Inicializa o botao como entrada

  digitalWrite(led_verde, LOW);
  digitalWrite(led_vermelho, LOW);
  digitalWrite(led_amarelo, LOW);

  Serial.begin(9600); // Configuração para debug por interface serial entre ESP e computador com baud rate de 9600

  WiFi.begin("Wokwi-GUEST", ""); // Conexão à rede WiFi aberta com SSID Wokwi-GUEST

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("Conectado ao WiFi com sucesso!"); // Considerando que saiu do loop acima, o ESP32 agora está conectado ao WiFi (outra opção é colocar este comando dentro do if abaixo)

  
}

void loop() {
  int ldr_status=analogRead(pino_ldr);

  if(ldr_status>=limiar){
    //Serial.print("Está escuro. Ative a luz.");
    //Serial.println(ldr_status);
    estado_led_amarelo = !estado_led_amarelo;
    digitalWrite(led_amarelo, estado_led_amarelo);
    digitalWrite(led_verde, LOW);
    digitalWrite(led_vermelho, LOW);
    delay(500);


  } else {
    while (!timer_luz_verde.esta_pronto()) {
      semaforo_aberto();  
    }
    while (!timer_luz_amarela.esta_pronto()) {
      semaforo_amarelo();
    }
    while (!timer_luz_vermelha.esta_pronto()) {
      semaforo_fechado();  
    }
  }
}

void semaforo_aberto() {
  digitalWrite(led_verde, HIGH);
  digitalWrite(led_amarelo, LOW);
  digitalWrite(led_vermelho, LOW);
}

void semaforo_amarelo() {
  
  digitalWrite(led_verde, LOW);
  digitalWrite(led_amarelo, HIGH);
  digitalWrite(led_vermelho, LOW);
}

void semaforo_fechado() {
  
  digitalWrite(led_verde, LOW);
  digitalWrite(led_amarelo, LOW);
  digitalWrite(led_vermelho, HIGH);

  int leitura = digitalRead(pino_botao);
  if (leitura != ultimo_estado_botao) {
    ultimo_debounce = millis();
  }

  if ((millis() - ultimo_debounce) > delay_debounce) {

    if (leitura != estado_botao) {
      estado_botao = leitura;

      if (estado_botao == HIGH) {
        contagem_apertos++;
        Serial.println(contagem_apertos);
        //delay(50);
      } 

      if (contagem_apertos == 1) {
        delay(1000);
        return;
      }

      if (contagem_apertos == 3) {
        if(WiFi.status() == WL_CONNECTED){ // Se o ESP32 estiver conectado à Internet
          HTTPClient http;

          String caminho_servidor = "http://www.google.com.br/"; // Endpoint da requisição HTTP

          http.begin(caminho_servidor.c_str());

          int codigo_resposta_http = http.GET(); // Código do Resultado da Requisição HTTP

          if (codigo_resposta_http>0) {
            Serial.print("Código de resposta HTTP: ");
            Serial.println(codigo_resposta_http);
            //String payload = http.getString();
            //Serial.println(payload); Comentado para não poluir monitor serial.
            }
          else {
            Serial.print("Código de erro: ");
            Serial.println(codigo_resposta_http);
            }
            http.end();
          }

        else {
          Serial.println("WiFi desconectado");
        }
      }

    }
  }

  ultimo_estado_botao = leitura;
}