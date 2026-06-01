# ─────────────────────────────────────────────────────────────────────────────
# Stage 1: Build the Go server binary
# Using the official Go 1.24 image on Alpine for minimal size.
# ─────────────────────────────────────────────────────────────────────────────
FROM golang:1.24-alpine AS builder

# Install build dependencies
RUN apk add --no-cache git ca-certificates tzdata

WORKDIR /build

# Cache module downloads separately from source code for faster rebuilds
COPY go.mod go.sum ./
RUN go mod download

# Copy source and build — strip debug info with -s -w for smaller binary
COPY . .
RUN CGO_ENABLED=0 GOOS=linux GOARCH=amd64 \
    go build \
    -ldflags="-s -w -extldflags '-static'" \
    -trimpath \
    -o /eduerp-server \
    ./cmd/api

# ─────────────────────────────────────────────────────────────────────────────
# Stage 2: Production image — distroless for minimal attack surface
# No shell, no package manager, no unnecessary tools.
# ─────────────────────────────────────────────────────────────────────────────
FROM gcr.io/distroless/static-debian12:nonroot

# Copy only what's needed
COPY --from=builder /eduerp-server /eduerp-server
COPY --from=builder /usr/share/zoneinfo /usr/share/zoneinfo
COPY --from=builder /etc/ssl/certs/ca-certificates.crt /etc/ssl/certs/

# Run as non-root user (uid 65532 = nonroot in distroless)
USER nonroot:nonroot

# Expose the API port
EXPOSE 8080

# Health check — Docker will mark unhealthy after 3 consecutive failures
HEALTHCHECK --interval=30s --timeout=5s --start-period=10s --retries=3 \
    CMD ["/eduerp-server", "-healthcheck"] || exit 1

ENTRYPOINT ["/eduerp-server"]
