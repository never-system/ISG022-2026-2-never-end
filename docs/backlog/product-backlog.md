# Backlog de Produto — NeverDice

## Product Goal

Permitir que jogadores autorizados configurem e utilizem a torre NeverDice conectada, recebam resultados confiáveis do ESP32-CAM por meio da API e do MQTT e acompanhem as rolagens pela web, com acesso mínimo necessário, comunicação protegida e rastreabilidade das ações.

## Atores

| Ator | Objetivo legítimo | Pode fazer | Não pode fazer |
|------|-------------------|------------|----------------|
| **Jogador** | Receber resultados de rolagem em tempo real | Assinar tópicos MQTT; visualizar histórico e animação | Alterar resultado; configurar torre de outra pessoa |
| **Usuário que configura a torre** | Realizar onboarding do ESP32-CAM | Informar nome da torre e credenciais Wi-Fi; reiniciar configuração | Vincular torre já pertencente a outro usuário |
| **ESP32-CAM** | Publicar eventos e fotos; receber comandos | Conectar-se aos canais autorizados; enviar mensagens no contrato definido | Publicar em tópicos de outro dispositivo; decidir autorização de usuário |
| **Backend/API** | Validar identidade, processar fotos e intermediar web e dispositivo | Autenticar/autorizar; validar payloads; persistir; auditar | Confiar no frontend ou em ID enviado pelo cliente |
| **Broker MQTT** | Transportar mensagens entre clientes autorizados | Aceitar conexões e publicar/entregar conforme ACL | Ser tratado como autorização de negócio |
| **Cliente Web** | Apresentar resultados e animações ao usuário | Solicitar dados à API; assinar tópicos permitidos | Ser a única camada de autorização; guardar segredo no navegador |

## Épicos

### Épico 1: Fundação e Infraestrutura
- Setup do repositório, branches, CI e configuração de ambientes
- Definição de Product Goal, atores e escopo
- Baseline de segurança e inventário de ativos

### Épico 2: Firmware e Dispositivo
- Onboarding do ESP32-CAM (modo AP)
- Detecção de vibração e captura de foto
- Comunicação MQTT (eventos e comandos)
- Configuração remota via MQTT

### Épico 3: Backend e API
- Endpoint de recebimento de foto (multipart)
- Armazenamento no S3
- Processamento de imagem com modelo de visão
- Publicação de resultado via MQTT

### Épico 4: Cliente Web
- Conexão MQTT via WebSocket
- Exibição de animação 3D (DiceBox)
- Histórico de rolagens e estatísticas
- Tratamento de erros e reconexão

### Épico 5: Segurança
- Autenticação e autorização
- Proteção de canais (TLS, MQTT ACL)
- Validação de entradas e uploads
- Logging seguro e testes de segurança

## Itens de Backlog

| ID | Épico | Item | Tipo | Prioridade | Estado | Dependências |
|----|-------|------|------|-----------|--------|--------------|
| US-001 | Fundação | Validar Product Goal, atores e escopo do NeverDice | Story | Must | Pronto | — |
| SEC-001 | Fundação | Inventariar ativos com dono, CIA/AUT e classificação | Security | Must | Pronto | US-001 |
| SEC-002 | Fundação | Registrar riscos dos fluxos AP e ESP32/API/MQTT/web | Security | Must | Pronto | SEC-001 |
| ARC-001 | Fundação | Documentar componentes, fluxos e fronteiras de confiança | Task | Must | Pronto | SEC-002 |
| US-002 | Firmware | Configurar ESP32-CAM para detecção de vibração (SW-420) | Story | Must | Em andamento | ARC-001 |
| US-003 | Firmware | Implementar captura de foto e envio via HTTP multipart | Story | Must | Em andamento | US-002 |
| US-004 | Firmware | Configurar tópicos MQTT e publicação de eventos | Story | Must | Em andamento | ARC-001 |
| US-005 | Firmware | Implementar onboarding via modo AP | Story | Must | A fazer | US-002 |
| US-006 | Backend | Criar endpoint de recebimento de foto | Story | Must | Em andamento | ARC-001 |
| US-007 | Backend | Integrar upload para S3 | Story | Must | Em andamento | US-006 |
| US-008 | Backend | Processar imagem com OpenAI GPT-4o-mini | Story | Must | Em andamento | US-007 |
| US-009 | Backend | Publicar resultado via MQTT | Story | Must | A fazer | US-008 |
| US-010 | Web | Conectar ao broker MQTT via WebSocket | Story | Must | Em andamento | ARC-001 |
| US-011 | Web | Exibir animação 3D com DiceBox | Story | Must | Em andamento | US-010 |
| US-012 | Web | Manter histórico de rolagens e estatísticas | Story | Should | Em andamento | US-011 |
| SEC-003 | Segurança | Definir autenticação de usuário e sessão | Security | Must | A fazer | US-001 |
| SEC-004 | Segurança | Autorizar por recurso no backend (usuário + dispositivo) | Security | Must | A fazer | SEC-003 |
| SEC-005 | Segurança | Proteger onboarding AP contra repetição e vazamento | Security | Must | A fazer | US-005 |
| SEC-006 | Segurança | Identificar individualmente cada ESP32 | Security | Must | A fazer | ARC-001 |
| SEC-007 | Segurança | Aplicar TLS e ACL mínima no MQTT | Security | Must | A fazer | SEC-006 |
| SEC-008 | Segurança | Validar schema, tamanho e duplicidade de mensagens | Security | Must | A fazer | US-004, US-009 |
| SEC-009 | Segurança | Cobrir validação, injeção, XSS e rate limit | Security | Must | A fazer | ARC-001 |
| SEC-010 | Segurança | Definir minimização, retenção e descarte de dados | Security | Should | A fazer | SEC-001 |
| SEC-011 | Segurança | Remover segredos do código e configurar varredura | Security | Must | A fazer | ARC-001 |
| TEST-001 | Evidências | Criar testes positivos, negativos e de falha segura | Test | Must | A fazer | SEC-003 a SEC-009 |
| DOC-001 | Defesa | Atualizar riscos residuais, ADRs e checklist | Task | Should | A fazer | Todos os itens Must |

## Critérios de aceite dos itens prioritários

### SEC-001 — Inventário de ativos

- Existem pelo menos cinco ativos específicos do NeverDice.
- Cada ativo possui valor, dono/responsável, CIA/AUT justificada e exemplos.
- O inventário diferencia confirmado de hipótese.

### ARC-001 — Arquitetura e fronteiras

- O diagrama mostra usuário/web, API, dados, broker MQTT, ESP32 e AP.
- Os fluxos de onboarding e de telemetria/comando estão identificados.
- Cada fronteira possui regra de autenticação, validação e autorização.

### SEC-003 — Autenticação

- Usuário com credencial inválida ou sessão expirada recebe negação.
- Senhas são armazenadas apenas com hash adequado.
- Sessões expiram por inatividade e duração máxima.

### SEC-004 — Autorização por recurso

- Usuário A não consulta ou comanda recurso de B.
- A decisão ocorre no backend/API.
- Tentativas negadas geram log sanitizado.

### SEC-007 — MQTT

- Cliente não autenticado não conecta ou não publica/assina.
- Dispositivo A publica/assina apenas seu escopo.
- TLS está documentado para o ambiente real.

## Definition of Ready (DoR)

Um item entra na Sprint quando possui:

- Contexto e resultado compreensíveis.
- Critérios de aceite verificáveis.
- Dependências conhecidas.
- Impacto de segurança identificado.
- Tamanho compatível com a Sprint.
- Dúvidas críticas registradas como hipótese.

## Definition of Done (DoD)

Um item é considerado concluído quando:

- Implementação/documentação está integrada e executável (quando aplicável).
- Critérios de aceite foram atendidos.
- Revisão por outro integrante foi feita.
- Testes relevantes foram executados.
- Nenhum segredo real foi versionado.
- Threat model e documentação foram atualizados.
- Evidência foi anexada à Issue/PR.
- Risco residual foi registrado.

## Roadmap (Sprints ISG022)

| Sprint | Foco | Saída mínima |
|--------|------|-------------|
| 0 | Domínio, atores, Product Goal, repositório e backlog | README, papéis, escopo, 12–20 itens iniciais |
| 1 | Ativos, CIA/AUT, ameaças, vulnerabilidades e baseline | Inventário, riscos iniciais e requisitos |
| 2 | Arquitetura e fronteiras de confiança | Diagrama, threat model e ADRs |
| 3 | Identidade, autorização, sessão | Fluxos, controles server-side e testes negativos |
| 4 | Validação, injeção, uploads, erros e MQTT | Correções e evidências antes/depois |
| 5 | Logging, auditoria, dependências e testes | Logs sanitizados, suíte de testes |
| 6 | Hardening, riscos residuais e defesa | Release candidata e checklist |
