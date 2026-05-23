# Usar a imagem oficial do Bun como base
FROM oven/bun:latest AS builder

# Definir diretório de trabalho
WORKDIR /app

# Copiar arquivos de dependências
COPY package.json bun.lock ./

# Instalar dependências
RUN bun install --frozen-lockfile

# Copiar o código-fonte
COPY . .

# Estágio de produção
FROM oven/bun:latest

# Definir diretório de trabalho
WORKDIR /app

# Copiar dependências do estágio anterior
COPY --from=builder /app/node_modules ./node_modules
COPY --from=builder /app/package.json ./package.json
COPY --from=builder /app/bun.lock ./bun.lock

# Copiar código-fonte
COPY src ./src
COPY tsconfig.json ./tsconfig.json

# Expor porta (deve corresponder à porta da API)
EXPOSE 3000

# Definir variáveis de ambiente padrão (podem ser sobrescritas)
ENV NODE_ENV=production

# Comando para iniciar a aplicação
CMD ["bun", "run", "src/index.ts"]
