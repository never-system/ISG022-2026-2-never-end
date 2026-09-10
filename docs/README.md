# Documentação do NeverDice

Esta pasta reúne toda a documentação técnica e de segurança do projeto **NeverDice**, desenvolvido para a disciplina **ISG022 — Segurança no Desenvolvimento de Aplicações** (FATEC Mauá, 2026/2).

## Estrutura

```text
docs/
├── README.md                        # Este arquivo
├── architecture/
│   └── architecture-v1.md           # Arquitetura do sistema, componentes e trust boundaries
├── backlog/
│   └── product-backlog.md           # Backlog de produto e segurança
├── product/
│   └── pitch.md                     # Pitch e visão do produto
├── references/
│   └── isg022/
│       └── README.md                # Referências e materiais da disciplina ISG022
└── security/
    ├── security-baseline.md         # Baseline de segurança e controles
    ├── initial-risks.md             # Registro inicial de riscos
    └── security-plan.md             # Plano de segurança detalhado
```

## Resumo dos documentos

| Documento | Conteúdo |
|-----------|----------|
| [Arquitetura](architecture/architecture-v1.md) | Diagramas de contexto, componentes, fluxos e fronteiras de confiança |
| [Backlog](backlog/product-backlog.md) | Product Goal, atores, épicos, itens de backlog e critérios de aceite |
| [Pitch](product/pitch.md) | Visão geral, problema e valor para o jogador |
| [Referências ISG022](references/isg022/README.md) | Índice dos materiais da disciplina e atribuições |
| [Security Baseline](security/security-baseline.md) | Ativos prioritários, princípios de projeto seguro e controles vinculados a riscos |
| [Riscos Iniciais](security/initial-risks.md) | Matriz de riscos iniciais com probabilidades, impactos e tratamentos |
| [Plano de Segurança](security/security-plan.md) | Plano completo de segurança incremental, com controles, testes e evidências |

## Como manter esta documentação

1. **Atualizar ao mudar de comportamento.** Toda alteração em fluxo, ator, endpoint, tópico MQTT ou upload deve provocar a pergunta: _qual documento precisa mudar?_
2. **Separar fato de hipótese.** Marque explicitamente o que está confirmado no código/notas e o que ainda é proposta a validar.
3. **Não versionar segredos.** Nunca inclua senhas, tokens, chaves ou credenciais reais em nenhum arquivo de documentação.
4. **Usar Markdown padrão do GitHub.** Evite sintaxe exclusiva do Obsidian (wikilinks, embeds, callouts). Use links relativos.
5. **Referenciar por caminho.** Links entre documentos devem ser relativos (ex.: `../security/initial-risks.md`).
6. **Manter consistência de IDs.** Riscos (`R-XX`), ativos (`A-XX`), fronteiras (`F-XX`) e testes (`T-XX`) devem manter IDs estáveis entre documentos.
7. **Revisar por Pull Request.** Alterações em documentos de segurança devem ser revisadas por outro integrante antes do merge.
