# 📟 Alarme de Proximidade com MSP430F5529  
Este projeto desenvolve um sistema de alarme de proximidade usando o microcontrolador MSP430F5529, um sensor ultrassônico e um módulo Bluetooth. O sistema notifica o usuário sobre invasões sem alertar o invasor com barulhos, aumentando a segurança residencial.

---

## 📋 **Descrição do Problema**  
O sistema foi idealizado para resolver problemas de segurança residencial, onde os alarmes tradicionais acabam alertando os invasores. Este projeto notifica o dono da casa via celular, permitindo ações imediatas e discretas.

---

## 🛠️ **Componentes Utilizados**  
- **Microcontrolador:** MSP430F5529  
- **Módulo Bluetooth:** HC-05  
- **Sensor Ultrassônico:** HC-SR04  
- **Resistores:** 1 de 22k e 1 de 33k  
- **Jumper Macho/Fêmea:** 4 unidades  
- **Jumper Macho/Macho:** 4 unidades  
- **Mini Protoboard:** 1 unidade  

---

## 🔌 **Conexões**  
### **Tabela de Pinos**  
| **Pino MSP** | **Sinal**             |  
|--------------|-----------------------|  
| P1.3         | HC-SR04: ECHO          |  
| P6.1         | HC-SR04: TRIGGER       |  
| P3.4         | HC-05: TX              |  
| P3.3         | HC-05: RX              |  

### **Tabela de Alimentação**  
| **Periférico** | **Voltagem** |  
|----------------|--------------|  
| HC-SR04        | 5V           |  
| HC-05          | 5V           |  

---

## ⚙️ **Configurações do MSP430F5529**  
### **Módulo Bluetooth (HC-05)**  
- **RX:** Conectado ao pino P3.3 (recebimento de dados).  
- **TX:** Conectado ao pino P3.4 (transmissão de dados).  
- **Interrupção:** Habilitada via `UCA0IE = UCRXIE`.  

#### **Configuração do Baud Rate:**  
- `UCA0BR0 = 6` e `UCA0BR1 = 0`: Configuram a velocidade de transmissão.  
- `UCA0MCTL = UCBRS_0 + UCBRF_13 + UCOS16`: Maior precisão no baud rate.  

### **Sensor Ultrassônico (HC-SR04)**  
- **TRIGGER (P6.1):** Configurado como saída para envio de pulsos de 10 µs.  
- **ECHO (P1.3):** Captura o tempo de resposta para cálculo da distância.  
- **Timer:**  
  - `TA0CCRO`: Intervalo de medição definido em 250 ms.  
  - `TA0CTL = TASSEL__ACLK | ID__4 | MC__CONTINUOUS | TACLR`: Configura o timer.  

#### **Cálculo da Distância**  
O tempo de início é armazenado em `start_time` e o tempo final em `end_time`. A distância é calculada usando a fórmula:  
- Distância (cm) = (tempo de eco / 2) × velocidade do som (~34.000 cm/s).  

### **Botão S2**  
- Configurado como entrada com resistor de pull-up para melhor medição.  
- Configurado para interrupção, desativando o sistema ao ser pressionado.  

### **LED Vermelho**  
- Configurado como saída para ser ativado sempre que o sensor detectar movimento.  

---

## 📊 **Resultado e Análise**  
O projeto atendeu aos objetivos propostos:  
- Funcionamento adequado do sensor de proximidade e comunicação Bluetooth.  
- Resposta satisfatória às entradas de armação e desarme do sistema.  
- Taxa de erro no cálculo da distância foi de aproximadamente 3 cm, considerada irrelevante para o funcionamento do sistema.  

---

## 📚 **Conclusão**  
O projeto demonstrou várias implementações práticas aprendidas ao longo do semestre, incluindo:  
- GPIO  
- Interrupções  
- Comunicação serial (I2C, SPI e UART)  

Essas implementações mostraram-se eficazes para resolver um problema real, demonstrando potencial para novos projetos futuros.  

---

## 🚀 **Como Executar o Projeto**  
1. Conecte os componentes conforme as tabelas de pinos e alimentação.  
2. Compile e carregue o código no MSP430F5529 utilizando o CCS (Code Composer Studio).  
3. Ative o sistema pressionando o botão S2.  
4. Utilize um dispositivo Bluetooth para monitorar os alertas.  

---

## 🛡️ **Licença**  
Este projeto é aberto e livre para uso e modificação.  
