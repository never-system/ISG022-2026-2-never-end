# Security Baseline — NeverDice

**Projeto:** NeverDice — Torre de dados inteligente
**Disciplina:** ISG-022 — Segurança no Desenvolvimento de Aplicações
**Data:** 04/09/2026
**Versão:** 1.0 — baseline inicial

## 1. Contexto do sistema

NeverDice é uma torre de dado físico que detecta o lançamento de um dado, captura a imagem do resultado, processa com modelo de visão artificial e transmite o resultado em tempo real para os jogadores via MQTT. O sistema integra ESP32-CAM (firmware), API backend (Bun + Elysia.js), broker MQTT e cliente web (React).

O objetivo desta baseline é registrar o que precisa ser protegido, os riscos iniciais plausíveis e as decisões de arquitetura que devem orientar a evolução do produto. Ela não representa a implementação completa de todos os controles.

## 2. Papéis e responsabilidades

| Papel | Responsabilidade nesta baseline |
|-------|-------------------------------|
| Product Manager | Identificar ativos mais valiosos e priorizar riscos. |
| Tech Lead | Definir fronteiras de confiança, decisões arquiteturais e controles técnicos. |
| Team Members | Levantar ativos e riscos, propor controles e revisar documentação. |

## 3. Ativos prioritários

| ID | Ativo | Valor para o projeto | CIA/AUT mais relevante | Responsável/componente |
|----|-------|---------------------|----------------------|----------------------|
| A-01 | Credenciais Wi-Fi do onboarding | Permitem colocar o ESP32-CAM na rede | C, AUT | Usuário + firmware |
| A-02 | Identidade do ESP32-CAM | Permite confiar na origem da foto e do evento MQTT | AUT, I, C | Firmware/operação |
| A-03 | Foto do dado | Pode comprovar o lance e contém imagem capturada pela torre | C, I, D | API + S3 |
| A-04 | Resultado digital da jogada | É o dado que todos os jogadores usam | I, D, AUT | Backend + clientes |
| A-05 | Mensagens MQTT/tópicos | Transportam evento e resultado em tempo real | C, I, D, AUT | Broker/API |
| A-06 | API de recebimento da foto | É a entrada do backend para o processamento | I, D, AUT | Backend |
| A-07 | Código-fonte e dependências | Contêm lógica, dependências e nomes de ambientes | I, C, AUT | Equipe/GitHub |

## 4. Princípios de projeto seguro adotados

### 4.1 Menor privilégio

Cada componente (usuário, ESP32, API, cliente web) tem somente as ações estritamente necessárias. O ESP32 publica apenas no escopo da sua torre; o cliente web assina apenas tópicos permitidos; a API expõe apenas endpoints necessários.

### 4.2 Defesa em profundidade

A proteção não depende apenas de uma camada. TLS protege o transporte; autenticação valida identidade; autorização verifica permissão; validação rejeita entradas inválidas; logs registram negações sem expor dados sensíveis.

### 4.3 Redução da superfície de ataque

A API expõe apenas o endpoint de processamento de imagem. Tópicos MQTT são nomeados por dispositivo. Operações administrativas não estão disponíveis para usuários comuns. Portas e serviços desnecessários não são publicados.

### 4.4 Secure by default

Novos dispositivos começam sem permissões extras. O acesso é negado até que o vínculo e a permissão sejam explicitamente confirmados. Logs não contêm senhas ou tokens por padrão.

### 4.5 Fail secure

Em caso de falha de autenticação, validação ou indisponibilidade do broker, o sistema bloqueia a operação protegida. O ESP32 não é aceito como origem confiável quando a validação do evento falhar.

### 4.6 Fronteiras de confiança

As principais fronteiras são: Internet ↔ cliente web; cliente web ↔ API; API ↔ broker MQTT; broker ↔ ESP32; ESP32 ↔ rede local (onboarding). Toda informação que cruza uma fronteira deve ser autenticada, autorizada e validada.

## 5. Decisões e controles vinculados aos riscos

| ID | Risco tratado | Princípio | Decisão de projeto | Controle/evidência | Risco residual |
|----|--------------|-----------|-------------------|-------------------|---------------|
| C-01 | Foto falsa ou de dispositivo não autorizado é aceita pela API | Fronteira de confiança | API valida identidade do dispositivo antes de processar foto | Autenticação por dispositivo; rejeição de origem desconhecida; teste de dispositivo falso | Comprometimento físico do dispositivo pode permanecer |
| C-02 | Resultado adulterado é exibido para jogadores | Integridade + autenticidade | Backend valida origem e integridade do resultado antes de publicar | Schema; correlação foto–resultado; validação de payload; teste de resultado inválido | Erro do modelo de visão não é eliminado totalmente |
| C-03 | Senha Wi-Fi aparece em log ou resposta | Redução da superfície de ataque + secure by default | Logs registram apenas "configuração recebida"; nunca o valor da senha | Allowlist de campos; mascaramento; teste de não vazamento | Erro de configuração pode permanecer |
| C-04 | Cliente não autorizado recebe ou publica mensagens MQTT | Menor privilégio | ACL por dispositivo; tópicos com escopo mínimo; autenticação individual | ACL; identidade por cliente; teste de publicação indevida | Configuração do broker pode divergir por ambiente |
| C-05 | Upload excessivo ou malicioso derruba a API | Defesa em profundidade + fail secure | Limite de tamanho; tipo real; timeout; storage fora de área executável | Validação de upload; rate limit; teste com arquivo inválido | Ataque de disponibilidade pode exigir monitoramento |

## 6. Riscos residuais e premissas

As decisões acima reduzem os riscos, mas não eliminam vulnerabilidades. A baseline assume que:

- O repositório possui controle de acesso.
- Segredos não serão commitados.
- O broker terá credenciais separadas por ambiente.
- O ESP32 poderá ser identificado e revogado.
- Testes são executados apenas no ambiente autorizado do projeto.

## 7. Próximos passos

1. Atualizar o diagrama de arquitetura com as fronteiras de confiança e os controles principais.
2. Criar cenários detalhados de risco com justificativas (ver [Riscos Iniciais](initial-risks.md)).
3. Implementar autorização por recurso antes de liberar resultados.
4. Definir mecanismo de autenticação e expiração de sessão.
5. Definir autenticação, idempotência e revogação para o NeverDice.
6. Adicionar testes de acesso permitido, acesso negado, sessão inválida e payload inválido.
7. Revisar o documento por outro integrante e registrar decisões no GitHub.
