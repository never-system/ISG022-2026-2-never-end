# Plano de Segurança — NeverDice

> **Finalidade:** orientar o grupo na análise, priorização, implementação e demonstração dos controles de segurança do NeverDice durante o Projeto Integrador de ISG022.
>
> **Regra de leitura:** este documento separa fatos de hipóteses. Tudo marcado como **Confirmado** vem das notas ou do código disponível. Tudo marcado como **Proposto — validar** é uma decisão sugerida para o grupo confirmar antes de implementar.

## Legenda

| Marca | Significado | Ação |
|-------|------------|------|
| **Confirmado** | Presente nas notas originais ou no código. | Pode orientar o trabalho; relacionar ao código quando aplicável. |
| **Proposto — validar** | Hipótese ou controle recomendado. | Confirmar com grupo, código e professor; depois decidir ou registrar aceitação do risco. |
| **Evidência a obter** | Prova que ainda precisa ser produzida. | Vincular a Issue, PR, teste ou decisão. |

---

## 1. Escopo

### 1.1 Dentro do escopo

- Onboarding do dispositivo (modo AP), incluindo configuração e troca de credenciais Wi-Fi.
- Identidade e autorização de usuários, dispositivos e serviços.
- Firmware/configuração do ESP32-CAM e armazenamento local.
- API e seus endpoints, validação, respostas, tratamento de erro e rate limit.
- Broker MQTT, clientes, tópicos, ACLs, QoS e mensagens.
- Aplicação web e exposição de dados no navegador.
- Uploads (foto do dado) e storage.
- Logs, métricas e testes de segurança.

### 1.2 Fora do escopo inicial

- Segurança física do local de instalação do ESP32.
- Certificação industrial ou homologação regulatória.
- Segurança de redes e serviços de terceiros.
- Pentest de infraestrutura institucional sem autorização.
- Alta disponibilidade de produção e SOC 24×7.

---

## 2. Inventário de ativos

| ID | Ativo | Valor | Dono | CIA/AUT | Estado |
|----|-------|-------|------|---------|--------|
| A-01 | Credenciais Wi-Fi do onboarding | Permitem colocar o ESP32 na rede | Usuário + firmware | C, AUT | Confirmado |
| A-02 | Identidade do ESP32-CAM | Confiança na origem da foto e evento | Firmware/operação | AUT, I, C | Confirmado |
| A-03 | Foto do dado | Comprova o lance | API + S3 | C, I, D | Confirmado |
| A-04 | Resultado digital da jogada | Produto principal do sistema | Backend + clientes | I, D, AUT | Confirmado |
| A-05 | Mensagens MQTT | Transporte em tempo real | Broker/API | C, I, D, AUT | Confirmado |
| A-06 | API de recebimento da foto | Entrada do backend | Backend | I, D, AUT | Confirmado |
| A-07 | Código-fonte e dependências | Lógica e configurações | Equipe/GitHub | I, C, AUT | Confirmado |

---

## 3. Atores e permissões

| Ator | Estado | Objetivo legítimo | Pode fazer | Não pode fazer |
|------|--------|-------------------|------------|----------------|
| Jogador | Proposto — validar | Receber resultado em tempo real | Assinar tópicos MQTT; ver histórico | Alterar resultado; configurar torre alheia |
| Usuário que configura | Proposto — validar | Onboarding do ESP32 autorizado | Informar nome e Wi-Fi da torre | Vincular torre de outro proprietário |
| ESP32-CAM | Confirmado | Enviar evento, foto e receber comando | Publicar no escopo próprio | Publicar em tópicos de outro dispositivo |
| Backend/API | Confirmado | Validar, processar e intermediar | Autenticar, autorizar, validar payloads | Confiar em ID enviado pelo cliente |
| Cliente Web | Confirmado | Receber resultado e exibir animação | Assinar tópicos permitidos | Ser autoridade de acesso |
| Broker MQTT | Confirmado | Transportar mensagens | Conformar ACL | Ser autorização de negócio |

---

## 4. Fronteiras de confiança

| ID | Fronteira | Dados | Validações |
|----|-----------|-------|-----------|
| F-01 | Navegador → AP do ESP32 | Nome da torre e Wi-Fi | Janela limitada; não logar senha; validar tamanho |
| F-02 | ESP32 → API | Foto e métricas | Autenticação do dispositivo; limite; tipo; timeout |
| F-03 | ESP32 ↔ Broker MQTT | Eventos e comandos | Identidade individual; ACL; tópicos mínimos |
| F-04 | API → S3 | Foto armazenada | Bucket privado; permissão mínima; URL com expiração |
| F-05 | API → Cliente Web | Resultado | Autenticação se não for público |
| F-06 | Código/CI → Ambiente | Configurações e segredos | Segredos fora do Git; revisão |

---

## 5. Matriz de riscos e controles

| ID | Ativo | Ameaça | Vulnerabilidade | Prob. | Impacto | Nível | Tratamento | Controles | Residual |
|----|-------|--------|----------------|-------|---------|-------|-----------|----------|----------|
| R-01 | A-01 Credenciais Wi-Fi | Interceptação no onboarding | AP transmite ou persiste senha sem proteção | M | A | Alto | Mitigar | Janela curta; não logar; apagar após uso | Limitação do hardware |
| R-02 | A-02 Identidade ESP32 | Dispositivo falso | API aceita ID público sem credencial | M | A | Alto | Mitigar | Identidade individual; autenticação; ACL | Comprometimento físico |
| R-03 | A-04 Resultado | Adulteração | Backend aceita payload inválido | M | A | Alto | Mitigar | Schema; correlação; validação | Erro de modelo de visão |
| R-04 | A-05 MQTT | Publicação indevida | ACL ampla ou ausente | M | A | Alto | Mitigar | TLS; identidade; ACL mínima | Configuração do broker |
| R-05 | A-03 Foto/API | Upload abusivo | Sem limite ou validação | M | M | Médio/Alto | Mitigar | Limite; tipo; rate limit; timeout | Monitoramento |
| R-06 | Logs | Vazamento de dados | Log registra dados sensíveis | M | M | Médio | Mitigar | Allowlist; mascaramento; acesso restrito | Configuração de retenção |

---

## 6. Threat model — Onboarding AP

### 6.1 Fluxo

1. ESP32 inicia em modo AP.
2. Usuário conecta-se à rede do ESP32.
3. Acessa página de configuração no navegador.
4. Informa nome da torre e credenciais Wi-Fi.
5. ESP32 salva configuração e conecta à rede principal.
6. Dispositivo pronto para uso normal.

### 6.2 Ameaças e controles

| ID | Ameaça | Vulnerabilidade | Controle proposto | Teste |
|----|--------|----------------|-------------------|-------|
| AP-T1 | Pessoa assume modo de configuração | Modo aberto, janela infinita | Janela curta; autenticação; bloqueio após falhas | Tentar configurar sem auth e após timeout |
| AP-T2 | Observação da senha | Senha em log, URL ou resposta | Canal protegido; mascaramento; exclusão após uso | Inspecionar logs e respostas com dado fictício |
| AP-T3 | Configuração repetida | Mensagem reutilizável | Nonce/expiração; uso único; invalidação após sucesso | Repetir operação e comprovar recusa |
| AP-T4 | Dispositivo de outro dono é vinculado | ID público previsível | Pareamento autenticado; confirmação do proprietário | Usuário A tenta vincular dispositivo de B |
| AP-T5 | Entrada causa abuso | SSID/senha sem limite | Tamanho; charset; timeout; retorno genérico | Enviar entradas vazias e grandes |
| AP-T6 | Exposição após sucesso | Credenciais permanecem na memória | Encerrar modo AP; apagar temporários | Verificar estado após sucesso e reset |

---

## 7. Threat model — Fluxo ESP32 → API → MQTT → Web

### 7.1 Fluxo

1. ESP32 detecta vibração (SW-420) e aguarda dado parar.
2. ESP32 captura foto e envia para a API via HTTP POST.
3. API valida, armazena no S3 e processa com modelo de visão.
4. API publica resultado via MQTT.
5. Cliente web recebe resultado e exibe animação.

### 7.2 Ameaças e controles

| ID | Ameaça | Vulnerabilidade | Controle proposto | Teste |
|----|--------|----------------|-------------------|-------|
| ESP-T1 | Falsificação de ESP32 | ID sem credencial | Identidade individual; autenticação | Cliente fictício tenta publicar como outro dispositivo |
| ESP-T2 | Foto adulterada | API aceita qualquer origem | Autenticação; schema; vínculo | Enviar foto de dispositivo não autorizado |
| ESP-T3 | Resultado falso | Backend aceita payload inválido | Schema; correlação; validação | Enviar resultado sem correlação com foto |
| ESP-T4 | MQTT indevido | ACL ampla ou wildcard | ACL mínima por dispositivo; tópicos sem segredo | Testar publish/permitido e negado |
| ESP-T5 | Replay de comando | Mensagem válida repetida | ID único; expiração; idempotência | Reenviar comando e comprovar ausência de efeito duplicado |
| ESP-T6 | Vazamento no navegador | Web conecta diretamente ao broker | API como mediadora; sessão mínima | Inspecionar rede do navegador |
| ESP-T7 | XSS | Dados exibidos sem encoding | Validação; encoding de saída; schema | Enviar marcador fictício e comprovar tratamento como texto |
| ESP-T8 | Indisponibilidade | Mensagens grandes ou rápidas | Rate limit; quota; timeout; backoff | Ensaio controlado de volume |

---

## 8. Plano de implementação incremental

### 8.1 Ordem recomendada

1. **Fundação:** escopo, atores, backlog, `.gitignore`, configuração externa.
2. **Contrato seguro:** definir mensagens, estados, tamanhos, erros e correlação.
3. **Identidade:** decidir como usuário, serviço e ESP32 serão identificados.
4. **Autorização:** decidir quem pode configurar, enviar foto e receber resultado.
5. **Transporte:** proteger HTTP e MQTT com TLS.
6. **Validação:** limitar entradas, fotos, eventos e parâmetros.
7. **Fluxo funcional:** integrar sem retirar controles.
8. **Logs e erros:** observar sem vazar dados.
9. **Testes:** cobrir permitido, negado, inválido e falha.
10. **Hardening:** dependências, configurações e risco residual.

### 8.2 Roadmap de Sprints

| Sprint | Foco | Saída |
|--------|------|-------|
| 0 | Fundação e backlog | README, papéis, escopo, 12–20 itens |
| 1 | Ativos, riscos e baseline | Inventário, riscos, requisitos |
| 2 | Arquitetura e fronteiras | Diagrama, threat model, ADRs |
| 3 | Identidade, autorização, sessão | Fluxos, controles, testes negativos |
| 4 | Validação, uploads, MQTT | Correções e evidências |
| 5 | Logs, testes e auditoria | Logs sanitizados, suíte de testes |
| 6 | Hardening e defesa | Release candidata, checklist |

---

## 9. Checklist de entrega

### 9.1 Conteúdo técnico

- [ ] Product Goal validado com o grupo.
- [ ] Escopo dentro/fora documentado.
- [ ] Atores e permissões aprovados.
- [ ] Inventário com pelo menos cinco ativos específicos.
- [ ] Arquitetura com componentes reais, dois fluxos e fronteiras.
- [ ] Onboarding AP testado ou marcado como hipótese.
- [ ] Fluxo ESP32/API/MQTT/web desenhado nas direções reais.
- [ ] Autenticação, autorização e sessão especificados.
- [ ] Dados, segredos, TLS, MQTT e uploads com regras documentadas.
- [ ] Logs não expõem credenciais.
- [ ] Riscos com ameaça, vulnerabilidade, exploração, impacto, tratamento e residual.

### 9.2 Segredos e higiene

- [ ] Não há credencial real em arquivos, histórico ou Issues.
- [ ] Configuração local está fora do versionamento.
- [ ] Foi feita revisão do diff e secret scanning (se disponível).

### 9.3 Defesa

- [ ] A equipe consegue explicar Product Goal e atores.
- [ ] A equipe consegue narrar os dois fluxos e as fronteiras.
- [ ] A equipe demonstra acesso permitido e negação.
- [ ] A equipe explica o que está confirmado, proposto e residual.

---

## 10. Referências

- Ver [docs/references/isg022/README.md](../references/isg022/README.md) para os materiais da disciplina.
- [OWASP Threat Modeling Cheat Sheet](https://cheatsheetseries.owasp.org/cheatsheets/Threat_Modeling_Cheat_Sheet.html)
- [OWASP ASVS](https://owasp.org/www-project-application-security-verification-standard/)
