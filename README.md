# Rastreamento Inercial
Desenvolvimento de um sistema de rastreamento inercial.

Coloquei neste repositório os códigos que estou escrevendo.

## Arquivos do repositório

* **Arduino**: *"Sketches"*  para serem enviados para o arduino.
    * Leitor Sensores: Sketches de exemplo para leitura. Não será usado no final.
    * **Host1Sensor**: Ainda em desenvolvimento, será utilizado para receber os dados de 1 board nRF24LE1 conectada aos sensores e, então, os enviar para o computador via Serial.
* **Keil**: Somente o código em C, que planejo usar no 8051 do nRF24LE1. Ainda há muitos erros e coisas que não entendo.
    * **DMP-MPU6050**: Métodos e funções que são utilizadas pelo arduino para a configuração e comunicação com o DMP do MPU6050. Eles estão basicamente listados, aos poucos estou lendo as bibliotecas do I2Cdev e as escrevendo. **Falta muita coisa** ainda.
    * Exemplos: alguns exemplos que encontrei na internet de como usar o nRF24LE1.
    * **readSensores***: Códigos para leitura de dados dos sensores, por enquanto está na forma mais simples e ainda vou fazer a parte da comunicação utilizando o rádio.

* **Python**: Um data-logger bem útil pra salvar algumas leituras se for preciso. Há, também, algumas leituras salvas e plotadas no matlab.

* **nRF24LE1**: Algumas informações e dúvidas sobre o nRF24LE1.
