# Referências ISG022 — NeverDice

## Sobre este documento

Este arquivo é o índice de referências da disciplina **ISG022 — Segurança no Desenvolvimento de Aplicações** (FATEC Mauá, 2026/2) utilizadas pelo projeto NeverDice. Os materiais originais da disciplina (PDFs, planilhas, apresentações) **não estão incluídos neste repositório**, a menos que indicado de forma explícita.

## Materiais da disciplina

| Material | Conteúdo principal | Uso no NeverDice |
|----------|-------------------|-----------------|
| **Projeto Integrador — Segurança de Aplicações** | Security by Design incremental; Sprints; papéis; GitHub; artefatos; DoR/DoD; requisitos mínimos; ética e defesa | Roadmap, definição de Sprints, artefatos obrigatórios e critérios de entrega |
| **Oficina — Product Backlog** | Product Goal, atores, épicos, histórias, critérios Given/When/Then, priorização MoSCoW, dependências e rastreabilidade risco–requisito–Issue | Estrutura do backlog, critérios de aceite e organização do trabalho |
| **Guia Prático — Security Baseline e Arquitetura Segura** | Ativos específicos, CIA/AUT, sequência de cenários de risco, princípios de projeto seguro, fronteiras de confiança e riscos residuais | Construção do inventário de ativos, baseline de segurança e threat model |
| **CookBook Git, Python e C** | Branches curtas, commits rastreáveis, Pull Requests, revisão, `.gitignore`, configuração externa, logging seguro, testes | Fluxo de trabalho, higiene do repositório e boas práticas de versionamento |

## Referências externas indicadas

- [OWASP Threat Modeling Cheat Sheet](https://cheatsheetseries.owasp.org/cheatsheets/Threat_Modeling_Cheat_Sheet.html)
- [OWASP ASVS (Application Security Verification Standard)](https://owasp.org/www-project-application-security-verification-standard/)
- [GitHub Issues](https://docs.github.com/en/issues)
- [GitHub Projects](https://docs.github.com/en/issues/planning-and-tracking-with-projects)
- [OWASP Cheat Sheet Series](https://cheatsheetseries.owasp.org/)

## Artefatos derivados no repositório

Os seguintes documentos foram derivados dos materiais da disciplina e das notas do projeto:

| Artefato | Caminho | Origem |
|----------|---------|--------|
| Arquitetura v1 | [docs/architecture/architecture-v1.md](../../architecture/architecture-v1.md) | Escopo, diagramas de fluxo e sequência |
| Backlog de produto | [docs/backlog/product-backlog.md](../../backlog/product-backlog.md) | Plano de Segurança, Oficina de Product Backlog |
| Security Baseline | [docs/security/security-baseline.md](../../security/security-baseline.md) | Guia Prático, Plano de Segurança |
| Riscos iniciais | [docs/security/initial-risks.md](../../security/initial-risks.md) | Plano de Segurança, escopo |
| Plano de segurança | [docs/security/security-plan.md](../../security/security-plan.md) | Plano de Segurança ISG022 |

## Atribuição e direitos autorais

Os materiais da disciplina ISG022 são propriedade da **FATEC Mauá** e dos respectivos autores. Eles são utilizados neste projeto exclusivamente para fins educacionais, no contexto da disciplina. Não há intenção de redistribuição ou uso comercial.

Se você é aluno de outra instituição ou turma, consulte o professor responsável antes de utilizar estes materiais como referência.

## Como adicionar novas referências

1. Inclua o material em `docs/references/isg022/` (se autorizado pelo professor).
2. Atualize a tabela acima com o nome, conteúdo e uso no projeto.
3. Registre em qual artefato do repositório o material foi utilizado.
4. Não inclua credenciais, dados pessoais ou informações sensíveis do material original.
