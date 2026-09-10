# NeverDice

Torre de dados inteligente para RPG — detecta o resultado de um dado físico e transmite em tempo real para jogadores online via MQTT.

Desenvolvido para a disciplina **ISG022 — Segurança no Desenvolvimento de Aplicações** (FATEC Mauá, 2026/2).

## Visão do produto

NeverDice une o toque real do dado à praticidade do mundo digital. O jogador lança o dado na torre, o dispositivo detecta o resultado de forma segura e visível para todos — sem que ninguém precise abaixar a câmera para provar nada.

## Como funciona

1. Jogador lança dado na torre.
2. Sensor SW-420 detecta vibração e aguarda o dado parar.
3. ESP32-CAM captura foto do resultado.
4. Backend processa a imagem com modelo de visão (GPT-4o-mini).
5. Resultado chega em tempo real a todos via MQTT.

## Stack

| Camada | Tecnologia |
|--------|-----------|
| Firmware | ESP32-CAM, Arduino, PlatformIO, SW-420 |
| Backend | Bun, Elysia.js, TypeScript, OpenAI API, S3 |
| Frontend | React, TypeScript, Vite, MQTT.js, DiceBox (3D) |
| Comunicação | MQTT (WSS), HTTP/HTTPS (multipart) |

## Estrutura do repositório

```text
never-end/
├── README.md
├── docs/
│   ├── architecture/       # Arquitetura e diagramas
│   ├── backlog/            # Backlog de produto e segurança
│   ├── product/            # Pitch e visão do produto
│   ├── references/isg022/  # Referências da disciplina
│   └── security/           # Baseline, riscos e plano de segurança
├── apps/web/               # Cliente web (React + Vite)
├── services/api/           # API backend (Bun + Elysia.js)
├── fimware/esp32-cam/      # Firmware ESP32-CAM (PlatformIO)
└── tools/detection-cli/    # CLI de detecção de dado (Rust + OpenCV)
```

## Documentação

| Documento | Caminho |
|-----------|---------|
| Índice da documentação | [docs/README.md](docs/README.md) |
| Arquitetura | [docs/architecture/architecture-v1.md](docs/architecture/architecture-v1.md) |
| Backlog | [docs/backlog/product-backlog.md](docs/backlog/product-backlog.md) |
| Pitch | [docs/product/pitch.md](docs/product/pitch.md) |
| Security Baseline | [docs/security/security-baseline.md](docs/security/security-baseline.md) |
| Riscos Iniciais | [docs/security/initial-risks.md](docs/security/initial-risks.md) |
| Plano de Segurança | [docs/security/security-plan.md](docs/security/security-plan.md) |
| Referências ISG022 | [docs/references/isg022/README.md](docs/references/isg022/README.md) |

## Equipe

| Papel | Integrante |
|-------|------------|
| Product Manager | Enzo Luz Granato Barbosa |
| Tech Lead | Gabriel Menezes da Silva |
| Team Members | Giulia Gomes Grossi; João Victor de Sousa; Matheus Nascimento de Holanda |

## Licença

Projeto acadêmico — uso educacional. Materiais da disciplina ISG022 são propriedade da FATEC Mauá e dos respectivos autores.
