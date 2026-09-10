# Riscos Iniciais — NeverDice

**Escala:** B = Baixo; M = Médio; A = Alto. A classificação serve para priorizar a discussão e deve ser revisada quando surgirem novos controles.

## Matriz de riscos

| ID | Ativo | Ameaça | Vulnerabilidade | Exploração | Impacto | Prob. | Nível | Risco | Controles / decisões | Residual |
|----|-------|--------|----------------|-----------|---------|-------|-------|-------|---------------------|----------|
| R-01 | Credenciais Wi-Fi do onboarding | Pessoa próxima ao AP ou log indevido | Página AP/log guarda ou transmite senha sem proteção | Interceptação ou observação do modo AP | Acesso indevido à rede Wi-Fi do usuário | M | A | Alto | Janela curta; não registrar senha; apagar temporários; validar entrada | Limitação do modo AP e do hardware |
| R-02 | Identidade e foto do ESP32-CAM | Dispositivo falso ou clonado | API confia apenas em ID público e aceita foto de origem falsa | Publicação de foto falsa ou de outro dispositivo | Resultado adulterado; perda de confiança | M | A | Alto | Identidade individual; autenticação; TLS; vínculo dispositivo; teste de origem falsa | Comprometimento físico pode permanecer |
| R-03 | Resultado da jogada | Cliente ou processo adulterador | Backend aceita payload inválido ou resultado sem correlação | Envio de resultado falso ou repetido | Histórico de rolagem adulterado | M | A | Alto | Schema; correlação; validação; origem autenticada; auditoria | Erro do modelo de visão não é eliminado |
| R-04 | Tópicos MQTT | Cliente não autorizado | ACL ausente/ampla ou cliente compartilha credencial | Publicação ou assinatura indevida em tópicos de outro dispositivo | Exposição de resultados ou publicação falsa | M | A | Alto | TLS; identidade individual; ACL mínima; sem wildcard amplo | Configuração do broker pode divergir |
| R-05 | Foto/API | Envio abusivo | Upload sem limite, autenticação ou validação de conteúdo | Upload excessivo ou arquivo malicioso | Indisponibilidade da API; processamento indevido | M | M | Médio/Alto | Limite de tamanho; tipo real; rate limit; storage privado; timeout | Monitoramento pode ser necessário |
| R-06 | Logs | Operador ou processo expõe dados | Log registra senha, token, foto completa ou payload MQTT | Consulta ou exfiltração de logs | Exposição de credenciais ou dados pessoais | M | M | Médio | Allowlist de campos; mascaramento; acesso restrito; teste de não vazamento | Configuração de retenção pode falhar |

## Justificativas de priorização

**R-01** é prioritário porque o onboarding recebe credenciais sensíveis (senha Wi-Fi) e o modo AP pode ser observado. Uma falha permite acesso à rede do usuário.

**R-02** é prioritário porque a integridade do resultado depende da autenticidade do dispositivo. Se a API aceitar foto de qualquer origem, o resultado não tem valor.

**R-03** é prioritário porque o resultado é o produto principal do sistema. Um resultado adulterado destrói a confiança de todos os jogadores.

**R-04** é prioritário porque MQTT é o canal de comunicação em tempo real. ACL ausente ou ampla permite que qualquer cliente leia ou publique resultados.

**R-05** tem impacto médio porque afeta disponibilidade e integridade do processamento, mas não compromete diretamente a confidencialidade de dados sensíveis.

**R-06** é relevante porque logs mal configurados podem transformar observabilidade em fonte de vazamento.

## Premissas

- O NeverDice será tratado como sistema externo não confiável por padrão.
- As decisões de autorização ocorrerão no backend/API.
- Segredos não serão armazenados no código-fonte ou no repositório.
- As classificações devem ser revistas após os primeiros testes e após a definição da infraestrutura de implantação.
- Testes ofensivos devem ser executados apenas no ambiente autorizado do projeto.
