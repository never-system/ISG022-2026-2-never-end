# NeverDice — Pitch e Visão do Produto

## Problema

Em sessões online de RPG, jogadores lançam dados físicos que os demais não conseguem verificar. Isso gera desconfiança, reduz a imersão e tira o suspense das rolagens. Digitais puros, por outro lado, não trazem a emoção do toque real do dado.

## Solução

**NeverDice** é uma torre de dados que une o toque físico do dado à praticidade do mundo digital. O jogador lança o dado dentro da torre, o dispositivo detecta o resultado de forma segura e visível para todos, e cada lance mantém o suspense — sem que ninguém precise abaixar a câmera para provar nada.

## Como funciona (resumo)

1. O jogador lança o dado na torre.
2. O sensor SW-420 detecta a vibração e aguarda o dado parar.
3. A ESP32-CAM captura a foto do resultado.
4. O backend processa a imagem com modelo de visão (ChatGPT / Gemini).
5. O resultado chega em tempo real a todos os jogadores via MQTT.

## Público-alvo

Jogadores de RPG online que desejam a autenticidade do dado físico com a comodidade do meio digital.

## Diferencial

- **Resultado verificável e em tempo real** — sem manipulação humana intermediária.
- **Dado físico** — mantém a sensação tátil e o suspense do lance.
- **Integração com mesas online** — resultado entregue via MQTT para web e mobile.

## Status atual

O projeto encontra-se em fase de implementação para a disciplina ISG022. O firmware para ESP32-CAM, a API backend e o cliente web estão em desenvolvimento ativo. A documentação de segurança e arquitetura está sendo construída incrementalmente nas Sprints da disciplina.
