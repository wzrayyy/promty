FROM golang:1.23-alpine AS builder

WORKDIR /build
COPY . .

RUN --mount=type=cache,target=/go/pkg/mod go build;


FROM alpine AS runner

WORKDIR /app
COPY --from=builder /build/proxy .

EXPOSE 80/tcp
CMD ./proxy
