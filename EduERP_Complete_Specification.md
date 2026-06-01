# EduERP - Complete Architecture & Development Specification

## Educational ERP Simulation Desktop Application for Belgian Secondary Schools

**Version:** 1.0.0  
**Date:** March 2026  
**Target Platform:** Windows 10/11  
**Primary Language:** Dutch (nl-BE)  
**Secondary Languages:** English (en-GB), French (fr-BE)

---

# PART 1: ARCHITECTURE DOCUMENT

## 1.1 System Overview

EduERP is a multi-tier distributed application consisting of:

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              CLIENT TIER                                     │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                    EduERP Desktop Application                        │   │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐ │   │
│  │  │  UI Layer   │  │  Business   │  │   Sync      │  │   Local     │ │   │
│  │  │  (Qt6/QML)  │  │    Logic    │  │   Engine    │  │   Cache     │ │   │
│  │  └─────────────┘  └─────────────┘  └─────────────┘  └─────────────┘ │   │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐ │   │
│  │  │ Simulation  │  │   Auth      │  │  WebSocket  │  │   SQLite    │ │   │
│  │  │   Engine    │  │   Manager   │  │   Client    │  │   Cache     │ │   │
│  │  └─────────────┘  └─────────────┘  └─────────────┘  └─────────────┘ │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                              Windows 10/11                                   │
└─────────────────────────────────────────────────────────────────────────────┘
                                      │
                                      │ HTTPS (TLS 1.3)
                                      │ WebSocket (WSS)
                                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                           GATEWAY TIER                                       │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                    Google Cloud Load Balancer                        │   │
│  │              (SSL Termination, DDoS Protection)                      │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────────────┘
                                      │
                                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                           APPLICATION TIER                                   │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                    Cloud Run / Compute Engine                        │   │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐ │   │
│  │  │   REST API  │  │  WebSocket  │  │   Auth      │  │  Background │ │   │
│  │  │   Server    │  │   Server    │  │ Middleware  │  │   Workers   │ │   │
│  │  │   (Go/Gin)  │  │   (Go/Gorilla)│  │   (JWT)     │  │   (Go)      │ │   │
│  │  └─────────────┘  └─────────────┘  └─────────────┘  └─────────────┘ │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────────────┘
                                      │
                                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                             DATA TIER                                        │
│  ┌─────────────────────────┐  ┌─────────────────────────────────────────┐  │
│  │   Google Cloud SQL      │  │         Google Cloud Storage            │  │
│  │     (PostgreSQL 17)     │  │         (File Storage)                  │  │
│  │  ┌─────────────────┐   │  │  ┌─────────┐ ┌─────────┐ ┌─────────┐   │  │
│  │  │  Primary DB     │   │  │  │ Avatars │ │ Banners │ │  PDF    │   │  │
│  │  │  (Multi-tenant) │   │  │  │         │ │         │ │Exports  │   │  │
│  │  └─────────────────┘   │  │  └─────────┘ └─────────┘ └─────────┘   │  │
│  │  ┌─────────────────┐   │  └─────────────────────────────────────────┘  │
│  │  │  Read Replica   │   │                                               │
│  │  │  (Optional)     │   │                                               │
│  │  └─────────────────┘   │                                               │
│  └─────────────────────────┘                                               │
│  ┌─────────────────────────┐                                               │
│  │    Secret Manager       │                                               │
│  │  (OAuth secrets, DB     │                                               │
│  │   credentials, JWT keys)│                                               │
│  └─────────────────────────┘                                               │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 1.2 Component Communication Architecture

### 1.2.1 Client-Server Communication Patterns

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        COMMUNICATION PATTERNS                                │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  1. REST API (Request-Response)                                             │
│  ═══════════════════════════════                                            │
│                                                                             │
│     Client                    Server                                        │
│       │    ──── HTTPS GET ────▶    │                                        │
│       │    ◀─── JSON Response ───  │                                        │
│       │                         │                                        │
│       │    ─── HTTPS POST ─────▶   │                                        │
│       │    ◀─── 201 Created ─────  │                                        │
│                                                                             │
│  Use Cases: Authentication, CRUD operations, file uploads                   │
│  Headers: Authorization: Bearer <JWT>, X-Client-Version: x.y.z              │
│                                                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  2. WebSocket (Real-time Bidirectional)                                     │
│  ═══════════════════════════════════════                                    │
│                                                                             │
│     Client                    Server                                        │
│       │    ─── WSS Handshake ──▶   │                                        │
│       │    ◀── 101 Switching ───   │                                        │
│       │                         │                                        │
│       │    ◀─── Team Update ─────  │                                        │
│       │    ──── User Action ────▶  │                                        │
│       │    ◀─── Chat Message ────  │                                        │
│       │    ──── Typing Indicator ▶ │                                        │
│                                                                             │
│  Use Cases: Real-time collaboration, chat, simulation sync                  │
│  Heartbeat: 30-second ping/pong to detect disconnections                    │
│  Reconnection: Exponential backoff (1s, 2s, 4s, 8s, max 30s)                │
│                                                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  3. Server-Sent Events (One-way Server → Client)                            │
│  ═════════════════════════════════════════════════                          │
│                                                                             │
│     Client                    Server                                        │
│       │    ──── SSE Connect ────▶   │                                        │
│       │    ◀─── event: notification │                                        │
│       │    ◀─── data: {...}         │                                        │
│       │    ◀─── event: simulation   │                                        │
│       │    ◀─── data: {...}         │                                        │
│                                                                             │
│  Use Cases: Notifications, simulation events, announcements                 │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 1.2.2 Message Flow Diagrams

#### Authentication Flow (OAuth 2.0 + PKCE)

```
┌─────────┐     ┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│  User   │     │  EduERP     │     │   Backend   │     │   Google/   │
│         │     │   Client    │     │   Server    │     │  Microsoft  │
└────┬────┘     └──────┬──────┘     └──────┬──────┘     └──────┬──────┘
     │                 │                   │                   │
     │  Click "Login"  │                   │                   │
     │────────────────▶│                   │                   │
     │                 │  1. Generate PKCE │                   │
     │                 │     (code_verifier, code_challenge)   │
     │                 │                   │                   │
     │                 │  2. Open browser/system browser       │
     │                 │──────────────────────────────────────▶│
     │                 │                   │                   │
     │                 │◀──────────────────────────────────────│
     │                 │   Authorization Code + PKCE params    │
     │                 │                   │                   │
     │                 │  3. POST /auth/exchange                 │
     │                 │     {code, code_verifier, provider}   │
     │                 │──────────────────▶│                   │
     │                 │                   │  4. Exchange with │
     │                 │                   │     OAuth provider│
     │                 │                   │──────────────────▶│
     │                 │                   │◀──────────────────│
     │                 │                   │   ID Token +      │
     │                 │                   │   Access Token    │
     │                 │                   │                   │
     │                 │                   │  5. Validate domain│
     │                 │                   │     (must be      │
     │                 │                   │     @school.be)   │
     │                 │                   │                   │
     │                 │                   │  6. Create/Update │
     │                 │                   │     user record   │
     │                 │                   │                   │
     │                 │◀──────────────────│  7. Return JWT    │
     │                 │   {access_token, refresh_token, user} │
     │                 │                   │                   │
     │  8. Store tokens│                   │                   │
     │     in Windows  │                   │                   │
     │     Credential  │                   │                   │
     │     Manager     │                   │                   │
     │◀────────────────│                   │                   │
     │                 │                   │                   │
```

#### Real-time Collaboration Flow (WebSocket)

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│  Student A  │     │  Student B  │     │  WebSocket  │     │  PostgreSQL │
│  (CEO Role) │     │  (CFO Role) │     │   Server    │     │    DB       │
└──────┬──────┘     └──────┬──────┘     └──────┬──────┘     └──────┬──────┘
       │                   │                   │                   │
       │  1. WSS Connect   │                   │                   │
       │  (with JWT)       │                   │                   │
       │──────────────────▶│                   │                   │
       │                   │  2. WSS Connect   │                   │
       │                   │  (same team room) │                   │
       │                   │──────────────────▶│                   │
       │                   │                   │                   │
       │  3. Edit budget   │                   │                   │
       │  field            │                   │                   │
       │──────────────────▶│                   │                   │
       │                   │  4. Lock field    │                   │
       │                   │  (optimistic)     │                   │
       │                   │──────────────────▶│                   │
       │                   │                   │  5. Broadcast     │
       │                   │                   │     "field_locked"│
       │  6. Show lock     │◀──────────────────│                   │
       │     indicator     │                   │                   │
       │◀──────────────────│                   │                   │
       │                   │                   │                   │
       │  7. Save changes  │                   │                   │
       │──────────────────▶│                   │                   │
       │                   │  8. Update DB     │                   │
       │                   │──────────────────────────────────────▶│
       │                   │                   │                   │
       │                   │                   │  9. Broadcast     │
       │                   │                   │     "field_updated│
       │  10. Update UI    │◀──────────────────│                   │
       │◀──────────────────│                   │                   │
       │                   │                   │                   │
```

## 1.3 Layered Architecture (Client)

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         PRESENTATION LAYER                                   │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  QML UI Components (Declarative, Reactive)                          │   │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐   │   │
│  │  │   Views     │ │  Dialogs    │ │   Forms     │ │  Charts     │   │   │
│  │  │(Pages/Screens)│ │(Modals)     │ │(Input)      │ │(Data Viz)   │   │   │
│  │  └─────────────┘ └─────────────┘ └─────────────┘ └─────────────┘   │   │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐   │   │
│  │  │  Animations │ │  Themes     │ │  Icons      │ │  Tooltips   │   │   │
│  │  │(Spring/Easing)│ │(Light/Dark/HC)│ │(SVG paths)  │ │(Help)       │   │   │
│  │  └─────────────┘ └─────────────┘ └─────────────┘ └─────────────┘   │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
├─────────────────────────────────────────────────────────────────────────────┤
│                         APPLICATION LAYER                                    │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  C++ Business Logic Controllers                                     │   │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐   │   │
│  │  │   Auth      │ │ Simulation  │ │   Team      │ │  Messaging  │   │   │
│  │  │ Controller  │ │ Controller  │ │ Controller  │ │ Controller  │   │   │
│  │  └─────────────┘ └─────────────┘ └─────────────┘ └─────────────┘   │   │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐   │   │
│  │  │   User      │ │   Admin     │ │   Teacher   │ │  Settings   │   │   │
│  │  │ Controller  │ │ Controller  │ │ Controller  │ │ Controller  │   │   │
│  │  └─────────────┘ └─────────────┘ └─────────────┘ └─────────────┘   │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
├─────────────────────────────────────────────────────────────────────────────┤
│                          DOMAIN LAYER                                        │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Core Business Entities & Services                                  │   │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐   │   │
│  │  │   User      │ │  Company    │ │ Simulation  │ │   Role      │   │   │
│  │  │   Entity    │ │   Entity    │ │   Engine    │ │   Service   │   │   │
│  │  └─────────────┘ └─────────────┘ └─────────────┘ └─────────────┘   │   │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐   │   │
│  │  │  Finance    │ │    HR       │ │   Sales     │ │  Inventory  │   │   │
│  │  │   Service   │ │  Service    │ │   Service   │ │   Service   │   │   │
│  │  └─────────────┘ └─────────────┘ └─────────────┘ └─────────────┘   │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
├─────────────────────────────────────────────────────────────────────────────┤
│                         INFRASTRUCTURE LAYER                                 │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Technical Services & External Communication                        │   │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐   │   │
│  │  │   HTTP      │ │  WebSocket  │ │   SQLite    │ │   File      │   │   │
│  │  │   Client    │ │   Client    │ │   Cache     │ │   I/O       │   │   │
│  │  └─────────────┘ └─────────────┘ └─────────────┘ └─────────────┘   │   │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐   │   │
│  │  │   Auth      │ │   I18n      │ │   Logger    │ │   Config    │   │   │
│  │  │   Service   │ │   Service   │ │   Service   │ │   Service   │   │   │
│  │  └─────────────┘ └─────────────┘ └─────────────┘ └─────────────┘   │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 1.4 Module Dependencies

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      MODULE DEPENDENCY GRAPH                                 │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│                           ┌─────────────┐                                   │
│                           │   Core      │                                   │
│                           │ (Base types, │                                   │
│                           │  utilities)  │                                   │
│                           └──────┬──────┘                                   │
│                                  │                                          │
│           ┌──────────────────────┼──────────────────────┐                  │
│           │                      │                      │                  │
│           ▼                      ▼                      ▼                  │
│    ┌─────────────┐        ┌─────────────┐        ┌─────────────┐           │
│    │   Network   │        │    Data     │        │     UI      │           │
│    │   Layer     │        │   Layer     │        │  Foundation │           │
│    │(HTTP/WS)    │        │(Models/Cache)│        │(Components) │           │
│    └──────┬──────┘        └──────┬──────┘        └──────┬──────┘           │
│           │                      │                      │                  │
│           └──────────────────────┼──────────────────────┘                  │
│                                  │                                          │
│                                  ▼                                          │
│                           ┌─────────────┐                                   │
│                           │   Feature   │                                   │
│                           │   Modules   │                                   │
│                           └──────┬──────┘                                   │
│                                  │                                          │
│      ┌───────────┬───────────┬───┴───┬───────────┬───────────┐             │
│      ▼           ▼           ▼       ▼           ▼           ▼             │
│  ┌───────┐  ┌───────┐  ┌───────┐ ┌───────┐ ┌───────┐  ┌───────┐          │
│  │ Auth  │  │  ERP  │  │ Team  │ │ Chat  │ │ Admin │  │ Profile│          │
│  │Module │  │Modules│  │Module │ │Module │ │Module │  │Module  │          │
│  └───────┘  └───────┘  └───────┘ └───────┘ └───────┘  └───────┘          │
│                                                                             │
│  ERP Modules Internal Dependencies:                                        │
│  ┌─────────────────────────────────────────────────────────────┐          │
│  │  Finance ◄──► Inventory ◄──► Sales ◄──► Marketing           │          │
│  │     ▲           ▲            ▲           ▲                  │          │
│  │     └───────────┴────────────┴───────────┘                  │          │
│  │                    Dashboard (KPI Aggregator)               │          │
│  └─────────────────────────────────────────────────────────────┘          │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 1.5 Multi-Tenancy Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                       MULTI-TENANCY DATA MODEL                               │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                    PostgreSQL Database (Single Instance)             │   │
│  │                                                                     │   │
│  │  ┌─────────────────────────────────────────────────────────────┐   │   │
│  │  │                    SHARED TABLES (Global)                    │   │   │
│  │  │  - app_settings, system_logs, oauth_providers                │   │   │
│  │  │  - update_notifications, feature_flags                       │   │   │
│  │  └─────────────────────────────────────────────────────────────┘   │   │
│  │                                                                     │   │
│  │  ┌─────────────────────────────────────────────────────────────┐   │   │
│  │  │  TENANT 1: School A (school_id = 1)                         │   │   │
│  │  │  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐           │   │   │
│  │  │  │  users  │ │ classes │ │  teams  │ │companies│           │   │   │
│  │  │  │(isolated)│ │(isolated)│ │(isolated)│ │(isolated)│          │   │   │
│  │  │  └─────────┘ └─────────┘ └─────────┘ └─────────┘           │   │   │
│  │  │  All queries filtered by: WHERE school_id = 1               │   │   │
│  │  └─────────────────────────────────────────────────────────────┘   │   │
│  │                                                                     │   │
│  │  ┌─────────────────────────────────────────────────────────────┐   │   │
│  │  │  TENANT 2: School B (school_id = 2)                         │   │   │
│  │  │  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐           │   │   │
│  │  │  │  users  │ │ classes │ │  teams  │ │companies│           │   │   │
│  │  │  │(isolated)│ │(isolated)│ │(isolated)│ │(isolated)│          │   │   │
│  │  │  └─────────┘ └─────────┘ └─────────┘ └─────────┘           │   │   │
│  │  │  All queries filtered by: WHERE school_id = 2               │   │   │
│  │  └─────────────────────────────────────────────────────────────┘   │   │
│  │                                                                     │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Row-Level Security (RLS) Policy Example:                                  │
│  ```sql                                                                     │
│  CREATE POLICY tenant_isolation ON users                                   │
│  FOR ALL                                                                    │
│  USING (school_id = current_setting('app.current_school_id')::INTEGER);    │
│  ```                                                                        │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 1.6 Offline-First Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      OFFLINE SYNCHRONIZATION STRATEGY                        │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                     SYNCHRONIZATION STATES                           │   │
│  │                                                                     │   │
│  │   ┌─────────────┐    ┌─────────────┐    ┌─────────────┐            │   │
│  │   │   ONLINE    │───▶│   SYNCING   │───▶│   OFFLINE   │            │   │
│  │   │  (Normal)   │◀───│  (Active)   │◀───│  (Cached)   │            │   │
│  │   └─────────────┘    └─────────────┘    └─────────────┘            │   │
│   │                                                                     │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Local SQLite Cache Schema (Subset of Server Data):                        │
│  ```sql                                                                     │
│  -- User profile (read-only from server)                                   │
│  CREATE TABLE cached_user (                                                │
│      id INTEGER PRIMARY KEY,                                               │
│      display_name TEXT,                                                    │
│      avatar_url TEXT,                                                      │
│      cached_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,                        │
│      expires_at TIMESTAMP  -- 24 hour TTL                                  │
│  );                                                                        │
│                                                                             │
│  -- Company data (read-write, sync queue)                                  │
│  CREATE TABLE cached_company (                                             │
│      id INTEGER PRIMARY KEY,                                               │
│      server_id INTEGER,  -- NULL if not yet synced                         │
│      name TEXT,                                                            │
│      data JSON,                                                            │
│      sync_status TEXT CHECK(sync_status IN ('synced', 'pending', 'error')),│
│      modified_at TIMESTAMP,                                                │
│      sync_attempts INTEGER DEFAULT 0                                       │
│  );                                                                        │
│                                                                             │
│  -- Outgoing sync queue                                                    │
│  CREATE TABLE sync_queue (                                                 │
│      id INTEGER PRIMARY KEY AUTOINCREMENT,                                 │
│      entity_type TEXT,  -- 'company', 'message', etc.                      │
│      entity_id INTEGER,                                                    │
│      operation TEXT,  -- 'create', 'update', 'delete'                      │
│      payload JSON,                                                         │
│      created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,                       │
│      retry_count INTEGER DEFAULT 0                                         │
│  );                                                                        │
│  ```                                                                        │
│                                                                             │
│  Sync Strategy:                                                            │
│  1. Read operations: Check local cache first, fetch from server if stale   │
│  2. Write operations: Queue locally, attempt immediate sync, retry on fail │
│  3. Conflict resolution: Last-write-wins with server timestamp             │
│  4. Background sync: Periodic sync when online (every 60s)                 │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

# PART 2: DATABASE SCHEMA

## 2.1 Schema Overview

The PostgreSQL database uses a **single-database, multi-tenant** architecture with row-level security policies for tenant isolation. All tables include `school_id` for tenant filtering.

## 2.2 Complete Table Definitions

### 2.2.1 Core Identity Tables

```sql
-- =====================================================
-- TABLE: schools
-- PURPOSE: Multi-tenant isolation - each school is a tenant
-- =====================================================
CREATE TABLE schools (
    id                      SERIAL PRIMARY KEY,
    name                    VARCHAR(255) NOT NULL,
    subdomain               VARCHAR(63) UNIQUE,  -- For future multi-tenant URL
    oauth_domains           TEXT[] NOT NULL,     -- Allowed email domains
    default_language        VARCHAR(5) DEFAULT 'nl-BE',
    allowed_languages       VARCHAR(5)[] DEFAULT ARRAY['nl-BE', 'en-GB', 'fr-BE'],
    theme_restrictions      JSONB DEFAULT '{}',  -- {locked: bool, allowed_themes: []}
    streak_enabled          BOOLEAN DEFAULT TRUE,
    friend_system_enabled   BOOLEAN DEFAULT TRUE,
    cross_class_messaging   BOOLEAN DEFAULT FALSE,
    energy_saving_default   BOOLEAN DEFAULT FALSE,
    animation_default       VARCHAR(20) DEFAULT 'full', -- full, reduced, none
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    is_active               BOOLEAN DEFAULT TRUE,
    deleted_at              TIMESTAMP WITH TIME ZONE  -- Soft delete for GDPR
);

CREATE INDEX idx_schools_oauth_domains ON schools USING GIN(oauth_domains);
CREATE INDEX idx_schools_active ON schools(is_active) WHERE is_active = TRUE;

-- =====================================================
-- TABLE: users
-- PURPOSE: All user accounts (students, teachers, admins)
-- =====================================================
CREATE TABLE users (
    id                      SERIAL PRIMARY KEY,
    school_id               INTEGER NOT NULL REFERENCES schools(id) ON DELETE CASCADE,
    email                   VARCHAR(255) NOT NULL,
    oauth_provider          VARCHAR(20) NOT NULL,  -- 'google', 'microsoft'
    oauth_subject           VARCHAR(255) NOT NULL, -- Provider's user ID
    role                    VARCHAR(20) NOT NULL,  -- 'super_admin', 'school_admin', 'teacher', 'student'
    
    -- Profile fields
    display_name            VARCHAR(100),
    username                VARCHAR(50),  -- Unique handle @username
    avatar_url              VARCHAR(500),
    banner_url              VARCHAR(500),
    bio                     VARCHAR(500),
    
    -- Settings
    preferred_language      VARCHAR(5) DEFAULT 'nl-BE',
    theme_preference        VARCHAR(50) DEFAULT 'system',
    font_size               VARCHAR(10) DEFAULT 'medium', -- small, medium, large
    animation_preference    VARCHAR(20) DEFAULT 'full',
    layout_density          VARCHAR(10) DEFAULT 'comfortable',
    energy_saving_mode      BOOLEAN DEFAULT FALSE,
    
    -- Privacy settings
    profile_visibility      VARCHAR(20) DEFAULT 'friends', -- everyone, friends, class, teacher
    friend_requests_allowed VARCHAR(20) DEFAULT 'class',   -- everyone, class, disabled
    
    -- Status
    is_active               BOOLEAN DEFAULT TRUE,
    last_login_at           TIMESTAMP WITH TIME ZONE,
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    deleted_at              TIMESTAMP WITH TIME ZONE,
    
    -- Constraints
    CONSTRAINT unique_email_per_school UNIQUE(school_id, email),
    CONSTRAINT unique_username_per_school UNIQUE(school_id, username),
    CONSTRAINT unique_oauth UNIQUE(oauth_provider, oauth_subject)
);

CREATE INDEX idx_users_school ON users(school_id);
CREATE INDEX idx_users_role ON users(role);
CREATE INDEX idx_users_school_role ON users(school_id, role);
CREATE INDEX idx_users_active ON users(school_id, is_active) WHERE is_active = TRUE;

-- =====================================================
-- TABLE: user_sessions
-- PURPOSE: JWT refresh token tracking for security
-- =====================================================
CREATE TABLE user_sessions (
    id                      SERIAL PRIMARY KEY,
    user_id                 INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    refresh_token_hash      VARCHAR(64) NOT NULL,  -- SHA-256 of token
    device_info             VARCHAR(255),          -- OS, browser, app version
    ip_address              INET,
    expires_at              TIMESTAMP WITH TIME ZONE NOT NULL,
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    last_used_at            TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    revoked_at              TIMESTAMP WITH TIME ZONE
);

CREATE INDEX idx_sessions_user ON user_sessions(user_id);
CREATE INDEX idx_sessions_token ON user_sessions(refresh_token_hash);
CREATE INDEX idx_sessions_expires ON user_sessions(expires_at) WHERE revoked_at IS NULL;
```

### 2.2.2 Class and Team Management Tables

```sql
-- =====================================================
-- TABLE: classes
-- PURPOSE: School classes/groups
-- =====================================================
CREATE TABLE classes (
    id                      SERIAL PRIMARY KEY,
    school_id               INTEGER NOT NULL REFERENCES schools(id) ON DELETE CASCADE,
    name                    VARCHAR(100) NOT NULL,  -- e.g., "5de jaar Economie A"
    description             TEXT,
    academic_year           VARCHAR(9) NOT NULL,    -- e.g., "2025-2026"
    teacher_id              INTEGER NOT NULL REFERENCES users(id),
    max_team_size           INTEGER DEFAULT 4,
    allowed_modules         VARCHAR(50)[] DEFAULT ARRAY['finance', 'sales', 'inventory', 'hr', 'marketing', 'logistics'],
    simulation_time_scale   VARCHAR(20) DEFAULT 'realtime', -- realtime, accelerated, turn_based
    is_active               BOOLEAN DEFAULT TRUE,
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

CREATE INDEX idx_classes_school ON classes(school_id);
CREATE INDEX idx_classes_teacher ON classes(teacher_id);
CREATE INDEX idx_classes_active ON classes(school_id, is_active) WHERE is_active = TRUE;

-- =====================================================
-- TABLE: class_memberships
-- PURPOSE: Student-to-class many-to-many relationship
-- =====================================================
CREATE TABLE class_memberships (
    id                      SERIAL PRIMARY KEY,
    class_id                INTEGER NOT NULL REFERENCES classes(id) ON DELETE CASCADE,
    student_id              INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    joined_at               TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    left_at                 TIMESTAMP WITH TIME ZONE,
    
    CONSTRAINT unique_student_class UNIQUE(class_id, student_id, left_at)
);

CREATE INDEX idx_memberships_class ON class_memberships(class_id);
CREATE INDEX idx_memberships_student ON class_memberships(student_id);
CREATE INDEX idx_memberships_active ON class_memberships(student_id) WHERE left_at IS NULL;

-- =====================================================
-- TABLE: teams
-- PURPOSE: Student teams within a class
-- =====================================================
CREATE TABLE teams (
    id                      SERIAL PRIMARY KEY,
    class_id                INTEGER NOT NULL REFERENCES classes(id) ON DELETE CASCADE,
    name                    VARCHAR(100) NOT NULL,
    company_name            VARCHAR(100),  -- Their simulated company
    current_simulation_id   INTEGER,       -- Reference to active simulation
    is_active               BOOLEAN DEFAULT TRUE,
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    created_by              INTEGER NOT NULL REFERENCES users(id)
);

CREATE INDEX idx_teams_class ON teams(class_id);
CREATE INDEX idx_teams_simulation ON teams(current_simulation_id);

-- =====================================================
-- TABLE: team_memberships
-- PURPOSE: Student-to-team assignments with roles
-- =====================================================
CREATE TABLE team_memberships (
    id                      SERIAL PRIMARY KEY,
    team_id                 INTEGER NOT NULL REFERENCES teams(id) ON DELETE CASCADE,
    student_id              INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    role                    VARCHAR(30) NOT NULL,  -- 'ceo', 'cfo', 'sales_manager', etc.
    joined_at               TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    left_at                 TIMESTAMP WITH TIME ZONE,
    
    CONSTRAINT unique_student_team UNIQUE(team_id, student_id, left_at)
);

CREATE INDEX idx_team_members_team ON team_memberships(team_id);
CREATE INDEX idx_team_members_student ON team_memberships(student_id);
CREATE INDEX idx_team_members_role ON team_memberships(team_id, role) WHERE left_at IS NULL;
```

### 2.2.3 Simulation Core Tables

```sql
-- =====================================================
-- TABLE: simulation_companies
-- PURPOSE: Student/team companies in simulations
-- =====================================================
CREATE TABLE simulation_companies (
    id                      SERIAL PRIMARY KEY,
    school_id               INTEGER NOT NULL REFERENCES schools(id) ON DELETE CASCADE,
    team_id                 INTEGER REFERENCES teams(id) ON DELETE SET NULL,
    creator_id              INTEGER NOT NULL REFERENCES users(id),
    
    -- Company details
    name                    VARCHAR(100) NOT NULL,
    logo_url                VARCHAR(500),
    industry_template       VARCHAR(50) NOT NULL,  -- 'retail', 'tech', 'logistics', etc.
    initial_budget          DECIMAL(15, 2) NOT NULL DEFAULT 100000.00,
    currency_code           VARCHAR(3) DEFAULT 'EUR',
    
    -- Simulation settings
    time_scale              VARCHAR(20) DEFAULT 'realtime',
    simulation_speed        INTEGER DEFAULT 1,  -- 1x, 2x, 5x, 10x for accelerated
    current_simulated_date  DATE NOT NULL DEFAULT CURRENT_DATE,
    simulation_start_date   DATE NOT NULL DEFAULT CURRENT_DATE,
    
    -- Status
    status                  VARCHAR(20) DEFAULT 'active', -- active, paused, completed, archived
    is_ai_enabled           BOOLEAN DEFAULT FALSE,
    
    -- Metadata
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    archived_at             TIMESTAMP WITH TIME ZONE
);

CREATE INDEX idx_companies_school ON simulation_companies(school_id);
CREATE INDEX idx_companies_team ON simulation_companies(team_id);
CREATE INDEX idx_companies_creator ON simulation_companies(creator_id);
CREATE INDEX idx_companies_status ON simulation_companies(status);

-- =====================================================
-- TABLE: simulation_modules
-- PURPOSE: Enabled modules per company
-- =====================================================
CREATE TABLE simulation_modules (
    id                      SERIAL PRIMARY KEY,
    company_id              INTEGER NOT NULL REFERENCES simulation_companies(id) ON DELETE CASCADE,
    module_type             VARCHAR(30) NOT NULL,  -- 'finance', 'sales', 'inventory', etc.
    is_enabled              BOOLEAN DEFAULT TRUE,
    config                  JSONB DEFAULT '{}',    -- Module-specific settings
    unlocked_at             TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    
    CONSTRAINT unique_company_module UNIQUE(company_id, module_type)
);

CREATE INDEX idx_modules_company ON simulation_modules(company_id);

-- =====================================================
-- TABLE: kpi_snapshots
-- PURPOSE: Time-series KPI data for charts and analysis
-- =====================================================
CREATE TABLE kpi_snapshots (
    id                      BIGSERIAL PRIMARY KEY,
    company_id              INTEGER NOT NULL REFERENCES simulation_companies(id) ON DELETE CASCADE,
    snapshot_date           DATE NOT NULL,
    
    -- Financial KPIs
    revenue                 DECIMAL(15, 2) DEFAULT 0,
    expenses                DECIMAL(15, 2) DEFAULT 0,
    net_profit              DECIMAL(15, 2) DEFAULT 0,
    profit_margin           DECIMAL(5, 4) DEFAULT 0,  -- 0.15 = 15%
    cash_on_hand            DECIMAL(15, 2) DEFAULT 0,
    total_assets            DECIMAL(15, 2) DEFAULT 0,
    total_liabilities       DECIMAL(15, 2) DEFAULT 0,
    equity                  DECIMAL(15, 2) DEFAULT 0,
    
    -- Operational KPIs
    inventory_value         DECIMAL(15, 2) DEFAULT 0,
    customer_satisfaction   DECIMAL(3, 2) DEFAULT 0,  -- 0.0 to 1.0
    employee_satisfaction   DECIMAL(3, 2) DEFAULT 0,
    market_share            DECIMAL(5, 4) DEFAULT 0,
    brand_awareness         DECIMAL(5, 4) DEFAULT 0,
    
    -- Sales KPIs
    total_orders            INTEGER DEFAULT 0,
    total_customers         INTEGER DEFAULT 0,
    average_order_value     DECIMAL(10, 2) DEFAULT 0,
    
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

CREATE INDEX idx_kpi_company ON kpi_snapshots(company_id);
CREATE INDEX idx_kpi_date ON kpi_snapshots(company_id, snapshot_date);
CREATE INDEX idx_kpi_recent ON kpi_snapshots(company_id, snapshot_date DESC);

-- Partition by month for performance (optional, for large datasets)
-- CREATE TABLE kpi_snapshots_y2025m01 PARTITION OF kpi_snapshots
--     FOR VALUES FROM ('2025-01-01') TO ('2025-02-01');
```

### 2.2.4 Finance Module Tables

```sql
-- =====================================================
-- TABLE: financial_accounts
-- PURPOSE: Chart of accounts for double-entry bookkeeping
-- =====================================================
CREATE TABLE financial_accounts (
    id                      SERIAL PRIMARY KEY,
    company_id              INTEGER NOT NULL REFERENCES simulation_companies(id) ON DELETE CASCADE,
    account_code            VARCHAR(20) NOT NULL,  -- e.g., "1000", "4000"
    account_name            VARCHAR(100) NOT NULL,
    account_type            VARCHAR(20) NOT NULL,  -- 'asset', 'liability', 'equity', 'revenue', 'expense'
    parent_account_id       INTEGER REFERENCES financial_accounts(id),
    is_active               BOOLEAN DEFAULT TRUE,
    
    CONSTRAINT unique_account_code UNIQUE(company_id, account_code)
);

CREATE INDEX idx_accounts_company ON financial_accounts(company_id);
CREATE INDEX idx_accounts_type ON financial_accounts(company_id, account_type);

-- =====================================================
-- TABLE: financial_ledger_entries
-- PURPOSE: Double-entry bookkeeping journal entries
-- =====================================================
CREATE TABLE financial_ledger_entries (
    id                      BIGSERIAL PRIMARY KEY,
    company_id              INTEGER NOT NULL REFERENCES simulation_companies(id) ON DELETE CASCADE,
    entry_date              DATE NOT NULL,
    
    -- Transaction details
    debit_account_id        INTEGER NOT NULL REFERENCES financial_accounts(id),
    credit_account_id       INTEGER NOT NULL REFERENCES financial_accounts(id),
    amount                  DECIMAL(15, 2) NOT NULL,
    currency_code           VARCHAR(3) DEFAULT 'EUR',
    
    -- Reference info
    description             TEXT,
    reference_type          VARCHAR(30),  -- 'invoice', 'payment', 'payroll', 'adjustment'
    reference_id            INTEGER,      -- Link to source document
    
    -- Metadata
    created_by              INTEGER REFERENCES users(id),
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    simulation_event_id     INTEGER       -- Link to simulation event if applicable
);

CREATE INDEX idx_ledger_company ON financial_ledger_entries(company_id);
CREATE INDEX idx_ledger_date ON financial_ledger_entries(company_id, entry_date);
CREATE INDEX idx_ledger_accounts ON financial_ledger_entries(debit_account_id, credit_account_id);

-- =====================================================
-- TABLE: financial_reports_cache
-- PURPOSE: Cached financial statements for performance
-- =====================================================
CREATE TABLE financial_reports_cache (
    id                      SERIAL PRIMARY KEY,
    company_id              INTEGER NOT NULL REFERENCES simulation_companies(id) ON DELETE CASCADE,
    report_type             VARCHAR(30) NOT NULL,  -- 'income_statement', 'balance_sheet', 'cash_flow'
    period_start            DATE NOT NULL,
    period_end              DATE NOT NULL,
    report_data             JSONB NOT NULL,
    generated_at            TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    
    CONSTRAINT unique_report_period UNIQUE(company_id, report_type, period_start, period_end)
);

CREATE INDEX idx_reports_company ON financial_reports_cache(company_id);
```

### 2.2.5 Sales & CRM Module Tables

```sql
-- =====================================================
-- TABLE: simulated_customers
-- PURPOSE: AI-generated customer entities
-- =====================================================
CREATE TABLE simulated_customers (
    id                      SERIAL PRIMARY KEY,
    company_id              INTEGER NOT NULL REFERENCES simulation_companies(id) ON DELETE CASCADE,
    
    -- Customer profile
    name                    VARCHAR(100) NOT NULL,
    customer_type           VARCHAR(20) NOT NULL,  -- 'individual', 'business'
    segment                 VARCHAR(30),           -- 'premium', 'standard', 'budget'
    
    -- Simulation attributes
    loyalty_score           INTEGER DEFAULT 50,    -- 0-100
    price_sensitivity       INTEGER DEFAULT 50,    -- 0-100 (higher = more sensitive)
    quality_expectation     INTEGER DEFAULT 50,    -- 0-100
    
    -- Status
    is_active               BOOLEAN DEFAULT TRUE,
    acquired_at             TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    churned_at              TIMESTAMP WITH TIME ZONE,
    
    -- Relationships
    total_orders            INTEGER DEFAULT 0,
    lifetime_value          DECIMAL(15, 2) DEFAULT 0
);

CREATE INDEX idx_customers_company ON simulated_customers(company_id);
CREATE INDEX idx_customers_active ON simulated_customers(company_id, is_active) WHERE is_active = TRUE;

-- =====================================================
-- TABLE: sales_pipeline
-- PURPOSE: CRM pipeline stages
-- =====================================================
CREATE TABLE sales_pipeline (
    id                      SERIAL PRIMARY KEY,
    company_id              INTEGER NOT NULL REFERENCES simulation_companies(id) ON DELETE CASCADE,
    customer_id             INTEGER NOT NULL REFERENCES simulated_customers(id),
    
    -- Pipeline stage
    stage                   VARCHAR(30) NOT NULL,  -- 'lead', 'prospect', 'proposal', 'negotiation', 'closed_won', 'closed_lost'
    stage_entered_at        TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    expected_value          DECIMAL(12, 2),
    probability             DECIMAL(3, 2) DEFAULT 0.20,  -- Close probability
    
    -- Assignment
    assigned_to             INTEGER REFERENCES users(id),  -- Team member responsible
    
    -- Notes
    notes                   TEXT,
    
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    closed_at               TIMESTAMP WITH TIME ZONE
);

CREATE INDEX idx_pipeline_company ON sales_pipeline(company_id);
CREATE INDEX idx_pipeline_stage ON sales_pipeline(company_id, stage);
CREATE INDEX idx_pipeline_assigned ON sales_pipeline(assigned_to);

-- =====================================================
-- TABLE: sales_orders
-- PURPOSE: Customer orders
-- =====================================================
CREATE TABLE sales_orders (
    id                      SERIAL PRIMARY KEY,
    company_id              INTEGER NOT NULL REFERENCES simulation_companies(id) ON DELETE CASCADE,
    customer_id             INTEGER NOT NULL REFERENCES simulated_customers(id),
    pipeline_id             INTEGER REFERENCES sales_pipeline(id),
    
    -- Order details
    order_number            VARCHAR(20) NOT NULL,
    order_date              DATE NOT NULL,
    delivery_date           DATE,
    status                  VARCHAR(20) DEFAULT 'pending', -- pending, confirmed, shipped, delivered, cancelled
    
    -- Financial
    subtotal                DECIMAL(12, 2) NOT NULL,
    tax_amount              DECIMAL(12, 2) DEFAULT 0,
    discount_amount         DECIMAL(12, 2) DEFAULT 0,
    total_amount            DECIMAL(12, 2) NOT NULL,
    currency_code           VARCHAR(3) DEFAULT 'EUR',
    
    -- Payment
    payment_status          VARCHAR(20) DEFAULT 'unpaid', -- unpaid, partial, paid
    paid_amount             DECIMAL(12, 2) DEFAULT 0,
    
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    
    CONSTRAINT unique_order_number UNIQUE(company_id, order_number)
);

CREATE INDEX idx_orders_company ON sales_orders(company_id);
CREATE INDEX idx_orders_customer ON sales_orders(customer_id);
CREATE INDEX idx_orders_status ON sales_orders(company_id, status);

-- =====================================================
-- TABLE: sales_order_items
-- PURPOSE: Line items for orders
-- =====================================================
CREATE TABLE sales_order_items (
    id                      SERIAL PRIMARY KEY,
    order_id                INTEGER NOT NULL REFERENCES sales_orders(id) ON DELETE CASCADE,
    product_id              INTEGER NOT NULL,  -- Reference to inventory_products
    quantity                INTEGER NOT NULL,
    unit_price              DECIMAL(10, 2) NOT NULL,
    discount_percent        DECIMAL(5, 2) DEFAULT 0,
    total_price             DECIMAL(12, 2) NOT NULL
);

CREATE INDEX idx_order_items_order ON sales_order_items(order_id);
```

### 2.2.6 Inventory Module Tables

```sql
-- =====================================================
-- TABLE: inventory_products
-- PURPOSE: Product catalog
-- =====================================================
CREATE TABLE inventory_products (
    id                      SERIAL PRIMARY KEY,
    company_id              INTEGER NOT NULL REFERENCES simulation_companies(id) ON DELETE CASCADE,
    
    -- Product details
    sku                     VARCHAR(50) NOT NULL,
    name                    VARCHAR(200) NOT NULL,
    description             TEXT,
    category                VARCHAR(50),
    
    -- Pricing
    cost_price              DECIMAL(10, 2) NOT NULL,
    selling_price           DECIMAL(10, 2) NOT NULL,
    
    -- Inventory
    current_stock           INTEGER DEFAULT 0,
    min_stock_level         INTEGER DEFAULT 10,
    max_stock_level         INTEGER DEFAULT 1000,
    reorder_point           INTEGER DEFAULT 25,
    
    -- Unit
    unit_of_measure         VARCHAR(20) DEFAULT 'stuks',  -- pieces, kg, liters, etc.
    
    -- Status
    is_active               BOOLEAN DEFAULT TRUE,
    
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    
    CONSTRAINT unique_sku UNIQUE(company_id, sku)
);

CREATE INDEX idx_products_company ON inventory_products(company_id);
CREATE INDEX idx_products_category ON inventory_products(company_id, category);
CREATE INDEX idx_products_low_stock ON inventory_products(company_id, current_stock, reorder_point) 
    WHERE current_stock <= reorder_point;

-- =====================================================
-- TABLE: inventory_suppliers
-- PURPOSE: Product suppliers
-- =====================================================
CREATE TABLE inventory_suppliers (
    id                      SERIAL PRIMARY KEY,
    company_id              INTEGER NOT NULL REFERENCES simulation_companies(id) ON DELETE CASCADE,
    name                    VARCHAR(100) NOT NULL,
    contact_email           VARCHAR(255),
    contact_phone           VARCHAR(20),
    lead_time_days          INTEGER DEFAULT 7,  -- Average delivery time
    reliability_score       DECIMAL(3, 2) DEFAULT 0.95,  -- 0-1 delivery success rate
    is_active               BOOLEAN DEFAULT TRUE
);

CREATE INDEX idx_suppliers_company ON inventory_suppliers(company_id);

-- =====================================================
-- TABLE: inventory_purchase_orders
-- PURPOSE: Purchase orders to suppliers
-- =====================================================
CREATE TABLE inventory_purchase_orders (
    id                      SERIAL PRIMARY KEY,
    company_id              INTEGER NOT NULL REFERENCES simulation_companies(id) ON DELETE CASCADE,
    supplier_id             INTEGER NOT NULL REFERENCES inventory_suppliers(id),
    
    po_number               VARCHAR(20) NOT NULL,
    order_date              DATE NOT NULL,
    expected_delivery       DATE,
    actual_delivery         DATE,
    
    status                  VARCHAR(20) DEFAULT 'draft', -- draft, sent, confirmed, received, cancelled
    
    total_amount            DECIMAL(12, 2) NOT NULL,
    currency_code           VARCHAR(3) DEFAULT 'EUR',
    
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    
    CONSTRAINT unique_po_number UNIQUE(company_id, po_number)
);

CREATE INDEX idx_po_company ON inventory_purchase_orders(company_id);
CREATE INDEX idx_po_status ON inventory_purchase_orders(company_id, status);

-- =====================================================
-- TABLE: inventory_movements
-- PURPOSE: Stock movements (in/out/adjustment)
-- =====================================================
CREATE TABLE inventory_movements (
    id                      BIGSERIAL PRIMARY KEY,
    company_id              INTEGER NOT NULL REFERENCES simulation_companies(id) ON DELETE CASCADE,
    product_id              INTEGER NOT NULL REFERENCES inventory_products(id),
    
    movement_type           VARCHAR(20) NOT NULL,  -- 'in', 'out', 'adjustment', 'transfer'
    quantity                INTEGER NOT NULL,      -- Positive for in, negative for out
    
    -- Reference
    reference_type          VARCHAR(30),  -- 'purchase_order', 'sales_order', 'adjustment', 'production'
    reference_id            INTEGER,
    
    -- Stock after movement
    stock_after             INTEGER NOT NULL,
    
    -- Cost (for valuation)
    unit_cost               DECIMAL(10, 2),
    total_cost              DECIMAL(12, 2),
    
    notes                   TEXT,
    created_by              INTEGER REFERENCES users(id),
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

CREATE INDEX idx_movements_company ON inventory_movements(company_id);
CREATE INDEX idx_movements_product ON inventory_movements(product_id);
CREATE INDEX idx_movements_date ON inventory_movements(company_id, created_at);
```

### 2.2.7 HR Module Tables

```sql
-- =====================================================
-- TABLE: simulated_employees
-- PURPOSE: Company employees
-- =====================================================
CREATE TABLE simulated_employees (
    id                      SERIAL PRIMARY KEY,
    company_id              INTEGER NOT NULL REFERENCES simulation_companies(id) ON DELETE CASCADE,
    
    -- Personal info
    first_name              VARCHAR(50) NOT NULL,
    last_name               VARCHAR(50) NOT NULL,
    email                   VARCHAR(100),
    
    -- Employment
    department              VARCHAR(50) NOT NULL,  -- 'sales', 'production', 'admin', etc.
    job_title               VARCHAR(50) NOT NULL,
    employment_type         VARCHAR(20) DEFAULT 'fulltime', -- fulltime, parttime, contract
    
    -- Compensation
    base_salary             DECIMAL(10, 2) NOT NULL,  -- Monthly gross
    bonus_potential         DECIMAL(5, 4) DEFAULT 0.10,  -- Max 10% bonus
    
    -- Skills & Performance
    skill_level             INTEGER DEFAULT 3,  -- 1-5 scale
    productivity_score      INTEGER DEFAULT 100,  -- Percentage
    satisfaction_score      INTEGER DEFAULT 75,  -- 0-100
    
    -- Status
    status                  VARCHAR(20) DEFAULT 'active', -- active, on_leave, terminated
    hired_at                DATE NOT NULL DEFAULT CURRENT_DATE,
    terminated_at           DATE,
    
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

CREATE INDEX idx_employees_company ON simulated_employees(company_id);
CREATE INDEX idx_employees_dept ON simulated_employees(company_id, department);
CREATE INDEX idx_employees_status ON simulated_employees(company_id, status);

-- =====================================================
-- TABLE: hr_payroll_records
-- PURPOSE: Monthly payroll calculations
-- =====================================================
CREATE TABLE hr_payroll_records (
    id                      SERIAL PRIMARY KEY,
    company_id              INTEGER NOT NULL REFERENCES simulation_companies(id) ON DELETE CASCADE,
    employee_id             INTEGER NOT NULL REFERENCES simulated_employees(id),
    
    pay_period_start        DATE NOT NULL,
    pay_period_end          DATE NOT NULL,
    
    -- Earnings
    base_salary             DECIMAL(10, 2) NOT NULL,
    bonus_amount            DECIMAL(10, 2) DEFAULT 0,
    overtime_amount         DECIMAL(10, 2) DEFAULT 0,
    total_earnings          DECIMAL(10, 2) NOT NULL,
    
    -- Deductions (Belgian simplified model)
    social_security         DECIMAL(10, 2) NOT NULL,  -- ~13.07% employee share
    withholding_tax         DECIMAL(10, 2) NOT NULL,  -- Prepayment
    other_deductions        DECIMAL(10, 2) DEFAULT 0,
    total_deductions        DECIMAL(10, 2) NOT NULL,
    
    -- Net
    net_pay                 DECIMAL(10, 2) NOT NULL,
    
    -- Employer costs
    employer_social_sec     DECIMAL(10, 2) NOT NULL,  -- ~25% employer share
    total_employer_cost     DECIMAL(10, 2) NOT NULL,
    
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    
    CONSTRAINT unique_payroll_period UNIQUE(employee_id, pay_period_start, pay_period_end)
);

CREATE INDEX idx_payroll_company ON hr_payroll_records(company_id);
CREATE INDEX idx_payroll_period ON hr_payroll_records(company_id, pay_period_start);

-- =====================================================
-- TABLE: hr_training_records
-- PURPOSE: Employee training investments
-- =====================================================
CREATE TABLE hr_training_records (
    id                      SERIAL PRIMARY KEY,
    company_id              INTEGER NOT NULL REFERENCES simulation_companies(id) ON DELETE CASCADE,
    employee_id             INTEGER NOT NULL REFERENCES simulated_employees(id),
    
    training_type           VARCHAR(50) NOT NULL,
    description             TEXT,
    cost                    DECIMAL(10, 2) NOT NULL,
    duration_hours          INTEGER,
    
    -- Impact
    productivity_gain       INTEGER DEFAULT 0,  -- Percentage points gained
    satisfaction_gain       INTEGER DEFAULT 0,
    
    completed_at            DATE,
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

CREATE INDEX idx_training_company ON hr_training_records(company_id);
CREATE INDEX idx_training_employee ON hr_training_records(employee_id);
```

### 2.2.8 Marketing Module Tables

```sql
-- =====================================================
-- TABLE: marketing_campaigns
-- PURPOSE: Marketing campaigns and their performance
-- =====================================================
CREATE TABLE marketing_campaigns (
    id                      SERIAL PRIMARY KEY,
    company_id              INTEGER NOT NULL REFERENCES simulation_companies(id) ON DELETE CASCADE,
    
    name                    VARCHAR(100) NOT NULL,
    description             TEXT,
    
    -- Campaign details
    channel                 VARCHAR(30) NOT NULL,  -- 'social_media', 'tv', 'print', 'online_ads', 'email'
    target_audience         VARCHAR(50),
    
    -- Budget & Timing
    budget                  DECIMAL(12, 2) NOT NULL,
    start_date              DATE NOT NULL,
    end_date                DATE,
    
    -- Status
    status                  VARCHAR(20) DEFAULT 'planned', -- planned, active, completed, cancelled
    
    -- Results (populated after completion)
    actual_spend            DECIMAL(12, 2) DEFAULT 0,
    estimated_reach         INTEGER DEFAULT 0,
    actual_reach            INTEGER DEFAULT 0,
    conversions             INTEGER DEFAULT 0,
    revenue_attributed      DECIMAL(12, 2) DEFAULT 0,
    roi                     DECIMAL(5, 4) DEFAULT 0,
    
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    created_by              INTEGER REFERENCES users(id)
);

CREATE INDEX idx_campaigns_company ON marketing_campaigns(company_id);
CREATE INDEX idx_campaigns_status ON marketing_campaigns(company_id, status);
CREATE INDEX idx_campaigns_dates ON marketing_campaigns(start_date, end_date);

-- =====================================================
-- TABLE: marketing_brand_metrics
-- PURPOSE: Brand awareness tracking over time
-- =====================================================
CREATE TABLE marketing_brand_metrics (
    id                      SERIAL PRIMARY KEY,
    company_id              INTEGER NOT NULL REFERENCES simulation_companies(id) ON DELETE CASCADE,
    metric_date             DATE NOT NULL,
    
    brand_awareness_score   INTEGER DEFAULT 0,  -- 0-100
    brand_sentiment_score   INTEGER DEFAULT 50, -- 0-100 (50 = neutral)
    
    social_media_followers  INTEGER DEFAULT 0,
    social_media_engagement DECIMAL(5, 4) DEFAULT 0,  -- Engagement rate
    
    website_visits          INTEGER DEFAULT 0,
    
    CONSTRAINT unique_brand_metric_date UNIQUE(company_id, metric_date)
);

CREATE INDEX idx_brand_metrics_company ON marketing_brand_metrics(company_id);
CREATE INDEX idx_brand_metrics_date ON marketing_brand_metrics(company_id, metric_date);
```

### 2.2.9 Simulation Events & AI Tables

```sql
-- =====================================================
-- TABLE: simulation_events
-- PURPOSE: Random and teacher-triggered events
-- =====================================================
CREATE TABLE simulation_events (
    id                      SERIAL PRIMARY KEY,
    company_id              INTEGER NOT NULL REFERENCES simulation_companies(id) ON DELETE CASCADE,
    
    event_type              VARCHAR(50) NOT NULL,  -- 'market_change', 'supplier_issue', 'competitor_action', etc.
    event_category          VARCHAR(20) NOT NULL,  -- 'economic', 'operational', 'competitive', 'regulatory'
    
    title                   VARCHAR(200) NOT NULL,
    description             TEXT NOT NULL,
    
    -- Impact (JSON for flexibility)
    impact_config           JSONB NOT NULL,  -- {revenue_impact: -0.15, cost_impact: 0.10, duration_days: 30}
    
    -- Timing
    triggered_at            TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    expires_at              TIMESTAMP WITH TIME ZONE,
    resolved_at             TIMESTAMP WITH TIME ZONE,
    
    -- Source
    triggered_by            VARCHAR(20) DEFAULT 'system', -- 'system', 'teacher', 'manual'
    teacher_id              INTEGER REFERENCES users(id),
    
    -- Student response
    student_response        TEXT,
    response_submitted_at   TIMESTAMP WITH TIME ZONE
);

CREATE INDEX idx_events_company ON simulation_events(company_id);
CREATE INDEX idx_events_active ON simulation_events(company_id, resolved_at) WHERE resolved_at IS NULL;

-- =====================================================
-- TABLE: ai_agent_decisions
-- PURPOSE: Log of AI bot decisions for transparency
-- =====================================================
CREATE TABLE ai_agent_decisions (
    id                      BIGSERIAL PRIMARY KEY,
    company_id              INTEGER NOT NULL REFERENCES simulation_companies(id) ON DELETE CASCADE,
    
    agent_role              VARCHAR(30) NOT NULL,  -- 'ceo', 'cfo', etc.
    decision_style          VARCHAR(20) NOT NULL,  -- 'optimal', 'balanced', 'risky', 'poor', 'random'
    
    decision_type           VARCHAR(50) NOT NULL,  -- 'pricing', 'hiring', 'investment', etc.
    decision_context        JSONB NOT NULL,        -- Input parameters
    decision_result         JSONB NOT NULL,        -- Output/decision made
    
    reasoning_summary       TEXT,                  -- Human-readable explanation
    
    simulated_date          DATE NOT NULL,
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

CREATE INDEX idx_ai_decisions_company ON ai_agent_decisions(company_id);
CREATE INDEX idx_ai_decisions_role ON ai_agent_decisions(company_id, agent_role);
CREATE INDEX idx_ai_decisions_date ON ai_agent_decisions(company_id, simulated_date);
```

### 2.2.10 Messaging & Social Tables

```sql
-- =====================================================
-- TABLE: messages
-- PURPOSE: Direct messages between users
-- =====================================================
CREATE TABLE messages (
    id                      BIGSERIAL PRIMARY KEY,
    
    sender_id               INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    recipient_id            INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    
    -- Content
    content                 TEXT NOT NULL,
    content_type            VARCHAR(20) DEFAULT 'text', -- text, image, file
    
    -- Metadata
    is_edited               BOOLEAN DEFAULT FALSE,
    edited_at               TIMESTAMP WITH TIME ZONE,
    
    -- Status
    is_deleted              BOOLEAN DEFAULT FALSE,
    deleted_at              TIMESTAMP WITH TIME ZONE,
    deleted_by              INTEGER REFERENCES users(id),
    
    -- Read tracking
    read_at                 TIMESTAMP WITH TIME ZONE,
    
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    
    -- For conversation queries
    conversation_id         INTEGER  -- Derived: LEAST(sender_id, recipient_id) || GREATEST(...)
);

CREATE INDEX idx_messages_sender ON messages(sender_id);
CREATE INDEX idx_messages_recipient ON messages(recipient_id);
CREATE INDEX idx_messages_conversation ON messages(conversation_id, created_at DESC);
CREATE INDEX idx_messages_unread ON messages(recipient_id, read_at) WHERE read_at IS NULL;

-- =====================================================
-- TABLE: friendships
-- PURPOSE: Friend connections between students
-- =====================================================
CREATE TABLE friendships (
    id                      SERIAL PRIMARY KEY,
    requester_id            INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    addressee_id            INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    
    status                  VARCHAR(20) DEFAULT 'pending', -- pending, accepted, declined, blocked
    
    requested_at            TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    responded_at            TIMESTAMP WITH TIME ZONE,
    
    CONSTRAINT unique_friendship UNIQUE(requester_id, addressee_id)
);

CREATE INDEX idx_friendships_requester ON friendships(requester_id);
CREATE INDEX idx_friendships_addressee ON friendships(addressee_id);
CREATE INDEX idx_friendships_status ON friendships(requester_id, status);

-- =====================================================
-- TABLE: chat_streaks
-- PURPOSE: Track consecutive day messaging streaks
-- =====================================================
CREATE TABLE chat_streaks (
    id                      SERIAL PRIMARY KEY,
    user_a_id               INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    user_b_id               INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    
    current_streak          INTEGER DEFAULT 0,
    longest_streak          INTEGER DEFAULT 0,
    
    last_message_date       DATE,  -- Date of last message exchange
    streak_start_date       DATE,
    
    -- User preferences (can hide streak)
    user_a_hide_streak      BOOLEAN DEFAULT FALSE,
    user_b_hide_streak      BOOLEAN DEFAULT FALSE,
    
    updated_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    
    CONSTRAINT unique_chat_pair UNIQUE(user_a_id, user_b_id),
    CONSTRAINT ordered_users CHECK(user_a_id < user_b_id)
);

CREATE INDEX idx_streaks_user_a ON chat_streaks(user_a_id);
CREATE INDEX idx_streaks_user_b ON chat_streaks(user_b_id);
CREATE INDEX idx_streaks_active ON chat_streaks(user_a_id, current_streak) WHERE current_streak > 0;

-- =====================================================
-- TABLE: team_chat_messages
-- PURPOSE: Team-specific chat (separate from DMs)
-- =====================================================
CREATE TABLE team_chat_messages (
    id                      BIGSERIAL PRIMARY KEY,
    team_id                 INTEGER NOT NULL REFERENCES teams(id) ON DELETE CASCADE,
    sender_id               INTEGER NOT NULL REFERENCES users(id),
    
    content                 TEXT NOT NULL,
    content_type            VARCHAR(20) DEFAULT 'text',
    
    is_edited               BOOLEAN DEFAULT FALSE,
    edited_at               TIMESTAMP WITH TIME ZONE,
    
    is_deleted              BOOLEAN DEFAULT FALSE,
    deleted_by              INTEGER REFERENCES users(id),
    
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

CREATE INDEX idx_team_chat_team ON team_chat_messages(team_id);
CREATE INDEX idx_team_chat_recent ON team_chat_messages(team_id, created_at DESC);
```

### 2.2.11 Notification & Audit Tables

```sql
-- =====================================================
-- TABLE: notifications
-- PURPOSE: In-app notifications for users
-- =====================================================
CREATE TABLE notifications (
    id                      BIGSERIAL PRIMARY KEY,
    user_id                 INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    
    notification_type       VARCHAR(30) NOT NULL,  -- 'message', 'friend_request', 'simulation_event', etc.
    title                   VARCHAR(200) NOT NULL,
    body                    TEXT,
    
    -- Deep link
    action_url              VARCHAR(500),  -- Internal app route
    action_data             JSONB,         -- Additional routing data
    
    -- Priority
    priority                VARCHAR(10) DEFAULT 'normal', -- low, normal, high
    
    -- Status
    is_read                 BOOLEAN DEFAULT FALSE,
    read_at                 TIMESTAMP WITH TIME ZONE,
    
    -- Expiry
    expires_at              TIMESTAMP WITH TIME ZONE,
    
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

CREATE INDEX idx_notifications_user ON notifications(user_id);
CREATE INDEX idx_notifications_unread ON notifications(user_id, is_read) WHERE is_read = FALSE;
CREATE INDEX idx_notifications_created ON notifications(user_id, created_at DESC);

-- =====================================================
-- TABLE: audit_log
-- PURPOSE: Comprehensive action logging for compliance
-- =====================================================
CREATE TABLE audit_log (
    id                      BIGSERIAL PRIMARY KEY,
    school_id               INTEGER NOT NULL REFERENCES schools(id),
    user_id                 INTEGER REFERENCES users(id),
    
    action                  VARCHAR(50) NOT NULL,  -- 'create', 'update', 'delete', 'login', etc.
    entity_type             VARCHAR(50) NOT NULL,  -- 'user', 'company', 'class', etc.
    entity_id               INTEGER,
    
    -- Change details
    old_values              JSONB,
    new_values              JSONB,
    
    -- Context
    ip_address              INET,
    user_agent              TEXT,
    session_id              VARCHAR(64),
    
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW()
) PARTITION BY RANGE (created_at);

-- Create monthly partitions for audit log
CREATE TABLE audit_log_y2025m01 PARTITION OF audit_log
    FOR VALUES FROM ('2025-01-01') TO ('2025-02-01');
CREATE TABLE audit_log_y2025m02 PARTITION OF audit_log
    FOR VALUES FROM ('2025-02-01') TO ('2025-03-01');
-- ... continue for each month

CREATE INDEX idx_audit_school ON audit_log(school_id);
CREATE INDEX idx_audit_user ON audit_log(user_id);
CREATE INDEX idx_audit_action ON audit_log(entity_type, action);
CREATE INDEX idx_audit_created ON audit_log(created_at);
```

### 2.2.12 Assignment & Grading Tables

```sql
-- =====================================================
-- TABLE: assignments
-- PURPOSE: Teacher-created simulation assignments
-- =====================================================
CREATE TABLE assignments (
    id                      SERIAL PRIMARY KEY,
    class_id                INTEGER NOT NULL REFERENCES classes(id) ON DELETE CASCADE,
    created_by              INTEGER NOT NULL REFERENCES users(id),
    
    title                   VARCHAR(200) NOT NULL,
    description             TEXT,
    instructions            TEXT,
    
    -- Simulation parameters
    industry_template       VARCHAR(50),
    initial_budget          DECIMAL(15, 2),
    required_modules        VARCHAR(50)[],
    difficulty_level        VARCHAR(20) DEFAULT 'medium', -- easy, medium, hard
    
    -- Timing
    start_date              TIMESTAMP WITH TIME ZONE NOT NULL,
    due_date                TIMESTAMP WITH TIME ZONE,
    
    -- Grading
    max_points              INTEGER,
    grading_rubric          JSONB,  -- Structured rubric
    
    is_active               BOOLEAN DEFAULT TRUE,
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

CREATE INDEX idx_assignments_class ON assignments(class_id);
CREATE INDEX idx_assignments_dates ON assignments(start_date, due_date);

-- =====================================================
-- TABLE: assignment_submissions
-- PURPOSE: Student submissions for grading
-- =====================================================
CREATE TABLE assignment_submissions (
    id                      SERIAL PRIMARY KEY,
    assignment_id           INTEGER NOT NULL REFERENCES assignments(id) ON DELETE CASCADE,
    team_id                 INTEGER NOT NULL REFERENCES teams(id),
    company_id              INTEGER NOT NULL REFERENCES simulation_companies(id),
    
    submitted_at            TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    submitted_by            INTEGER NOT NULL REFERENCES users(id),
    
    -- Submission notes
    notes                   TEXT,
    
    -- Grading
    graded_at               TIMESTAMP WITH TIME ZONE,
    graded_by               INTEGER REFERENCES users(id),
    points_earned           INTEGER,
    grade_percent           DECIMAL(5, 2),
    letter_grade            VARCHAR(2),  -- A+, A, B, etc.
    feedback                TEXT,
    
    -- Status
    status                  VARCHAR(20) DEFAULT 'submitted' -- submitted, graded, returned
);

CREATE INDEX idx_submissions_assignment ON assignment_submissions(assignment_id);
CREATE INDEX idx_submissions_team ON assignment_submissions(team_id);
CREATE INDEX idx_submissions_status ON assignment_submissions(status);
```

## 2.3 Row-Level Security Policies

```sql
-- Enable RLS on all tenant tables
ALTER TABLE users ENABLE ROW LEVEL SECURITY;
ALTER TABLE simulation_companies ENABLE ROW LEVEL SECURITY;
ALTER TABLE classes ENABLE ROW LEVEL SECURITY;
-- ... etc for all tenant tables

-- Create policy function
CREATE OR REPLACE FUNCTION get_current_school_id() RETURNS INTEGER AS $$
BEGIN
    RETURN NULLIF(current_setting('app.current_school_id', TRUE), '')::INTEGER;
END;
$$ LANGUAGE plpgsql SECURITY DEFINER;

-- Apply policy to users table
CREATE POLICY tenant_isolation_users ON users
    FOR ALL
    USING (school_id = get_current_school_id());

-- Apply policy to companies table
CREATE POLICY tenant_isolation_companies ON simulation_companies
    FOR ALL
    USING (school_id = get_current_school_id());

-- Similar policies for all tenant tables...
```

## 2.4 Database Migration Strategy

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      DATABASE MIGRATION STRATEGY                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Tool: golang-migrate (for Go backend) or Flyway                            │
│                                                                             │
│  Naming Convention:                                                         │
│  V{version}__{description}.sql                                              │
│  Example: V001__create_schools_table.sql                                    │
│                                                                             │
│  Migration Types:                                                           │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Versioned Migrations (V) - Schema changes, run once               │   │
│  │  Repeatable Migrations (R) - Views, functions, can re-run          │   │
│  │  Baseline Migrations (B) - For existing databases                  │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Rollback Strategy:                                                         │
│  - Each migration has corresponding down migration                          │
│  - Critical migrations tested on staging first                              │
│  - Database backups before major migrations                                 │
│                                                                             │
│  Zero-Downtime Deployment:                                                  │
│  1. Deploy new code that supports both old and new schema                   │
│  2. Run migrations (backward-compatible changes only)                       │
│  3. Verify migration success                                                │
│  4. Deploy code that uses new schema features                               │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

# PART 3: API CONTRACT

## 3.1 API Design Principles

- **Base URL:** `https://api.eduerp.example.com/api/v1`
- **Content-Type:** `application/json` for all requests/responses
- **Authentication:** JWT Bearer token in `Authorization` header
- **Versioning:** URL path versioning (`/api/v1/`, `/api/v2/`)
- **Error Format:** Standardized error response structure
- **Pagination:** Cursor-based for large collections

## 3.2 Authentication Endpoints

### 3.2.1 OAuth Login Initiation

```
POST /auth/login

Description: Initiate OAuth login flow

Request Body:
{
    "provider": "google" | "microsoft",
    "code": "string",              // Authorization code from OAuth provider
    "code_verifier": "string",     // PKCE verifier
    "redirect_uri": "string"       // Must match registered URI
}

Response 200 OK:
{
    "success": true,
    "data": {
        "access_token": "eyJhbGciOiJSUzI1NiIs...",
        "refresh_token": "dGhpcyBpcyBhIHJlZnJlc2g...",
        "expires_in": 900,           // 15 minutes
        "token_type": "Bearer",
        "user": {
            "id": 123,
            "email": "jan.desmet@mijnschool.be",
            "display_name": "Jan De Smet",
            "role": "student",
            "school": {
                "id": 1,
                "name": "Mijn School",
                "default_language": "nl-BE"
            }
        }
    }
}

Response 401 Unauthorized:
{
    "success": false,
    "error": {
        "code": "INVALID_DOMAIN",
        "message": "Dit e-mailadres is niet toegestaan. Gebruik je schoolaccount.",
        "details": {
            "allowed_domains": ["mijnschool.be"],
            "provided_domain": "gmail.com"
        }
    }
}

Response 403 Forbidden:
{
    "success": false,
    "error": {
        "code": "ACCOUNT_DISABLED",
        "message": "Je account is uitgeschakeld. Neem contact op met je leraar."
    }
}
```

### 3.2.2 Token Refresh

```
POST /auth/refresh

Description: Refresh access token using refresh token

Headers:
Authorization: Bearer {refresh_token}

Response 200 OK:
{
    "success": true,
    "data": {
        "access_token": "eyJhbGciOiJSUzI1NiIs...",
        "expires_in": 900,
        "token_type": "Bearer"
    }
}

Response 401 Unauthorized:
{
    "success": false,
    "error": {
        "code": "TOKEN_EXPIRED",
        "message": "Je sessie is verlopen. Log opnieuw in."
    }
}
```

### 3.2.3 Logout

```
POST /auth/logout

Description: Revoke current session

Headers:
Authorization: Bearer {access_token}

Request Body:
{
    "revoke_all": false  // If true, revoke all user sessions
}

Response 200 OK:
{
    "success": true,
    "data": {
        "message": "Succesvol uitgelogd"
    }
}
```

## 3.3 User Management Endpoints

### 3.3.1 Get Current User

```
GET /users/me

Description: Get current authenticated user's profile

Headers:
Authorization: Bearer {access_token}

Response 200 OK:
{
    "success": true,
    "data": {
        "id": 123,
        "email": "jan.desmet@mijnschool.be",
        "display_name": "Jan De Smet",
        "username": "jan.desmet",
        "avatar_url": "https://storage.eduerp.com/avatars/123.jpg",
        "banner_url": "https://storage.eduerp.com/banners/123.jpg",
        "bio": "5de jaar Economie - Geïnteresseerd in ondernemerschap",
        "role": "student",
        "school_id": 1,
        "class_ids": [5, 6],
        "team_ids": [12],
        "settings": {
            "language": "nl-BE",
            "theme": "dark",
            "font_size": "medium",
            "animation_preference": "full",
            "energy_saving_mode": false
        },
        "privacy": {
            "profile_visibility": "friends",
            "friend_requests_allowed": "class"
        },
        "statistics": {
            "companies_created": 3,
            "total_simulation_hours": 45.5,
            "best_profit_achieved": 125000.00,
            "current_class": "5de jaar Economie A",
            "current_team": "Team Alpha"
        },
        "created_at": "2025-09-01T08:00:00Z",
        "last_login_at": "2026-03-26T10:30:00Z"
    }
}
```

### 3.3.2 Update User Profile

```
PATCH /users/me

Description: Update current user's profile

Headers:
Authorization: Bearer {access_token}

Request Body:
{
    "display_name": "Jan De Smet",
    "bio": "Nieuwe bio tekst",
    "settings": {
        "theme": "light",
        "font_size": "large"
    },
    "privacy": {
        "profile_visibility": "class"
    }
}

Response 200 OK:
{
    "success": true,
    "data": {
        "id": 123,
        "display_name": "Jan De Smet",
        "bio": "Nieuwe bio tekst",
        "settings": {
            "theme": "light",
            "font_size": "large"
        },
        "updated_at": "2026-03-26T11:00:00Z"
    }
}

Response 400 Bad Request:
{
    "success": false,
    "error": {
        "code": "VALIDATION_ERROR",
        "message": "De opgegeven gegevens zijn ongeldig.",
        "details": {
            "bio": "Bio mag maximaal 500 tekens bevatten."
        }
    }
}
```

### 3.3.3 Upload Avatar

```
POST /users/me/avatar

Description: Upload user avatar image

Headers:
Authorization: Bearer {access_token}
Content-Type: multipart/form-data

Request Body (multipart):
- file: image file (JPEG/PNG, max 2MB, max 512x512px)

Response 200 OK:
{
    "success": true,
    "data": {
        "avatar_url": "https://storage.eduerp.com/avatars/123_abc123.jpg"
    }
}

Response 400 Bad Request:
{
    "success": false,
    "error": {
        "code": "INVALID_FILE",
        "message": "Ongeldig bestand. Gebruik JPG of PNG, maximaal 2MB."
    }
}
```

### 3.3.4 List Users (Admin/Teacher Only)

```
GET /users

Description: List users in school (paginated)

Headers:
Authorization: Bearer {access_token}

Query Parameters:
- role: string (optional) - Filter by role
- class_id: integer (optional) - Filter by class
- is_active: boolean (optional) - Filter by status
- search: string (optional) - Search in name/email
- cursor: string (optional) - Pagination cursor
- limit: integer (optional, default 20, max 100)

Response 200 OK:
{
    "success": true,
    "data": {
        "items": [
            {
                "id": 123,
                "email": "jan.desmet@mijnschool.be",
                "display_name": "Jan De Smet",
                "role": "student",
                "is_active": true,
                "last_login_at": "2026-03-26T10:30:00Z"
            }
        ],
        "pagination": {
            "next_cursor": "eyJpZCI6MTIzfQ==",
            "has_more": true
        }
    }
}

Response 403 Forbidden:
{
    "success": false,
    "error": {
        "code": "INSUFFICIENT_PERMISSIONS",
        "message": "Je hebt geen toestemming om gebruikers te bekijken."
    }
}
```

### 3.3.5 Bulk Create Users (Admin Only)

```
POST /users/bulk

Description: Bulk create student accounts from CSV data

Headers:
Authorization: Bearer {access_token}

Request Body:
{
    "users": [
        {
            "email": "student1@mijnschool.be",
            "display_name": "Student Een",
            "class_id": 5
        },
        {
            "email": "student2@mijnschool.be",
            "display_name": "Student Twee",
            "class_id": 5
        }
    ],
    "send_welcome_email": false
}

Response 200 OK:
{
    "success": true,
    "data": {
        "created": 2,
        "failed": 0,
        "results": [
            {
                "email": "student1@mijnschool.be",
                "status": "created",
                "user_id": 124
            },
            {
                "email": "student2@mijnschool.be",
                "status": "created",
                "user_id": 125
            }
        ]
    }
}
```

## 3.4 Class Management Endpoints

### 3.4.1 Create Class

```
POST /classes

Description: Create a new class (Teacher/Admin)

Headers:
Authorization: Bearer {access_token}

Request Body:
{
    "name": "5de jaar Economie A",
    "description": "Economie richting, eerste klas",
    "academic_year": "2025-2026",
    "teacher_id": 10,
    "max_team_size": 4,
    "allowed_modules": ["finance", "sales", "inventory"],
    "simulation_time_scale": "accelerated"
}

Response 201 Created:
{
    "success": true,
    "data": {
        "id": 7,
        "name": "5de jaar Economie A",
        "teacher_id": 10,
        "join_code": "ECON5A2025",  // For students to join
        "created_at": "2026-03-26T11:00:00Z"
    }
}
```

### 3.4.2 Get Class Details

```
GET /classes/{class_id}

Description: Get class details with student list

Headers:
Authorization: Bearer {access_token}

Response 200 OK:
{
    "success": true,
    "data": {
        "id": 7,
        "name": "5de jaar Economie A",
        "description": "Economie richting, eerste klas",
        "academic_year": "2025-2026",
        "teacher": {
            "id": 10,
            "display_name": "Mevr. Jansen"
        },
        "students": [
            {
                "id": 123,
                "display_name": "Jan De Smet",
                "joined_at": "2025-09-01T08:00:00Z",
                "team_id": 12
            }
        ],
        "teams": [
            {
                "id": 12,
                "name": "Team Alpha",
                "member_count": 4,
                "company_name": "Alpha BV"
            }
        ],
        "settings": {
            "max_team_size": 4,
            "allowed_modules": ["finance", "sales", "inventory"],
            "simulation_time_scale": "accelerated"
        }
    }
}
```

### 3.4.3 Add Student to Class

```
POST /classes/{class_id}/students

Description: Add student to class (Teacher/Admin)

Headers:
Authorization: Bearer {access_token}

Request Body:
{
    "student_id": 124,
    "team_id": null  // Optional: assign to team immediately
}

Response 200 OK:
{
    "success": true,
    "data": {
        "membership_id": 45,
        "student": {
            "id": 124,
            "display_name": "Student Twee"
        },
        "joined_at": "2026-03-26T11:00:00Z"
    }
}
```

### 3.4.4 Create Team

```
POST /classes/{class_id}/teams

Description: Create a team within a class

Headers:
Authorization: Bearer {access_token}

Request Body:
{
    "name": "Team Beta",
    "student_ids": [123, 124, 125],
    "role_assignments": {
        "123": "ceo",
        "124": "cfo",
        "125": "sales_manager"
    }
}

Response 201 Created:
{
    "success": true,
    "data": {
        "id": 13,
        "name": "Team Beta",
        "class_id": 7,
        "members": [
            {
                "student_id": 123,
                "display_name": "Jan De Smet",
                "role": "ceo"
            },
            {
                "student_id": 124,
                "display_name": "Student Twee",
                "role": "cfo"
            },
            {
                "student_id": 125,
                "display_name": "Student Drie",
                "role": "sales_manager"
            }
        ],
        "created_at": "2026-03-26T11:00:00Z"
    }
}
```

## 3.5 Simulation Company Endpoints

### 3.5.1 Create Company

```
POST /companies

Description: Create a new simulation company

Headers:
Authorization: Bearer {access_token}

Request Body:
{
    "name": "Mijn Bedrijf BV",
    "industry_template": "retail_clothing",
    "initial_budget": 100000.00,
    "currency_code": "EUR",
    "team_id": 12,  // Optional: associate with team
    "time_scale": "accelerated",
    "simulation_speed": 2,
    "logo_url": null
}

Response 201 Created:
{
    "success": true,
    "data": {
        "id": 45,
        "name": "Mijn Bedrijf BV",
        "industry_template": "retail_clothing",
        "initial_budget": 100000.00,
        "current_budget": 100000.00,
        "time_scale": "accelerated",
        "simulation_speed": 2,
        "current_simulated_date": "2025-01-01",
        "status": "active",
        "modules": [
            {
                "type": "finance",
                "is_enabled": true
            },
            {
                "type": "sales",
                "is_enabled": true
            }
        ],
        "created_at": "2026-03-26T11:00:00Z"
    }
}
```

### 3.5.2 Get Company Details

```
GET /companies/{company_id}

Description: Get company details with current state

Headers:
Authorization: Bearer {access_token}

Response 200 OK:
{
    "success": true,
    "data": {
        "id": 45,
        "name": "Mijn Bedrijf BV",
        "logo_url": "https://storage.eduerp.com/logos/45.jpg",
        "industry_template": "retail_clothing",
        "initial_budget": 100000.00,
        "current_budget": 87500.00,
        "currency_code": "EUR",
        "time_scale": "accelerated",
        "simulation_speed": 2,
        "current_simulated_date": "2025-03-15",
        "simulation_start_date": "2025-01-01",
        "status": "active",
        "team": {
            "id": 12,
            "name": "Team Alpha",
            "members": [
                {
                    "id": 123,
                    "display_name": "Jan De Smet",
                    "role": "ceo",
                    "is_online": true
                }
            ]
        },
        "modules": [
            {
                "type": "finance",
                "is_enabled": true,
                "unlocked_at": "2026-03-26T11:00:00Z"
            }
        ],
        "current_kpis": {
            "revenue": 25000.00,
            "expenses": 15000.00,
            "net_profit": 10000.00,
            "profit_margin": 0.40,
            "cash_on_hand": 87500.00,
            "customer_satisfaction": 0.82,
            "employee_satisfaction": 0.75
        }
    }
}
```

### 3.5.3 Get KPI History

```
GET /companies/{company_id}/kpis

Description: Get KPI time-series data for charts

Headers:
Authorization: Bearer {access_token}

Query Parameters:
- start_date: date (optional)
- end_date: date (optional)
- metrics: string[] (optional) - Specific metrics to return

Response 200 OK:
{
    "success": true,
    "data": {
        "company_id": 45,
        "metrics": ["revenue", "expenses", "net_profit", "cash_on_hand"],
        "data_points": [
            {
                "date": "2025-01-01",
                "revenue": 0,
                "expenses": 5000.00,
                "net_profit": -5000.00,
                "cash_on_hand": 95000.00
            },
            {
                "date": "2025-02-01",
                "revenue": 15000.00,
                "expenses": 10000.00,
                "net_profit": 5000.00,
                "cash_on_hand": 90000.00
            },
            {
                "date": "2025-03-01",
                "revenue": 25000.00,
                "expenses": 15000.00,
                "net_profit": 10000.00,
                "cash_on_hand": 87500.00
            }
        ]
    }
}
```

## 3.6 Finance Module Endpoints

### 3.6.1 Get Chart of Accounts

```
GET /companies/{company_id}/finance/accounts

Description: Get company's chart of accounts

Headers:
Authorization: Bearer {access_token}

Response 200 OK:
{
    "success": true,
    "data": {
        "accounts": [
            {
                "id": 1,
                "account_code": "1000",
                "account_name": "Kas",
                "account_type": "asset",
                "balance": 25000.00
            },
            {
                "id": 2,
                "account_code": "1200",
                "account_name": "Bankrekening",
                "account_type": "asset",
                "balance": 62500.00
            },
            {
                "id": 10,
                "account_code": "4000",
                "account_name": "Verkopen",
                "account_type": "revenue",
                "balance": 25000.00
            },
            {
                "id": 20,
                "account_code": "6000",
                "account_name": "Aankopen",
                "account_type": "expense",
                "balance": 10000.00
            }
        ]
    }
}
```

### 3.6.2 Get Ledger Entries

```
GET /companies/{company_id}/finance/ledger

Description: Get general ledger entries (paginated)

Headers:
Authorization: Bearer {access_token}

Query Parameters:
- start_date: date (optional)
- end_date: date (optional)
- account_id: integer (optional) - Filter by account
- cursor: string (optional)
- limit: integer (optional, default 50)

Response 200 OK:
{
    "success": true,
    "data": {
        "entries": [
            {
                "id": 1,
                "entry_date": "2025-01-15",
                "debit_account": {
                    "id": 10,
                    "code": "4000",
                    "name": "Verkopen"
                },
                "credit_account": {
                    "id": 2,
                    "code": "1200",
                    "name": "Bankrekening"
                },
                "amount": 5000.00,
                "description": "Verkoop aan klant #001",
                "reference_type": "invoice",
                "reference_id": 1
            }
        ],
        "pagination": {
            "next_cursor": "eyJpZCI6MX0=",
            "has_more": false
        }
    }
}
```

### 3.6.3 Get Financial Report

```
GET /companies/{company_id}/finance/reports/{report_type}

Description: Get financial statement (income statement, balance sheet, cash flow)

Headers:
Authorization: Bearer {access_token}

Path Parameters:
- report_type: string - "income_statement", "balance_sheet", "cash_flow"

Query Parameters:
- period_start: date (required)
- period_end: date (required)

Response 200 OK (Income Statement):
{
    "success": true,
    "data": {
        "report_type": "income_statement",
        "period": {
            "start": "2025-01-01",
            "end": "2025-03-31"
        },
        "revenue": {
            "total": 25000.00,
            "breakdown": [
                {"account": "4000 - Verkopen", "amount": 25000.00}
            ]
        },
        "expenses": {
            "total": 15000.00,
            "breakdown": [
                {"account": "6000 - Aankopen", "amount": 10000.00},
                {"account": "6100 - Lonen", "amount": 4000.00},
                {"account": "6200 - Algemene kosten", "amount": 1000.00}
            ]
        },
        "net_profit": 10000.00,
        "profit_margin": 0.40
    }
}

Response 200 OK (Balance Sheet):
{
    "success": true,
    "data": {
        "report_type": "balance_sheet",
        "as_of": "2025-03-31",
        "assets": {
            "current": {
                "total": 87500.00,
                "items": [
                    {"account": "1000 - Kas", "amount": 25000.00},
                    {"account": "1200 - Bankrekening", "amount": 62500.00}
                ]
            },
            "fixed": {
                "total": 0,
                "items": []
            },
            "total": 87500.00
        },
        "liabilities": {
            "current": {
                "total": 0,
                "items": []
            },
            "long_term": {
                "total": 0,
                "items": []
            },
            "total": 0
        },
        "equity": {
            "share_capital": 100000.00,
            "retained_earnings": -12500.00,
            "total": 87500.00
        },
        "liabilities_and_equity": 87500.00
    }
}
```

## 3.7 Inventory Module Endpoints

### 3.7.1 Get Products

```
GET /companies/{company_id}/inventory/products

Description: Get product catalog with stock levels

Headers:
Authorization: Bearer {access_token}

Query Parameters:
- category: string (optional)
- low_stock: boolean (optional) - Only show low stock items
- search: string (optional)

Response 200 OK:
{
    "success": true,
    "data": {
        "products": [
            {
                "id": 1,
                "sku": "TSH-001-BLK",
                "name": "T-shirt Zwart - M",
                "description": "Katoenen t-shirt, zwart, maat M",
                "category": "kleding",
                "cost_price": 8.50,
                "selling_price": 24.99,
                "current_stock": 45,
                "min_stock_level": 10,
                "reorder_point": 25,
                "unit_of_measure": "stuks",
                "stock_status": "ok"  // ok, low, critical
            },
            {
                "id": 2,
                "sku": "JNS-001-BLU",
                "name": "Jeans Blauw - 32",
                "description": "Slim fit jeans, blauw, maat 32",
                "category": "kleding",
                "cost_price": 25.00,
                "selling_price": 59.99,
                "current_stock": 8,
                "min_stock_level": 10,
                "reorder_point": 25,
                "stock_status": "low"
            }
        ]
    }
}
```

### 3.7.2 Create Purchase Order

```
POST /companies/{company_id}/inventory/purchase-orders

Description: Create a new purchase order

Headers:
Authorization: Bearer {access_token}

Request Body:
{
    "supplier_id": 1,
    "items": [
        {
            "product_id": 2,
            "quantity": 50,
            "unit_price": 22.00
        }
    ],
    "expected_delivery": "2025-04-15"
}

Response 201 Created:
{
    "success": true,
    "data": {
        "id": 10,
        "po_number": "PO-2025-0010",
        "supplier": {
            "id": 1,
            "name": "Fashion Supplies BV"
        },
        "items": [
            {
                "product_id": 2,
                "product_name": "Jeans Blauw - 32",
                "quantity": 50,
                "unit_price": 22.00,
                "total_price": 1100.00
            }
        ],
        "total_amount": 1100.00,
        "status": "draft",
        "order_date": "2025-03-26",
        "expected_delivery": "2025-04-15"
    }
}
```

## 3.8 Messaging Endpoints

### 3.8.1 Get Conversations

```
GET /messages/conversations

Description: Get list of conversations for current user

Headers:
Authorization: Bearer {access_token}

Response 200 OK:
{
    "success": true,
    "data": {
        "conversations": [
            {
                "conversation_id": "123_456",
                "other_user": {
                    "id": 456,
                    "display_name": "Piet Peeters",
                    "avatar_url": "https://...",
                    "is_online": true
                },
                "last_message": {
                    "content": "Hallo, hoe gaat het met het project?",
                    "sent_at": "2026-03-26T10:30:00Z",
                    "is_from_me": false
                },
                "unread_count": 2,
                "streak": {
                    "current": 5,
                    "is_hidden": false
                }
            }
        ]
    }
}
```

### 3.8.2 Get Messages

```
GET /messages/conversations/{conversation_id}

Description: Get messages in a conversation

Headers:
Authorization: Bearer {access_token}

Query Parameters:
- before: string (optional) - Cursor for pagination
- limit: integer (optional, default 50)

Response 200 OK:
{
    "success": true,
    "data": {
        "conversation_id": "123_456",
        "other_user": {
            "id": 456,
            "display_name": "Piet Peeters",
            "avatar_url": "https://..."
        },
        "messages": [
            {
                "id": 1001,
                "content": "Hallo, hoe gaat het met het project?",
                "content_type": "text",
                "sender_id": 456,
                "sent_at": "2026-03-26T10:30:00Z",
                "is_read": true,
                "read_at": "2026-03-26T10:35:00Z"
            },
            {
                "id": 1002,
                "content": "Goed bezig! We zijn bijna klaar met de financiële analyse.",
                "content_type": "text",
                "sender_id": 123,
                "sent_at": "2026-03-26T10:40:00Z",
                "is_read": false
            }
        ],
        "pagination": {
            "has_more": true,
            "next_cursor": "eyJpZCI6OTAwfQ=="
        },
        "streak": {
            "current": 5,
            "longest": 12,
            "is_hidden": false
        }
    }
}
```

### 3.8.3 Send Message

```
POST /messages

Description: Send a direct message

Headers:
Authorization: Bearer {access_token}

Request Body:
{
    "recipient_id": 456,
    "content": "Hallo, hoe gaat het?",
    "content_type": "text"
}

Response 201 Created:
{
    "success": true,
    "data": {
        "id": 1003,
        "recipient_id": 456,
        "content": "Hallo, hoe gaat het?",
        "sent_at": "2026-03-26T11:00:00Z",
        "status": "sent"
    }
}
```

## 3.9 Friend System Endpoints

### 3.9.1 Get Friends

```
GET /friends

Description: Get friends list with online status

Headers:
Authorization: Bearer {access_token}

Response 200 OK:
{
    "success": true,
    "data": {
        "friends": [
            {
                "id": 456,
                "display_name": "Piet Peeters",
                "avatar_url": "https://...",
                "is_online": true,
                "last_seen_at": "2026-03-26T10:30:00Z",
                "friendship": {
                    "since": "2025-09-15T08:00:00Z"
                }
            }
        ],
        "pending_requests": [
            {
                "id": 789,
                "display_name": "Marie Jansen",
                "requested_at": "2026-03-25T14:00:00Z"
            }
        ]
    }
}
```

### 3.9.2 Send Friend Request

```
POST /friends/requests

Description: Send a friend request

Headers:
Authorization: Bearer {access_token}

Request Body:
{
    "user_id": 789
}

Response 201 Created:
{
    "success": true,
    "data": {
        "request_id": 50,
        "status": "pending",
        "requested_at": "2026-03-26T11:00:00Z"
    }
}

Response 400 Bad Request:
{
    "success": false,
    "error": {
        "code": "ALREADY_FRIENDS",
        "message": "Jullie zijn al bevriend."
    }
}
```

### 3.9.3 Respond to Friend Request

```
PATCH /friends/requests/{request_id}

Description: Accept or decline friend request

Headers:
Authorization: Bearer {access_token}

Request Body:
{
    "action": "accept"  // or "decline"
}

Response 200 OK:
{
    "success": true,
    "data": {
        "status": "accepted",
        "friend": {
            "id": 789,
            "display_name": "Marie Jansen"
        }
    }
}
```

## 3.10 Admin Endpoints

### 3.10.1 Get School Settings

```
GET /admin/school/settings

Description: Get school configuration (Admin only)

Headers:
Authorization: Bearer {access_token}

Response 200 OK:
{
    "success": true,
    "data": {
        "school": {
            "id": 1,
            "name": "Mijn School",
            "oauth_domains": ["mijnschool.be"],
            "default_language": "nl-BE",
            "allowed_languages": ["nl-BE", "en-GB", "fr-BE"],
            "features": {
                "streak_enabled": true,
                "friend_system_enabled": true,
                "cross_class_messaging": false
            },
            "theme_restrictions": {
                "locked": false,
                "allowed_themes": ["light", "dark", "high_contrast"]
            }
        }
    }
}
```

### 3.10.2 Update School Settings

```
PATCH /admin/school/settings

Description: Update school configuration (Admin only)

Headers:
Authorization: Bearer {access_token}

Request Body:
{
    "default_language": "nl-BE",
    "features": {
        "streak_enabled": false
    },
    "theme_restrictions": {
        "locked": true,
        "theme": "light"
    }
}

Response 200 OK:
{
    "success": true,
    "data": {
        "message": "Instellingen bijgewerkt."
    }
}
```

### 3.10.3 Get Audit Log

```
GET /admin/audit-log

Description: Get audit log entries (Admin only)

Headers:
Authorization: Bearer {access_token}

Query Parameters:
- start_date: date (required)
- end_date: date (required)
- user_id: integer (optional)
- action: string (optional)
- entity_type: string (optional)

Response 200 OK:
{
    "success": true,
    "data": {
        "entries": [
            {
                "id": 10001,
                "timestamp": "2026-03-26T10:00:00Z",
                "user": {
                    "id": 123,
                    "display_name": "Jan De Smet"
                },
                "action": "update",
                "entity_type": "company",
                "entity_id": 45,
                "old_values": {"name": "Oude Naam"},
                "new_values": {"name": "Nieuwe Naam"},
                "ip_address": "192.168.1.100"
            }
        ]
    }
}
```

## 3.11 Error Response Standard

```
All error responses follow this structure:

{
    "success": false,
    "error": {
        "code": "ERROR_CODE",
        "message": "Human-readable error message in Dutch",
        "details": {}  // Optional additional context
    }
}

Standard Error Codes:
┌────────────────────────┬─────────────────────────────────────────────────────┐
│ Code                   │ Description                                         │
├────────────────────────┼─────────────────────────────────────────────────────┤
│ UNAUTHORIZED           │ Missing or invalid authentication                   │
│ TOKEN_EXPIRED          │ JWT token has expired                               │
│ FORBIDDEN              │ Authenticated but insufficient permissions          │
│ NOT_FOUND              │ Requested resource not found                        │
│ VALIDATION_ERROR       │ Request validation failed                           │
│ RATE_LIMITED           │ Too many requests                                   │
│ INTERNAL_ERROR         │ Server internal error                               │
│ SERVICE_UNAVAILABLE    │ Service temporarily unavailable                     │
│ INVALID_DOMAIN         │ Email domain not allowed for OAuth                  │
│ ACCOUNT_DISABLED       │ User account is disabled                            │
│ DUPLICATE_ENTRY        │ Resource already exists                             │
│ INSUFFICIENT_FUNDS     │ Not enough money for transaction                    │
│ INVALID_OPERATION      │ Operation not allowed in current state              │
└────────────────────────┴─────────────────────────────────────────────────────┘
```

## 3.12 WebSocket Events

### 3.12.1 Connection

```
WebSocket URL: wss://api.eduerp.example.com/ws

Connection Headers:
Authorization: Bearer {access_token}
X-Client-Version: 1.0.0

On Connect:
Server sends:
{
    "type": "connection_established",
    "data": {
        "connection_id": "conn_abc123",
        "server_time": "2026-03-26T11:00:00Z"
    }
}
```

### 3.12.2 Team Room Subscription

```
Client sends:
{
    "type": "subscribe",
    "channel": "team:12",
    "data": {
        "company_id": 45
    }
}

Server confirms:
{
    "type": "subscribed",
    "channel": "team:12",
    "data": {
        "team_members": [
            {"id": 123, "display_name": "Jan", "role": "ceo", "is_online": true}
        ]
    }
}
```

### 3.12.3 Real-time Events

```
Team Member Activity:
{
    "type": "team:member_activity",
    "channel": "team:12",
    "data": {
        "user_id": 124,
        "display_name": "Piet",
        "action": "opened_module",
        "module": "finance",
        "timestamp": "2026-03-26T11:05:00Z"
    }
}

Field Lock (Conflict Prevention):
{
    "type": "team:field_locked",
    "channel": "team:12",
    "data": {
        "user_id": 124,
        "display_name": "Piet",
        "entity_type": "product",
        "entity_id": 1,
        "field": "selling_price",
        "locked_at": "2026-03-26T11:05:00Z"
    }
}

Data Update:
{
    "type": "team:data_updated",
    "channel": "team:12",
    "data": {
        "entity_type": "product",
        "entity_id": 1,
        "changes": {
            "selling_price": {"old": 24.99, "new": 29.99}
        },
        "updated_by": 124,
        "updated_at": "2026-03-26T11:06:00Z"
    }
}

Chat Message:
{
    "type": "team:chat_message",
    "channel": "team:12",
    "data": {
        "message_id": 500,
        "sender_id": 124,
        "display_name": "Piet",
        "content": "Ik heb de prijzen aangepast!",
        "sent_at": "2026-03-26T11:07:00Z"
    }
}

Simulation Event:
{
    "type": "simulation:event",
    "channel": "team:12",
    "data": {
        "event_id": 25,
        "event_type": "market_change",
        "title": "Economische recessie",
        "description": "De markt krimpt met 15% door een economische recessie.",
        "impact": {
            "demand_change": -0.15,
            "duration_days": 30
        },
        "triggered_at": "2026-03-26T11:10:00Z"
    }
}
```

---

# PART 4: C++ PROJECT STRUCTURE

## 4.1 Directory Layout

```
eduerp/
├── CMakeLists.txt                 # Root CMake configuration
├── cmake/
│   ├── modules/                   # Custom CMake find modules
│   │   FindQt6Components.cmake
│   │   Findnlohmann_json.cmake
│   │   Findjwt-cpp.cmake
│   └── options.cmake              # Build options configuration
│
├── docs/                          # Documentation
│   ├── developer/
│   │   ├── BUILD.md              # Build instructions
│   │   ├── ARCHITECTURE.md       # Architecture overview
│   │   └── TESTING.md            # Testing guidelines
│   ├── user/
│   │   ├── nl/                   # Dutch user manual
│   │   ├── en/                   # English user manual
│   │   └── fr/                   # French user manual
│   └── api/                      # API documentation (generated)
│
├── src/                          # Source code
│   ├── main.cpp                  # Application entry point
│   ├── app/                      # Application core
│   │   ├── CMakeLists.txt
│   │   ├── EduERPApplication.cpp
│   │   ├── EduERPApplication.h
│   │   ├── ApplicationConfig.cpp
│   │   ├── ApplicationConfig.h
│   │   ├── CommandLineParser.cpp
│   │   └── CommandLineParser.h
│   │
│   ├── core/                     # Core utilities and base classes
│   │   ├── CMakeLists.txt
│   │   ├── types/
│   │   │   ├── Result.h          # Result<T, E> type
│   │   │   ├── Option.h          # Optional type wrapper
│   │   │   ├── UUID.cpp
│   │   │   ├── UUID.h
│   │   │   ├── DateTime.cpp
│   │   │   └── DateTime.h
│   │   ├── memory/
│   │   │   ├── ObjectPool.h      # Memory pool for frequent allocations
│   │   │   └── BufferPool.cpp
│   │   ├── threading/
│   │   │   ├── ThreadPool.cpp
│   │   │   ├── ThreadPool.h
│   │   │   ├── TaskQueue.cpp
│   │   │   └── TaskQueue.h
│   │   └── utils/
│   │       ├── StringUtils.cpp
│   │       ├── StringUtils.h
│   │       ├── FileUtils.cpp
│   │       ├── FileUtils.h
│   │       ├── Validation.cpp
│   │       └── Validation.h
│   │
│   ├── domain/                   # Domain models (business entities)
│   │   ├── CMakeLists.txt
│   │   ├── user/
│   │   │   ├── User.cpp
│   │   │   ├── User.h
│   │   │   ├── UserRole.h
│   │   │   ├── UserSettings.cpp
│   │   │   └── UserSettings.h
│   │   ├── school/
│   │   │   ├── School.cpp
│   │   │   ├── School.h
│   │   │   ├── Class.cpp
│   │   │   ├── Class.h
│   │   │   ├── Team.cpp
│   │   │   └── Team.h
│   │   └── simulation/
│   │       ├── Company.cpp
│   │       ├── Company.h
│   │       ├── CompanySettings.cpp
│   │       ├── CompanySettings.h
│   │       ├── IndustryTemplate.h
│   │       ├── KPIData.cpp
│   │       └── KPIData.h
│   │
│   ├── simulation/               # ERP Simulation Engine
│   │   ├── CMakeLists.txt
│   │   ├── engine/
│   │   │   ├── SimulationEngine.cpp
│   │   │   ├── SimulationEngine.h
│   │   │   ├── SimulationClock.cpp
│   │   │   ├── SimulationClock.h
│   │   │   ├── EventSystem.cpp
│   │   │   └── EventSystem.h
│   │   ├── modules/
│   │   │   ├── IModule.h
│   │   │   ├── finance/
│   │   │   │   ├── FinanceModule.cpp
│   │   │   │   ├── FinanceModule.h
│   │   │   │   ├── Account.cpp
│   │   │   │   ├── Account.h
│   │   │   │   ├── Ledger.cpp
│   │   │   │   ├── Ledger.h
│   │   │   │   ├── FinancialReport.cpp
│   │   │   │   └── FinancialReport.h
│   │   │   ├── sales/
│   │   │   │   ├── SalesModule.cpp
│   │   │   │   ├── SalesModule.h
│   │   │   │   ├── Customer.cpp
│   │   │   │   ├── Customer.h
│   │   │   │   ├── Pipeline.cpp
│   │   │   │   └── Order.cpp
│   │   │   ├── inventory/
│   │   │   │   ├── InventoryModule.cpp
│   │   │   │   ├── InventoryModule.h
│   │   │   │   ├── Product.cpp
│   │   │   │   ├── Product.h
│   │   │   │   ├── StockManager.cpp
│   │   │   │   └── PurchaseOrder.cpp
│   │   │   ├── hr/
│   │   │   │   ├── HRModule.cpp
│   │   │   │   ├── HRModule.h
│   │   │   │   ├── Employee.cpp
│   │   │   │   ├── Employee.h
│   │   │   │   └── Payroll.cpp
│   │   │   ├── marketing/
│   │   │   │   ├── MarketingModule.cpp
│   │   │   │   ├── MarketingModule.h
│   │   │   │   └── Campaign.cpp
│   │   │   └── logistics/
│   │   │       ├── LogisticsModule.cpp
│   │   │       └── LogisticsModule.h
│   │   ├── ai/
│   │   │   ├── AIAgent.cpp
│   │   │   ├── AIAgent.h
│   │   │   ├── DecisionEngine.cpp
│   │   │   ├── DecisionEngine.h
│   │   │   ├── strategies/
│   │   │   │   ├── OptimalStrategy.cpp
│   │   │   │   ├── BalancedStrategy.cpp
│   │   │   │   ├── RiskyStrategy.cpp
│   │   │   │   └── PoorStrategy.cpp
│   │   │   └── templates/
│   │   │       ├── IndustryTemplateData.cpp
│   │   │       └── IndustryTemplateData.h
│   │   └── templates/
│   │       ├── retail_clothing.json
│   │       ├── tech_hardware.json
│   │       ├── logistics_delivery.json
│   │       ├── accounting_services.json
│   │       ├── food_beverage.json
│   │       ├── ecommerce_marketplace.json
│   │       ├── tech_giant.json
│   │       ├── semiconductor.json
│   │       └── belgian_sme.json
│   │
│   ├── services/                 # Business logic services
│   │   ├── CMakeLists.txt
│   │   ├── auth/
│   │   │   ├── AuthService.cpp
│   │   │   ├── AuthService.h
│   │   │   ├── TokenManager.cpp
│   │   │   └── TokenManager.h
│   │   ├── user/
│   │   │   ├── UserService.cpp
│   │   │   └── UserService.h
│   │   ├── company/
│   │   │   ├── CompanyService.cpp
│   │   │   └── CompanyService.h
│   │   ├── messaging/
│   │   │   ├── MessagingService.cpp
│   │   │   ├── MessagingService.h
│   │   │   ├── ChatStreakCalculator.cpp
│   │   │   └── ChatStreakCalculator.h
│   │   ├── social/
│   │   │   ├── FriendService.cpp
│   │   │   └── FriendService.h
│   │   └── sync/
│   │       ├── SyncService.cpp
│   │       ├── SyncService.h
│   │       ├── ConflictResolver.cpp
│   │       └── ConflictResolver.h
│   │
│   ├── infrastructure/           # Technical infrastructure
│   │   ├── CMakeLists.txt
│   │   ├── network/
│   │   │   ├── HttpClient.cpp
│   │   │   ├── HttpClient.h
│   │   │   ├── WebSocketClient.cpp
│   │   │   ├── WebSocketClient.h
│   │   │   ├── RequestBuilder.cpp
│   │   │   └── ResponseParser.cpp
│   │   ├── storage/
│   │   │   ├── LocalCache.cpp
│   │   │   ├── LocalCache.h
│   │   │   ├── SQLiteDatabase.cpp
│   │   │   ├── SQLiteDatabase.h
│   │   │   ├── CacheInvalidator.cpp
│   │   │   └── Migrations/
│   │   │       ├── V001__InitialSchema.sql
│   │   │       └── V002__AddMessagesTable.sql
│   │   ├── security/
│   │   │   ├── WindowsCredentialStore.cpp
│   │   │   ├── WindowsCredentialStore.h
│   │   │   ├── Encryption.cpp
│   │   │   └── Encryption.h
│   │   ├── logging/
│   │   │   ├── Logger.cpp
│   │   │   ├── Logger.h
│   │   │   ├── LogLevel.h
│   │   │   └── FileRotator.cpp
│   │   ├── config/
│   │   │   ├── ConfigManager.cpp
│   │   │   ├── ConfigManager.h
│   │   │   └── ConfigKeys.h
│   │   └── i18n/
│   │       ├── I18nManager.cpp
│   │       ├── I18nManager.h
│   │       ├── Locale.cpp
│   │       └── Locale.h
│   │
│   ├── ui/                       # User Interface (Qt6/QML)
│   │   ├── CMakeLists.txt
│   │   ├── main.qml              # Main QML entry
│   │   ├── components/           # Reusable UI components
│   │   │   ├── CMakeLists.txt
│   │   │   ├── EduButton.qml
│   │   │   ├── EduTextField.qml
│   │   │   ├── EduCard.qml
│   │   │   ├── EduTable.qml
│   │   │   ├── EduChart.qml
│   │   │   ├── EduDialog.qml
│   │   │   ├── EduAvatar.qml
│   │   │   ├── EduBadge.qml
│   │   │   ├── EduTooltip.qml
│   │   │   ├── EduLoadingIndicator.qml
│   │   │   ├── EduEmptyState.qml
│   │   │   └── EduNotification.qml
│   │   ├── layouts/              # Layout components
│   │   │   ├── MainLayout.qml
│   │   │   ├── Sidebar.qml
│   │   │   ├── TopBar.qml
│   │   │   └── ContentArea.qml
│   │   ├── views/                # Main view screens
│   │   │   ├── auth/
│   │   │   │   ├── LoginView.qml
│   │   │   │   └── OAuthCallbackView.qml
│   │   │   ├── dashboard/
│   │   │   │   ├── DashboardView.qml
│   │   │   │   ├── StudentDashboard.qml
│   │   │   │   ├── TeacherDashboard.qml
│   │   │   │   └── AdminDashboard.qml
│   │   │   ├── company/
│   │   │   │   ├── CompanyListView.qml
│   │   │   │   ├── CompanyCreateView.qml
│   │   │   │   └── CompanyDetailView.qml
│   │   │   ├── simulation/
│   │   │   │   ├── SimulationView.qml
│   │   │   │   ├── modules/
│   │   │   │   │   ├── FinanceModuleView.qml
│   │   │   │   │   ├── SalesModuleView.qml
│   │   │   │   │   ├── InventoryModuleView.qml
│   │   │   │   │   ├── HRModuleView.qml
│   │   │   │   │   ├── MarketingModuleView.qml
│   │   │   │   │   └── LogisticsModuleView.qml
│   │   │   │   └── components/
│   │   │   │       ├── KPIDashboard.qml
│   │   │   │       ├── EventNotification.qml
│   │   │   │       └── DecisionLog.qml
│   │   │   ├── messaging/
│   │   │   │   ├── ConversationsView.qml
│   │   │   │   ├── ChatView.qml
│   │   │   │   └── components/
│   │   │   │       ├── MessageBubble.qml
│   │   │   │       ├── StreakIndicator.qml
│   │   │   │       └── TypingIndicator.qml
│   │   │   ├── social/
│   │   │   │   ├── FriendsView.qml
│   │   │   │   └── FriendRequestsView.qml
│   │   │   ├── profile/
│   │   │   │   ├── ProfileView.qml
│   │   │   │   ├── ProfileEditView.qml
│   │   │   │   └── SettingsView.qml
│   │   │   └── admin/
│   │   │       ├── UserManagementView.qml
│   │   │       ├── ClassManagementView.qml
│   │   │       ├── SchoolSettingsView.qml
│   │   │       └── AuditLogView.qml
│   │   ├── dialogs/              # Dialog components
│   │   │   ├── ConfirmationDialog.qml
│   │   │   ├── ErrorDialog.qml
│   │   │   ├── InputDialog.qml
│   │   │   └── ProgressDialog.qml
│   │   ├── animations/           # Animation definitions
│   │   │   ├── AnimationConfig.qml
│   │   │   ├── SpringAnimation.qml
│   │   │   └── FadeAnimation.qml
│   │   └── theme/                # Theming system
│   │       ├── ThemeManager.cpp
│   │       ├── ThemeManager.h
│   │       ├── Theme.qml
│   │       ├── LightTheme.qml
│   │       ├── DarkTheme.qml
│   │       ├── HighContrastTheme.qml
│   │       └── CustomThemeBuilder.qml
│   │
│   └── controllers/              # UI Controllers (C++ backend for QML)
│       ├── CMakeLists.txt
│       ├── AuthController.cpp
│       ├── AuthController.h
│       ├── DashboardController.cpp
│       ├── DashboardController.h
│       ├── CompanyController.cpp
│       ├── CompanyController.h
│       ├── SimulationController.cpp
│       ├── SimulationController.h
│       ├── MessagingController.cpp
│       ├── MessagingController.h
│       ├── ProfileController.cpp
│       ├── ProfileController.h
│       └── AdminController.cpp
│           └── AdminController.h
│
├── tests/                        # Test suite
│   ├── CMakeLists.txt
│   ├── unit/                     # Unit tests
│   │   ├── core/
│   │   │   ├── test_types.cpp
│   │   │   └── test_utils.cpp
│   │   ├── domain/
│   │   │   ├── test_user.cpp
│   │   │   └── test_company.cpp
│   │   ├── simulation/
│   │   │   ├── test_finance_module.cpp
│   │   │   └── test_simulation_engine.cpp
│   │   └── services/
│   │       ├── test_auth_service.cpp
│   │       └── test_sync_service.cpp
│   ├── integration/              # Integration tests
│   │   ├── test_api_client.cpp
│   │   ├── test_websocket.cpp
│   │   └── test_offline_sync.cpp
│   ├── ui/                       # UI tests (Qt Test)
│   │   └── test_components.cpp
│   ├── fixtures/                 # Test data
│   │   ├── mock_responses/
│   │   └── test_database.sql
│   └── test_main.cpp             # Test runner entry
│
├── resources/                    # Application resources
│   ├── icons/                    # Application icons
│   │   ├── app_icon.ico
│   │   ├── app_icon.png
│   │   └── module_icons/
│   ├── images/                   # Static images
│   │   ├── splash_screen.png
│   │   ├── empty_states/
│   │   └── illustrations/
│   ├── fonts/                    # Custom fonts
│   │   └── Inter/
│   ├── translations/             # i18n files
│   │   ├── EduERP_nl_BE.ts
│   │   ├── EduERP_en_GB.ts
│   │   ├── EduERP_fr_BE.ts
│   │   └── translations.qrc
│   ├── sounds/                   # UI sounds (optional)
│   │   └── notification.wav
│   └── qml_resources.qrc         # QML resource file
│
├── third_party/                  # Third-party dependencies
│   ├── CMakeLists.txt
│   ├── nlohmann_json/            # JSON library (header-only)
│   ├── jwt-cpp/                  # JWT library (header-only)
│   ├── sqlitecpp/                # SQLite wrapper
│   └── cmake/                    # External project configs
│       └── AddGoogleTest.cmake
│
├── scripts/                      # Build and utility scripts
│   ├── build_windows.bat
│   ├── build_linux.sh
│   ├── package_installer.iss     # Inno Setup script
│   └── generate_translations.py
│
├── installer/                    # Installer resources
│   ├── windows/
│   │   ├── EduERP.iss
│   │   ├── LICENSE.txt
│   │   └── README.txt
│   └── assets/
│       ├── banner.bmp
│       └── wizard_image.bmp
│
├── .github/                      # GitHub configuration
│   └── workflows/
│       ├── build.yml
│       ├── release.yml
│       └── codeql.yml
│
├── .clang-format                 # Code formatting config
├── .clang-tidy                   # Static analysis config
├── .gitignore
├── LICENSE
├── README.md
└── VERSION                       # Current version file
```

## 4.2 CMake Configuration Overview

```cmake
# Root CMakeLists.txt structure

cmake_minimum_required(VERSION 3.20)
project(EduERP VERSION 1.0.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(EDUERP_BUILD_TESTS "Build tests" ON)
option(EDUERP_BUILD_DOCS "Build documentation" OFF)
option(EDUERP_ENABLE_SANITIZERS "Enable sanitizers" OFF)
option(EDUERP_STATIC_ANALYSIS "Enable static analysis" OFF)

# Find dependencies
find_package(Qt6 6.8 REQUIRED COMPONENTS 
    Core 
    Gui 
    Widgets 
    Qml 
    Quick 
    QuickControls2 
    Network 
    Sql
    WebSockets
)

find_package(nlohmann_json 3.12 REQUIRED)
find_package(jwt-cpp 0.7 REQUIRED)
find_package(SQLiteCpp 3.3 REQUIRED)
find_package(CURL 8.0 REQUIRED)

# Third-party dependencies
add_subdirectory(third_party)

# Source directories
add_subdirectory(src/core)
add_subdirectory(src/domain)
add_subdirectory(src/simulation)
add_subdirectory(src/services)
add_subdirectory(src/infrastructure)
add_subdirectory(src/ui)
add_subdirectory(src/controllers)
add_subdirectory(src/app)

# Main executable
add_executable(EduERP WIN32
    src/main.cpp
    resources/qml_resources.qrc
    resources/translations.qrc
)

target_link_libraries(EduERP PRIVATE
    EduERP::App
    EduERP::Controllers
    EduERP::UI
    Qt6::WinMain
)

# Tests
if(EDUERP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation
install(TARGETS EduERP
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

# CPack configuration for installer
set(CPACK_PACKAGE_NAME "EduERP")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR "NSIS")
include(CPack)
```

## 4.3 Key Design Patterns

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      DESIGN PATTERNS USED                                    │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  1. Model-View-Controller (MVC) Variant                                     │
│  ═══════════════════════════════════════                                    │
│                                                                             │
│     QML Views ◄────► C++ Controllers ◄────► C++ Services ◄────► Domain     │
│        │                    │                    │               Models     │
│        │                    │                    │                          │
│        └──── UI Layer ──────┴── Application ────┴── Domain ────┘           │
│                                                                             │
│  2. Repository Pattern (Data Access)                                        │
│  ═══════════════════════════════════                                        │
│                                                                             │
│     IUserRepository (interface)                                             │
│           ▲                                                                 │
│     ┌─────┴─────┐                                                           │
│     │           │                                                           │
│  RemoteUser  CachedUser                                                     │
│  Repository  Repository                                                     │
│  (HTTP API)  (SQLite)                                                       │
│                                                                             │
│  3. Service Layer Pattern                                                   │
│  ══════════════════════════                                                 │
│                                                                             │
│     UserService ──► coordinates UserRepository, AuthService, Validation    │
│     CompanyService ──► coordinates CompanyRepository, SimulationEngine     │
│                                                                             │
│  4. Observer Pattern (Qt Signals/Slots)                                     │
│  ══════════════════════════════════════                                     │
│                                                                             │
│     WebSocketClient::messageReceived ──► Controller::onMessageReceived     │
│                                         ──► View::updateDisplay()          │
│                                                                             │
│  5. Factory Pattern (Module Creation)                                       │
│  ════════════════════════════════════                                       │
│                                                                             │
│     IModule* createModule(ModuleType type, Company* company) {              │
│         switch(type) {                                                      │
│             case Finance: return new FinanceModule(company);                │
│             case Sales: return new SalesModule(company);                    │
│             // ...                                                          │
│         }                                                                   │
│     }                                                                       │
│                                                                             │
│  6. Strategy Pattern (AI Decision Making)                                   │
│  ════════════════════════════════════════                                   │
│                                                                             │
│     IDecisionStrategy (interface)                                           │
│           ▲                                                                 │
│     ┌─────┼─────┬─────────┐                                                 │
│     │     │     │         │                                                 │
│  Optimal Balanced Risky  Poor                                              │
│  Strategy Strategy Strategy Strategy                                        │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 4.4 Namespace Organization

```cpp
// Namespace hierarchy

namespace EduERP {
    
    // Core utilities
    namespace Core {
        namespace Types { }
        namespace Memory { }
        namespace Threading { }
        namespace Utils { }
    }
    
    // Domain models
    namespace Domain {
        namespace User { }
        namespace School { }
        namespace Simulation { }
    }
    
    // Simulation engine
    namespace Simulation {
        namespace Engine { }
        namespace Modules {
            namespace Finance { }
            namespace Sales { }
            namespace Inventory { }
            namespace HR { }
            namespace Marketing { }
            namespace Logistics { }
        }
        namespace AI { }
    }
    
    // Business services
    namespace Services {
        namespace Auth { }
        namespace User { }
        namespace Company { }
        namespace Messaging { }
        namespace Social { }
        namespace Sync { }
    }
    
    // Infrastructure
    namespace Infrastructure {
        namespace Network { }
        namespace Storage { }
        namespace Security { }
        namespace Logging { }
        namespace Config { }
        namespace I18n { }
    }
    
    // UI Controllers
    namespace UI {
        namespace Controllers { }
    }
    
} // namespace EduERP
```

---

# PART 5: UI/UX SPECIFICATION

## 5.1 Design System

### 5.1.1 Color Palette

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         COLOR SYSTEM                                         │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  PRIMARY PALETTE                                                            │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Primary         │ #1976D2 │ rgb(25, 118, 210)  │ Blauw (trust)     │   │
│  │  Primary Light   │ #63A4FF │ rgb(99, 164, 255)  │ Hover states      │   │
│  │  Primary Dark    │ #004BA0 │ rgb(0, 75, 160)    │ Active states     │   │
│  │  Secondary       │ #388E3C │ rgb(56, 142, 60)   │ Groen (success)   │   │
│  │  Secondary Light │ #6ABF69 │ rgb(106, 191, 105) │ Success states    │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  SEMANTIC COLORS                                                            │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Success         │ #4CAF50 │ Groen - Positieve acties, winst      │   │
│  │  Warning         │ #FF9800 │ Oranje - Waarschuwingen              │   │
│  │  Error           │ #F44336 │ Rood - Fouten, verlies               │   │
│  │  Info            │ #2196F3 │ Blauw - Informatieve berichten       │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  LIGHT THEME BACKGROUNDS                                                    │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Background      │ #FAFAFA │ Hoofdachtergrond                     │   │
│  │  Surface         │ #FFFFFF │ Kaarten, panelen                     │   │
│  │  Surface Variant │ #F5F5F5 │ Alternatieve oppervlakken            │   │
│  │  Divider         │ #E0E0E0 │ Scheidingen                          │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  DARK THEME BACKGROUNDS                                                     │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Background      │ #121212 │ Hoofdachtergrond                     │   │
│  │  Surface         │ #1E1E1E │ Kaarten, panelen                     │   │
│  │  Surface Variant │ #2C2C2C │ Alternatieve oppervlakken            │   │
│  │  Divider         │ #424242 │ Scheidingen                          │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  TEXT COLORS                                                                │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  On Background   │ #212121 (L) / #FFFFFF (D) │ Primaire tekst    │   │
│  │  On Surface      │ #424242 (L) / #E0E0E0 (D) │ Secundaire tekst  │   │
│  │  Disabled        │ #9E9E9E (L) / #757575 (D) │ Uitgeschakeld     │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  HIGH CONTRAST THEME (Accessibility)                                        │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Background      │ #000000 │ Zuiver zwart                         │   │
│  │  Surface         │ #000000 │ Zuiver zwart                         │   │
│  │  Text            │ #FFFFFF │ Zuiver wit                           │   │
│  │  Primary         │ #FFFF00 │ Geel (WCAG AAA)                      │   │
│  │  Border          │ #FFFFFF │ Witte randen voor focus              │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 5.1.2 Typography Scale

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      TYPOGRAPHY SYSTEM                                       │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Font Family: Inter (primary), Segoe UI (fallback for Windows)              │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Style           │ Size  │ Weight │ Line  │ Letter │ Usage          │   │
│  │                  │       │        │ Height│ Spacing│                │   │
│  ├─────────────────────────────────────────────────────────────────────┤   │
│  │  Display L       │ 48px  │ 300    │ 56px  │ -0.5px │ Hero titles    │   │
│  │  Display M       │ 36px  │ 400    │ 44px  │ -0.25px│ Page titles    │   │
│  │  Headline 1      │ 28px  │ 500    │ 36px  │ 0      │ Section titles │   │
│  │  Headline 2      │ 24px  │ 500    │ 32px  │ 0      │ Card titles    │   │
│  │  Headline 3      │ 20px  │ 500    │ 28px  │ 0.15px │ Subsection     │   │
│  │  Body Large      │ 16px  │ 400    │ 24px  │ 0.5px  │ Primary text   │   │
│  │  Body Medium     │ 14px  │ 400    │ 20px  │ 0.25px │ Secondary text │   │
│  │  Body Small      │ 12px  │ 400    │ 16px  │ 0.4px  │ Captions       │   │
│  │  Label Large     │ 14px  │ 500    │ 20px  │ 1.25px │ Button text    │   │
│  │  Label Medium    │ 12px  │ 500    │ 16px  │ 1px    │ Tags, badges   │   │
│  │  Label Small     │ 11px  │ 500    │ 16px  │ 0.5px  │ Table headers  │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Font Size Multipliers (Accessibility):                                     │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Small:    0.875x (14px base)                                       │   │
│  │  Medium:   1.000x (16px base) - Default                             │   │
│  │  Large:    1.125x (18px base)                                       │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 5.1.3 Spacing Scale

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                       SPACING SYSTEM                                         │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Base Unit: 4px                                                             │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Token     │ Value  │ Usage                                          │   │
│  ├─────────────────────────────────────────────────────────────────────┤   │
│  │  space-0   │ 0px    │ No spacing                                     │   │
│  │  space-1   │ 4px    │ Tight spacing, icon padding                    │   │
│  │  space-2   │ 8px    │ Small gaps, inline elements                    │   │
│  │  space-3   │ 12px   │ Compact spacing                                │   │
│  │  space-4   │ 16px   │ Default padding, card gutters                  │   │
│  │  space-5   │ 20px   │ Medium spacing                                 │   │
│  │  space-6   │ 24px   │ Section padding                                │   │
│  │  space-8   │ 32px   │ Large gaps, page margins                       │   │
│  │  space-10  │ 40px   │ Extra large spacing                            │   │
│  │  space-12  │ 48px   │ Section separators                             │   │
│  │  space-16  │ 64px   │ Major section breaks                           │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Border Radius:                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  radius-sm   │ 4px   │ Small elements, tags                          │   │
│  │  radius-md   │ 8px   │ Buttons, inputs, cards                        │   │
│  │  radius-lg   │ 12px  │ Large cards, modals                           │   │
│  │  radius-xl   │ 16px  │ Dialogs, panels                               │   │
│  │  radius-full │ 9999px│ Pills, avatars                                │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 5.2 Screen Specifications

### 5.2.1 Login Screen

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           LOGIN SCREEN                                       │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                                                                     │   │
│  │                    [EduERP Logo - 120x120px]                        │   │
│  │                                                                     │   │
│  │                    Welkom bij EduERP                                │   │
│  │                    Leer ondernemen door te doen                     │   │
│  │                                                                     │   │
│  │  ┌─────────────────────────────────────────────────────────────┐   │   │
│  │  │  [Google Icon]  Inloggen met Google                         │   │   │
│  │  └─────────────────────────────────────────────────────────────┘   │   │
│  │                                                                     │   │
│  │  ┌─────────────────────────────────────────────────────────────┐   │   │
│  │  │  [Microsoft Icon]  Inloggen met Microsoft                   │   │   │
│  │  └─────────────────────────────────────────────────────────────┘   │   │
│  │                                                                     │   │
│  │  ────────────────────  of  ────────────────────                    │   │
│  │                                                                     │   │
│  │  ┌─────────────────────────────────────────────────────────────┐   │   │
│  │  │  [School Icon]  Inloggen met schoolaccount                  │   │   │
│  │  └─────────────────────────────────────────────────────────────┘   │   │
│  │                                                                     │   │
│  │  Door in te loggen ga je akkoord met de                           │   │
│  │  [gebruiksvoorwaarden] en [privacyverklaring]                     │   │
│  │                                                                     │   │
│  │                    [Taal: Nederlands ▼]                             │   │
│  │                                                                     │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Layout:                                                                    │
│  - Centered card, max-width: 420px                                          │
│  - Vertical padding: space-12 (48px)                                        │
│  - Button height: 48px, full width                                          │
│  - Gap between buttons: space-4 (16px)                                      │
│                                                                             │
│  Interactions:                                                              │
│  - OAuth buttons open system browser for authentication                     │
│  - Loading state during OAuth flow                                          │
│  - Error message display for invalid domain                                 │
│                                                                             │
│  Accessibility:                                                             │
│  - All buttons keyboard accessible (Tab order)                              │
│  - Focus indicators visible                                                 │
│  - Screen reader labels for all interactive elements                        │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 5.2.2 Student Dashboard

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        STUDENT DASHBOARD                                     │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐ │
│  │ [≡]  EduERP    [🔍]    [🔔3]  [👤 Jan ▼]                              │ │
│  ├──────────┬────────────────────────────────────────────────────────────┤ │
│  │          │  Goedemorgen, Jan! 👋                                       │ │
│  │          │                                                              │ │
│  │  NAVIGATIE│  ┌─────────────────────────────────────────────────────┐  │ │
│  │          │  │  MIJN BEDRIJVEN                    [+ Nieuw bedrijf] │  │ │
│  │  🏠 Dashboard│  ├─────────────────────────────────────────────────────┤  │ │
│  │  📊 Simulatie│  │  ┌─────────────┐  ┌─────────────┐  ┌────────────┐ │  │ │
│  │  💬 Berichten│  │  │ ModeMax BV  │  │ TechStart   │  │ [+]        │ │  │ │
│  │  👥 Vrienden │  │  │ 👕 Retail   │  │ 💻 Tech     │  │ Nieuw      │ │  │ │
│  │  👤 Profiel  │  │  │             │  │             │  │ bedrijf    │ │  │ │
│  │          │  │  │ 💰 €45.230  │  │ 💰 €12.500  │  │            │ │  │ │
│  │          │  │  │ 📈 +12%     │  │ 📉 -5%      │  │            │ │  │ │
│  │          │  │  └─────────────┘  └─────────────┘  └────────────┘ │  │ │
│  │          │  └─────────────────────────────────────────────────────┘  │ │
│  │          │                                                              │ │
│  │          │  ┌─────────────────────────────────────────────────────┐  │ │
│  │          │  │  MIJN TEAM: Team Alpha                              │  │ │
│  │          │  │  ┌─────────────────────────────────────────────────┐│  │ │
│  │          │  │  │ [👤 Jan] [👤 Piet] [👤 Marie] [👤 Tom]         ││  │ │
│  │          │  │  │  CEO      CFO      Sales    Marketing           ││  │ │
│  │          │  │  │  🟢       🟢       ⚪         🟢                  ││  │ │
│  │          │  │  └─────────────────────────────────────────────────┘│  │ │
│  │          │  │  [💬 Teamchat openen]                               │  │ │
│  │          │  └─────────────────────────────────────────────────────┘  │ │
│  │          │                                                              │ │
│  │          │  ┌─────────────────────────────────────────────────────┐  │ │
│  │          │  │  RECENTE ACTIVITEIT                                 │  │ │
│  │          │  │  • Piet heeft de prijzen aangepast (10 min geleden) │  │ │
│  │          │  │  • Nieuwe bestelling #123 ontvangen (30 min geleden)│  │ │
│  │          │  │  • Maandrapport beschikbaar (2 uur geleden)         │  │ │
│  │          │  └─────────────────────────────────────────────────────┘  │ │
│  │          │                                                              │ │
│  │          │  ┌─────────────────────────────────────────────────────┐  │ │
│  │          │  │  OPDRACHTEN                                         │  │ │
│  │          │  │  📋 Opdracht 3: Marketingcampagne (deadline: 3 dagen)│  │ │
│  │          │  │     [Bekijk details]                                │  │ │
│  │          │  └─────────────────────────────────────────────────────┘  │ │
│  │          │                                                              │ │
│  └──────────┴────────────────────────────────────────────────────────────┘ │
│                                                                             │
│  Layout:                                                                    │
│  - Sidebar: 240px fixed width, collapsible on small screens                 │
│  - Top bar: 64px height, sticky                                             │
│  - Content area: responsive grid                                            │
│  - Cards: 280px min-width, auto-fit grid                                    │
│                                                                             │
│  Responsive Breakpoints:                                                    │
│  - < 768px: Sidebar becomes hamburger menu                                  │
│  - < 1024px: Company cards stack to 2 columns                               │
│  - >= 1024px: Company cards in 3 columns                                    │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 5.2.3 Finance Module View

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        FINANCE MODULE                                        │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐ │
│  │ [≡]  EduERP  >  ModeMax BV  >  💰 Financiën    [🔔]  [👤 Jan ▼]      │ │
│  ├────────────────────────────────────────────────────────────────────────┤│
│  │                                                                        ││
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ││
│  │  │  OMZET      │  │  WINST      │  │  KAS        │  │  MARGE      │  ││
│  │  │             │  │             │  │             │  │             │  ││
│  │  │  €125.000   │  │  €25.000    │  │  €87.500    │  │  20,0%      │  ││
│  │  │  📈 +15%    │  │  📈 +8%     │  │  📉 -2%     │  │  ➡️ 0%      │  ││
│  │  └─────────────┘  └─────────────┘  └─────────────┘  └─────────────┘  ││
│  │                                                                        ││
│  │  ┌─────────────────────────────────┐  ┌─────────────────────────────┐  ││
│  │  │  OMZETONTWIKKELING              │  │  KOSTENSTRUCTUUR            │  ││
│  │  │  [Line Chart: 6 months]         │  │  [Pie Chart]                │  ││
│  │  │                                 │  │  • Inkoop: 40%              │  ││
│  │  │  €140K ┤    ╭─╮                 │  │  • Lonen: 35%               │  ││
│  │  │  €120K ┤╭──╯  ╰──╮              │  │  • Overhead: 25%            │  ││
│  │  │  €100K ┤╯         ╰────         │  │                             │  ││
│  │  │        └────┬────┬────┬────┬    │  │                             │  ││
│  │  │           jan  feb  mrt  apr    │  │                             │  ││
│  │  │  [📊 Details]  [📥 Exporteren]  │  │  [📊 Details]               │  ││
│  │  └─────────────────────────────────┘  └─────────────────────────────┘  ││
│  │                                                                        ││
│  │  ┌─────────────────────────────────────────────────────────────────┐   ││
│  │  │  SNELLE ACTIES                                                  │   ││
│  │  │  [📋 Grootboek]  [📊 Balans]  [📈 Resultatenrekening]           │   ││
│  │  │  [💰 Kasstroom]  [📝 Boeking maken]  [📅 Afsluiten periode]     │   ││
│  │  └─────────────────────────────────────────────────────────────────┘   ││
│  │                                                                        ││
│  │  ┌─────────────────────────────────────────────────────────────────┐   ││
│  │  │  RECENTE BOEKINGEN                          [Alles bekijken →]  │   ││
│  │  │  ┌──────────┬─────────────┬──────────┬──────────┬────────────┐  │   ││
│  │  │  │ Datum    │ Omschrijving│ Debet    │ Credit   │ Bedrag     │  │   ││
│  │  │  ├──────────┼─────────────┼──────────┼──────────┼────────────┤  │   ││
│  │  │  │ 26/03    │ Verkoop #45 │ 1200     │ 4000     │ € 500,00   │  │   ││
│  │  │  │ 25/03    │ Inkoop #23  │ 6000     │ 1200     │ € 250,00   │  │   ││
│  │  │  │ 25/03    │ Loon maart  │ 6100     │ 1200     │ € 4.000,00 │  │   ││
│  │  │  └──────────┴─────────────┴──────────┴──────────┴────────────┘  │   ││
│  │  └─────────────────────────────────────────────────────────────────┘   ││
│  │                                                                        ││
│  └────────────────────────────────────────────────────────────────────────┘│
│                                                                             │
│  Number Formatting (Dutch):                                                 │
│  - Decimal separator: comma (,)                                             │
│  - Thousand separator: period (.)                                           │
│  - Currency: € 1.234,56                                                     │
│  - Percentage: 20,0% (always one decimal)                                   │
│                                                                             │
│  Role-based View Restrictions:                                              │
│  - CEO: Full access to all financial data                                   │
│  - CFO: Full access + can create journal entries                            │
│  - Sales Manager: View revenue, limited cost visibility                     │
│  - Other roles: View-only dashboard KPIs                                    │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 5.2.4 Team Chat Interface

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         TEAM CHAT                                            │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐ │
│  │ 💬 Team Alpha - Chat                                    [─] [□] [×]   │ │
│  ├───────────────────────────────────────────────────────────────────────┤ │
│  │                                                                       │ │
│  │  ┌─────────────────────────────────────────────────────────────────┐  │ │
│  │  │  🔥 Streak: 5 dagen                                             │  │ │
│  │  └─────────────────────────────────────────────────────────────────┘  │ │
│  │                                                                       │ │
│  │  ─── Gisteren ─────────────────────────────────────────────────────   │ │
│  │                                                                       │ │
│  │     ┌─────────────────────────────┐                                   │ │
│  │     │ 👤 Piet (CFO)               │                                   │ │
│  │     │ Heb de cashflow geüpdatet   │                                   │ │
│  │     │ voor volgende maand.        │                                   │ │
│  │     │                    14:32 ✓✓ │                                   │ │
│  │     └─────────────────────────────┘                                   │ │
│  │                                                                       │ │
│  │     ┌─────────────────────────────┐                                   │ │
│  │     │ 👤 Jij (CEO)                │                                   │ │
│  │     │ Top, thanks! Kun je ook     │                                   │ │
│  │     │ de nieuwe prijzen checken?  │                                   │ │
│  │     │                    14:35 ✓✓ │                                   │ │
│  │     └─────────────────────────────┘                                   │ │
│  │                                                                       │ │
│  │  ─── Vandaag ──────────────────────────────────────────────────────   │ │
│  │                                                                       │ │
│  │     ┌─────────────────────────────┐                                   │ │
│  │     │ 👤 Marie (Sales)            │                                   │ │
│  │     │ ⚠️ Nieuwe opdracht van      │                                   │ │
│  │     │ mevrouw Jansen binnen!      │                                   │ │
│  │     │                    09:15 ✓  │                                   │ │
│  │     └─────────────────────────────┘                                   │ │
│  │                                                                       │ │
│  │     ┌─────────────────────────────┐                                   │ │
│  │     │ 👤 Piet (CFO)               │                                   │ │
│  │     │ 💬 Piet is een bericht aan  │                                   │ │
│  │     │ het typen...                │                                   │ │
│  │     └─────────────────────────────┘                                   │ │
│  │                                                                       │ │
│  ├───────────────────────────────────────────────────────────────────────┤ │
│  │  [😊]  [📎]  [Schrijf een bericht...                    ]  [➤]       │ │
│  └───────────────────────────────────────────────────────────────────────┘ │
│                                                                             │
│  Chat Features:                                                             │
│  - Real-time message delivery via WebSocket                                 │
│  - Typing indicators (3-second timeout)                                     │
│  - Read receipts (✓ = sent, ✓✓ = read)                                     │
│  - Message history (load 50 at a time, infinite scroll)                     │
│  - Emoji picker (system emoji, not custom set)                              │
│  - File attachments (max 5MB, images only)                                  │
│                                                                             │
│  Streak Display:                                                            │
│  - Flame icon (SVG, not emoji) with day counter                             │
│  - Hidden if either user disabled streaks                                   │
│  - Reset notification when streak about to break (20 hours no message)      │
│                                                                             │
│  Teacher Monitoring:                                                        │
│  - Teacher can view all team chats (read-only)                              │
│  - Teacher messages appear with special indicator                           │
│  - Teacher can send messages to any team                                    │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 5.2.5 Teacher Dashboard

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        TEACHER DASHBOARD                                     │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐ │
│  │ [≡]  EduERP    [🔍]    [🔔5]  [👤 Mevr. Jansen ▼]                     │ │
│  ├──────────┬────────────────────────────────────────────────────────────┤ │
│  │          │  5de jaar Economie A - Overzicht                            │ │
│  │          │                                                              │ │
│  │  NAVIGATIE│  ┌─────────────────────────────────────────────────────┐  │ │
│  │          │  │  KLASOVERZICHT                                      │  │ │
│  │  🏠 Dashboard│  ┌─────────────────────────────────────────────────┐ │  │ │
│  │  📚 Klassen│  │  Totaal: 24 leerlingen  │  Online: 18  │  Teams: 6 │ │  │ │
│  │  👥 Teams  │  └─────────────────────────────────────────────────┘ │  │ │
│  │  📋 Opdrachten│  [👁️ Live weergave]  [⏸️ Pauzeer simulatie]      │  │ │
│  │  📊 Rapporten│  └─────────────────────────────────────────────────────┘  │ │
│  │  ⚙️ Instellingen│                                                        │ │
│  │          │  ┌─────────────────────────────────────────────────────┐  │ │
│  │          │  │  LEERLINGEN                                         │  │ │
│  │          │  │  ┌──────────┬────────────┬──────────┬────────────┐  │  │ │
│  │          │  │  │ Naam     │ Status     │ Bedrijf  │ Tijd vandaag│  │  │ │
│  │          │  │  ├──────────┼────────────┼──────────┼────────────┤  │  │ │
│  │          │  │  │ De Smet  │ 🟢 Online  │ ModeMax  │ 45 min     │  │  │ │
│  │          │  │  │ Peeters  │ 🟢 Online  │ ModeMax  │ 42 min     │  │  │ │
│  │          │  │  │ Jansen   │ ⚪ Offline │ TechStart│ 0 min      │  │  │ │
│  │          │  │  │ ...      │ ...        │ ...      │ ...        │  │  │ │
│  │          │  │  └──────────┴────────────┴──────────┴────────────┘  │  │ │
│  │          │  │  [📊 Exporteer naar Excel]                          │  │ │
│  │          │  └─────────────────────────────────────────────────────┘  │ │
│  │          │                                                              │ │
│  │          │  ┌─────────────────────────────────────────────────────┐  │ │
│  │          │  │  TEAM OVERZICHT                                     │  │ │
│  │          │  │  ┌─────────────┬──────────┬────────────┬───────────┐ │  │ │
│  │          │  │  │ Team        │ Leden    │ Bedrijf    │ Omzet     │ │  │ │
│  │          │  │  ├─────────────┼──────────┼────────────┼───────────┤ │  │ │
│  │          │  │  │ Team Alpha  │ 4/4      │ ModeMax    │ €125.000  │ │  │ │
│  │          │  │  │ Team Beta   │ 4/4      │ TechStart  │ €45.000   │ │  │ │
│  │          │  │  │ ...         │ ...      │ ...        │ ...       │ │  │ │
│  │          │  │  └─────────────┴──────────┴────────────┴───────────┘ │  │ │
│  │          │  │  [+ Nieuw team]  [🔄 Wijzig rollen]                  │  │ │
│  │          │  └─────────────────────────────────────────────────────┘  │ │
│  │          │                                                              │ │
│  │          │  ┌─────────────────────────────────────────────────────┐  │ │
│  │          │  │  OPDRACHTEN                                         │  │ │
│  │          │  │  • Opdracht 3: Marketingcampagne                    │  │ │
│  │          │  │    Deadline: 3 dagen │ Ingediend: 18/24              │  │ │
│  │          │  │    [Bekijk inzendingen]  [Wijzig deadline]          │  │ │
│  │          │  └─────────────────────────────────────────────────────┘  │ │
│  │          │                                                              │ │
│  │          │  ┌─────────────────────────────────────────────────────┐  │ │
│  │          │  │  ACTIVITEITENKAART (Heatmap)                        │  │ │
│  │          │  │  [Grid: Uren x Dagen met kleurintensiteit]          │  │ │
│  │          │  │  Ma Di Wo Do Vr                                     │  │ │
│  │          │  │  8h  ██░░██░░██                                       │  │ │
│  │          │  │  9h  ████████░░                                       │  │ │
│  │          │  │  10h ██████████                                       │  │ │
│  │          │  │  ...                                                  │  │ │
│  │          │  └─────────────────────────────────────────────────────┘  │ │
│  │          │                                                              │ │
│  └──────────┴────────────────────────────────────────────────────────────┘ │
│                                                                             │
│  Teacher Actions:                                                           │
│  - Click student name → View student profile                                │
│  - Click company name → Spectator view (read-only)                          │
│  - Right-click student → Context menu (reset password, move class, etc.)    │
│  - "Live weergave" → Real-time activity feed                                │
│  - "Pauzeer simulatie" → Pause all teams' simulations                       │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 5.2.6 Settings Screen

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                          SETTINGS                                            │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐ │
│  │ [≡]  EduERP  >  👤 Profiel  >  ⚙️ Instellingen    [🔔]  [👤 Jan ▼]    │ │
│  ├────────────────────────────────────────────────────────────────────────┤│
│  │                                                                        ││
│  │  ┌─────────────────┐  ┌─────────────────────────────────────────────┐ ││
│  │  │                 │  │  WEERGAVE                                   │ ││
│  │  │  [Avatar]       │  │                                             │ ││
│  │  │                 │  │  Thema                                      │ ││
│  │  │  Jan De Smet    │  │  (•) Licht  ( ) Donker  ( ) Systeem        │ ││
│  │  │  @jan.desmet    │  │                                             │ ││
│  │  │                 │  │  Accentkleur                                │ ││
│  │  │  [Wijzig foto]  │  │  [🔵] [🟢] [🟡] [🟠] [🔴] [🟣]            │ ││
│  │  │                 │  │                                             │ ││
│  │  │  ─────────────  │  │  Lettergrootte                              │ ││
│  │  │                 │  │  [Klein] [Normaal] [Groot]                   │ ││
│  │  │  📊 Statistieken│  │                                             │ ││
│  │  │  🌐 Taal        │  │  Animaties                                  │ ││
│  │  │  🔒 Privacy     │  │  (•) Volledig  ( ) Verminderd  ( ) Geen     │ ││
│  │  │  🔔 Meldingen   │  │                                             │ ││
│  │  │  ⚙️ Algemeen    │  │  Layoutdichtheid                            │ ││
│  │  │                 │  │  ( ) Compact  (•) Comfortabel                │ ││
│  │  └─────────────────┘  │                                             │ ││
│  │                       │  [✓] Energiebesparende modus                 │ ││
│  │  ───────────────────  │                                             │ ││
│  │                       │  ─────────────────────────────────────────   │ ││
│  │  ℹ️ EduERP v1.0.0    │  │  TAAL EN REGIO                              │ ││
│  │                       │                                             │ ││
│  │                       │  Taal: [Nederlands ▼]                        │ ││
│  │                       │  Getalnotatie: 1.234,56 (Belgisch)           │ ││
│  │                       │  Datumnotatie: 26/03/2026                    │ ││
│  │                       │  Valuta: Euro (€)                            │ ││
│  │                       │                                             │ ││
│  │                       │  ─────────────────────────────────────────   │ ││
│  │                       │  [💾 Wijzigingen opslaan]                    │ ││
│  │                       │                                             │ ││
│  │                       └─────────────────────────────────────────────┘ ││
│  │                                                                        ││
│  └────────────────────────────────────────────────────────────────────────┘│
│                                                                             │
│  Settings Categories:                                                       │
│  1. Algemeen (Display) - Theme, colors, font, animations                    │
│  2. Taal - Language, number/date/currency format                            │
│  3. Meldingen - Notification preferences per type                           │
│  4. Privacy - Profile visibility, friend requests                           │
│  5. Account - Password (OAuth relink), delete account                       │
│                                                                             │
│  Admin Override Indicators:                                                 │
│  - Locked settings show "🔒 Beheerd door school"                            │
│  - Disabled options are grayed out with tooltip explanation                 │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 5.3 Component Library

### 5.3.1 Core Components

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      COMPONENT SPECIFICATIONS                                │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  EduButton (Primary)                                                        │
│  ═════════════════════                                                      │
│  Props:                                                                     │
│    - text: string (required)                                                │
│    - onClick: function (required)                                           │
│    - variant: 'primary' | 'secondary' | 'danger' | 'ghost'                  │
│    - size: 'small' | 'medium' | 'large' (default: medium)                   │
│    - disabled: boolean (default: false)                                     │
│    - loading: boolean (default: false)                                      │
│    - icon: string (optional, icon name)                                     │
│                                                                             │
│  Style:                                                                     │
│    - Height: 36px (small), 44px (medium), 52px (large)                      │
│    - Padding: 0 16px (small), 0 24px (medium), 0 32px (large)               │
│    - Border radius: radius-md (8px)                                         │
│    - Font: Label Large (14px, weight 500)                                   │
│    - Primary: bg-primary, text-on-primary                                   │
│                                                                             │
│  States:                                                                    │
│    - Default: bg #1976D2                                                    │
│    - Hover: bg #1565C0 (darken 10%)                                         │
│    - Active: bg #0D47A1 (darken 20%)                                        │
│    - Disabled: opacity 0.5, cursor not-allowed                              │
│    - Loading: Spinner replaces icon/text                                    │
│                                                                             │
│  Accessibility:                                                             │
│    - Keyboard: Enter/Space to activate                                      │
│    - Focus: 2px outline offset 2px                                          │
│    - ARIA: role="button", aria-label if icon-only                           │
│                                                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  EduTextField                                                               │
│  ════════════════                                                           │
│  Props:                                                                     │
│    - label: string (required)                                               │
│    - value: string                                                          │
│    - placeholder: string                                                    │
│    - type: 'text' | 'password' | 'email' | 'number' | 'textarea'            │
│    - error: string (error message)                                          │
│    - helperText: string (optional hint)                                     │
│    - disabled: boolean                                                      │
│    - required: boolean                                                      │
│    - maxLength: number                                                      │
│                                                                             │
│  Style:                                                                     │
│    - Height: 56px (standard), auto (textarea)                               │
│    - Padding: 16px 12px                                                     │
│    - Border: 1px solid divider                                              │
│    - Border radius: radius-md (8px)                                         │
│    - Label: floats above when focused or has value                          │
│                                                                             │
│  States:                                                                    │
│    - Default: border #E0E0E0                                                │
│    - Focus: border #1976D2, label color primary                             │
│    - Error: border #F44336, label color error                               │
│    - Disabled: bg surface-variant, opacity 0.6                              │
│                                                                             │
│  Validation:                                                                │
│    - Real-time validation on blur                                           │
│    - Error message displayed below field                                    │
│    - Red border and icon when invalid                                       │
│                                                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  EduCard                                                                    │
│  ═════════                                                                  │
│  Props:                                                                     │
│    - title: string (optional)                                               │
│    - subtitle: string (optional)                                            │
│    - children: content                                                      │
│    - actions: array of buttons (optional)                                   │
│    - elevation: 0 | 1 | 2 | 3 (default: 1)                                  │
│    - clickable: boolean (default: false)                                    │
│    - onClick: function (if clickable)                                       │
│                                                                             │
│  Style:                                                                     │
│    - Background: surface color                                              │
│    - Border radius: radius-lg (12px)                                        │
│    - Padding: space-6 (24px)                                                │
│    - Shadow: increases with elevation                                       │
│    - Elevation 0: no shadow, 1px border                                     │
│    - Elevation 1: 0 2px 4px rgba(0,0,0,0.1)                                 │
│    - Elevation 2: 0 4px 8px rgba(0,0,0,0.12)                                │
│                                                                             │
│  Company Card (Specialized):                                                │
│  ┌─────────────────────────┐                                                │
│  │ [Logo]  Bedrijfsnaam    │                                                │
│  │         🏭 Industry      │                                                │
│  │                         │                                                │
│  │  💰 €45.230    📈 +12%  │                                                │
│  │  [Openen]  [Instellingen]│                                               │
│  └─────────────────────────┘                                                │
│                                                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  EduTable                                                                   │
│  ══════════                                                                 │
│  Props:                                                                     │
│    - columns: array of column definitions                                   │
│    - rows: array of data objects                                            │
│    - sortable: boolean (default: true)                                      │
│    - pagination: { page: number, pageSize: number, total: number }          │
│    - onSort: function(column, direction)                                    │
│    - onRowClick: function(row)                                              │
│    - emptyMessage: string                                                   │
│                                                                             │
│  Column Definition:                                                         │
│    - key: string (data property)                                            │
│    - title: string (header text)                                            │
│    - width: number | string (optional)                                      │
│    - align: 'left' | 'center' | 'right'                                     │
│    - sortable: boolean                                                      │
│    - formatter: function(value) => string                                   │
│                                                                             │
│  Style:                                                                     │
│    - Header: bg surface-variant, font-weight 500                            │
│    - Rows: alternating bg (zebra striping optional)                         │
│    - Hover: bg surface-variant at 50% opacity                               │
│    - Border: 1px solid divider between rows                                 │
│    - Cell padding: 12px 16px                                                │
│                                                                             │
│  Number Formatting in Tables:                                               │
│    - Currency: right-aligned, € symbol, Dutch format                        │
│    - Percentages: right-aligned, % symbol                                   │
│    - Dates: left-aligned, DD/MM/YYYY format                                 │
│                                                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  EduChart (Line Chart)                                                      │
│  ═════════════════════                                                      │
│  Props:                                                                     │
│    - type: 'line' | 'bar' | 'pie' | 'area'                                  │
│    - data: array of data points                                             │
│    - xKey: string (x-axis property)                                         │
│    - yKey: string (y-axis property)                                         │
│    - color: string (line/bar color)                                         │
│    - height: number (default: 300)                                          │
│    - showGrid: boolean (default: true)                                      │
│    - showTooltip: boolean (default: true)                                   │
│                                                                             │
│  Implementation:                                                            │
│    - Use Qt Charts module (QML)                                             │
│    - Hardware-accelerated rendering                                         │
│    - Animate on data change (configurable)                                  │
│    - Tooltip on hover with formatted values                                 │
│                                                                             │
│  Performance:                                                               │
│    - Limit to 1000 data points (sampling for more)                          │
│    - Disable animations in energy-saving mode                               │
│    - Use OpenGL rendering on supported hardware                             │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 5.4 Animation Specifications

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      ANIMATION SYSTEM                                        │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Animation Speed Multiplier (user-configurable):                            │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Setting    │ Multiplier │ Description                             │   │
│  ├─────────────────────────────────────────────────────────────────────┤   │
│  │  Volledig   │ 1.0x       │ Full 60fps animations (default)         │   │
│  │  Verminderd │ 0.5x       │ Faster, simpler transitions             │   │
│  │  Geen       │ 0x         │ Instant transitions                     │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Standard Animations:                                                       │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Animation         │ Duration │ Easing           │ Use Case         │   │
│  ├─────────────────────────────────────────────────────────────────────┤   │
│  │  Page transition   │ 300ms    │ ease-out         │ View navigation  │   │
│  │  Modal open        │ 200ms    │ ease-out-back    │ Dialogs appear   │   │
│  │  Modal close       │ 150ms    │ ease-in          │ Dialogs dismiss  │   │
│  │  Button press      │ 100ms    │ ease-out         │ Click feedback   │   │
│  │  Card hover        │ 150ms    │ ease-out         │ Elevation change │   │
│  │  Loading spinner   │ 1000ms   │ linear (loop)    │ Loading states   │   │
│  │  Toast notification│ 300ms    │ ease-out         │ Notification in  │   │
│  │  Stagger list      │ 50ms     │ ease-out         │ List items appear│   │
│  │  Chart data update │ 500ms    │ ease-in-out      │ Data transitions │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Spring Physics (for natural motion):                                       │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Property    │ Value    │ Description                               │   │
│  ├─────────────────────────────────────────────────────────────────────┤   │
│  │  Mass        │ 1.0      │ Heaviness of the object                   │   │
│  │  Stiffness   │ 100      │ Spring tension (higher = snappier)        │   │
│  │  Damping     │ 10       │ Friction (higher = less oscillation)      │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Reduced Motion Support:                                                    │
│  - Respect Windows "Show animations" setting                                │
│  - Disable parallax, spring physics                                         │
│  - Use simple opacity/translate transitions                                 │
│  - Instant transitions for "No animations" setting                          │
│                                                                             │
│  Energy Saving Mode:                                                        │
│  - Reduce animation duration by 50%                                         │
│  - Disable non-essential animations                                         │
│  - Reduce UI refresh rate to 30fps                                          │
│  - Pause background animations                                              │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 5.5 Responsive Design

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      RESPONSIVE BREAKPOINTS                                  │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Breakpoint Definitions:                                                    │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Name      │ Width      │ Target Device                             │   │
│  ├─────────────────────────────────────────────────────────────────────┤   │
│  │  xs        │ < 640px    │ Small tablets, large phones (rare)        │   │
│  │  sm        │ 640-1024px │ Tablets, small laptops                    │   │
│  │  md        │ 1024-1366px│ Standard laptops (1366x768 common)        │   │
│  │  lg        │ 1366-1600px│ Large laptops, small desktops             │   │
│  │  xl        │ > 1600px   │ Large desktops                            │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Minimum Supported Resolution: 1366x768                                     │
│  - All features accessible at this resolution                               │
│  - No horizontal scrolling on main content                                  │
│  - Sidebar collapses to hamburger menu at < 1024px                          │
│                                                                             │
│  Layout Adaptations:                                                        │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Element           │ xs/sm    │ md       │ lg/xl                    │   │
│  ├─────────────────────────────────────────────────────────────────────┤   │
│  │  Sidebar           │ Hidden   │ 200px    │ 240px                    │   │
│  │  Content padding   │ 16px     │ 24px     │ 32px                     │   │
│  │  Card grid columns │ 1        │ 2        │ 3-4                      │   │
│  │  Font scale        │ 0.875x   │ 1.0x     │ 1.0x                     │   │
│  │  Table density     │ Compact  │ Normal   │ Normal                   │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Touch Support (for future tablet support):                                 │
│  - Minimum touch target: 44x44px                                            │
│  - Swipe gestures for navigation                                            │
│  - Pinch-to-zoom for charts (optional)                                      │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

# PART 6: SIMULATION ENGINE SPECIFICATION

## 6.1 Simulation Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      SIMULATION ENGINE ARCHITECTURE                          │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                    SIMULATION ENGINE CORE                            │   │
│  │                                                                      │   │
│  │  ┌───────────────┐    ┌───────────────┐    ┌───────────────┐       │   │
│  │  │  Simulation   │◄──►│  Event        │◄──►│  AI Agent     │       │   │
│  │  │  Clock        │    │  System       │    │  Manager      │       │   │
│  │  └───────┬───────┘    └───────┬───────┘    └───────┬───────┘       │   │
│  │          │                    │                    │                │   │
│  │  ┌───────▼────────────────────▼────────────────────▼───────┐       │   │
│  │  │                  STATE MANAGER                           │       │   │
│  │  │  (Company state, KPI calculations, module coordination)  │       │   │
│  │  └───────┬────────────────────┬────────────────────┬───────┘       │   │
│  │          │                    │                    │                │   │
│  │  ┌───────▼───────┐   ┌────────▼────────┐   ┌──────▼───────┐       │   │
│  │  │   Finance     │   │     Sales       │   │  Inventory   │       │   │
│  │  │   Module      │   │     Module      │   │   Module     │       │   │
│  │  └───────────────┘   └─────────────────┘   └──────────────┘       │   │
│  │  ┌───────────────┐   ┌─────────────────┐   ┌──────────────┐       │   │
│  │  │      HR       │   │   Marketing     │   │  Logistics   │       │   │
│  │  │   Module      │   │     Module      │   │   Module     │       │   │
│  │  └───────────────┘   └─────────────────┘   └──────────────┘       │   │
│  │                                                                      │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Time Scales:                                                               │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Mode        │ Real Time │ Simulated Time │ Use Case                │   │
│  ├─────────────────────────────────────────────────────────────────────┤   │
│  │  Real-time   │ 1 minute  │ 1 day          │ Live classroom sessions │   │
│  │  Accelerated │ 1 minute  │ 1-10 days      │ Homework, practice      │   │
│  │  Turn-based  │ Manual    │ 1 week/month   │ Teacher-controlled pace │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 6.2 Financial Calculations

### 6.2.1 Double-Entry Bookkeeping

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    DOUBLE-ENTRY BOOKKEEPING SYSTEM                           │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Account Types and Normal Balances:                                         │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Type        │ Normal Balance │ Debit Effect │ Credit Effect       │   │
│  ├─────────────────────────────────────────────────────────────────────┤   │
│  │  Assets      │ Debit (+)      │ Increase     │ Decrease            │   │
│  │  Liabilities │ Credit (+)     │ Decrease     │ Increase            │   │
│  │  Equity      │ Credit (+)     │ Decrease     │ Increase            │   │
│  │  Revenue     │ Credit (+)     │ Decrease     │ Increase            │   │
│  │  Expenses    │ Debit (+)      │ Increase     │ Decrease            │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Standard Chart of Accounts (Belgian SME):                                  │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Code  │ Name                    │ Type      │ Category            │   │
│  ├─────────────────────────────────────────────────────────────────────┤   │
│  │  1000  │ Kas                     │ Asset     │ Current Assets      │   │
│  │  1200  │ Bankrekening            │ Asset     │ Current Assets      │   │
│  │  1300  │ Klanten                 │ Asset     │ Receivables         │   │
│  │  1400  │ Voorraad                │ Asset     │ Inventory           │   │
│  │  2000  │ Leveranciers            │ Liability │ Payables            │   │
│  │  2100  │ Korte termijn schulden  │ Liability │ Short-term Debt     │   │
│  │  3000  │ Eigen vermogen          │ Equity    │ Equity              │   │
│  │  4000  │ Verkopen                │ Revenue   │ Sales               │   │
│  │  4400  │ BTW verschuldigd        │ Liability │ VAT Payable         │   │
│  │  6000  │ Aankopen                │ Expense   │ COGS                │   │
│  │  6100  │ Lonen                   │ Expense   │ Personnel           │   │
│  │  6200  │ Algemene kosten         │ Expense   │ Overhead            │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Journal Entry Algorithm:                                                   │
│  ```cpp                                                                     │
│  bool createJournalEntry(                                                   │
│      int companyId,                                                         │
│      int debitAccountId,                                                    │
│      int creditAccountId,                                                   │
│      Decimal amount,                                                        │
│      const String& description,                                             │
│      const Date& entryDate                                                  │
│  ) {                                                                        │
│      // Validate accounts belong to company                                 │
│      // Validate amount > 0                                                 │
│      // Create ledger entry                                                 │
│      // Update account balances                                             │
│      // Trigger KPI recalculation                                           │
│  }                                                                          │
│  ```                                                                        │
│                                                                             │
│  Belgian VAT Simulation (Simplified):                                       │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Rate    │ Applies To                    │ Calculation             │   │
│  ├─────────────────────────────────────────────────────────────────────┤   │
│  │  21%     │ Standard goods/services       │ price * 0.21            │   │
│  │  12%     │ Social housing, restaurants   │ price * 0.12            │   │
│  │  6%      │ Essential goods, books        │ price * 0.06            │   │
│  │  0%      │ Exports, intra-EU             │ 0                       │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  VAT Calculation Example:                                                   │
│  ```                                                                        │
│  Sale: €100 (excl. VAT)                                                     │
│  VAT (21%): €21                                                             │
│  Total: €121                                                                │
│                                                                             │
│  Journal Entry:                                                             │
│  Debit: Bank (1200)      €121                                               │
│  Credit: Sales (4000)    €100                                               │
│  Credit: VAT Payable     €21                                                │
│  ```                                                                        │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 6.2.2 Financial Statement Generation

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    FINANCIAL STATEMENT ALGORITHMS                            │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Income Statement (Winst- en verliesrekening):                              │
│  ```cpp                                                                     │
│  IncomeStatement generateIncomeStatement(                                   │
│      int companyId,                                                         │
│      const Date& startDate,                                                 │
│      const Date& endDate                                                    │
│  ) {                                                                        │
│      // Revenue                                                             │
│      revenue = sum(ledger entries to revenue accounts)                      │
│                                                                             │
│      // Cost of Goods Sold                                                  │
│      cogs = sum(ledger entries to COGS accounts)                            │
│                                                                             │
│      // Gross Profit                                                        │
│      grossProfit = revenue - cogs                                           │
│                                                                             │
│      // Operating Expenses                                                  │
│      operatingExpenses = sum(payroll, rent, utilities, etc.)                │
│                                                                             │
│      // Operating Income (EBIT)                                             │
│      operatingIncome = grossProfit - operatingExpenses                      │
│                                                                             │
│      // Net Profit                                                          │
│      netProfit = operatingIncome - interest - taxes                         │
│                                                                             │
│      // Profit Margin                                                       │
│      profitMargin = revenue > 0 ? netProfit / revenue : 0                   │
│  }                                                                          │
│  ```                                                                        │
│                                                                             │
│  Balance Sheet (Balans):                                                    │
│  ```cpp                                                                     │
│  BalanceSheet generateBalanceSheet(                                         │
│      int companyId,                                                         │
│      const Date& asOfDate                                                   │
│  ) {                                                                        │
│      // Assets                                                              │
│      currentAssets = cash + receivables + inventory                         │
│      fixedAssets = equipment + buildings - accumulatedDepreciation          │
│      totalAssets = currentAssets + fixedAssets                              │
│                                                                             │
│      // Liabilities                                                         │
│      currentLiabilities = payables + shortTermDebt + vatPayable             │
│      longTermLiabilities = loans + bonds                                    │
│      totalLiabilities = currentLiabilities + longTermLiabilities            │
│                                                                             │
│      // Equity                                                              │
│      retainedEarnings = sum(all historical net profits)                     │
│      totalEquity = shareCapital + retainedEarnings                          │
│                                                                             │
│      // Balance Check                                                       │
│      assert(totalAssets == totalLiabilities + totalEquity)                  │
│  }                                                                          │
│  ```                                                                        │
│                                                                             │
│  Depreciation (Linear Method):                                              │
│  ```                                                                        │
│  Annual Depreciation = (Purchase Price - Salvage Value) / Useful Life       │
│                                                                             │
│  Example: Computer €2.000, 5 jaar, restwaarde €200                          │
│  Annual Depreciation = (2000 - 200) / 5 = €360/jaar                         │
│  ```                                                                        │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 6.3 Sales & CRM Simulation

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    SALES PIPELINE SIMULATION                                 │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Pipeline Stage Probabilities (Base):                                       │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Stage        │ Base Conversion │ Duration (days) │ Factors        │   │
│  ├─────────────────────────────────────────────────────────────────────┤   │
│  │  Lead         │ 40% → Prospect  │ 7-14            │ Marketing      │   │
│  │  Prospect     │ 30% → Proposal  │ 14-30           │ Sales effort   │   │
│  │  Proposal     │ 50% → Negotiate │ 7-21            │ Pricing        │   │
│  │  Negotiation  │ 60% → Closed Won│ 7-14            │ Terms          │   │
│  │  Closed Won   │ -               │ -               │ -              │   │
│  │  Closed Lost  │ -               │ -               │ -              │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Conversion Probability Modifiers:                                          │
│  ```cpp                                                                     │
│  float calculateConversionProbability(                                      │
│      PipelineStage stage,                                                   │
│      const Customer& customer,                                              │
│      const Company& company                                                 │
│  ) {                                                                        │
│      float baseProb = getBaseProbability(stage);                            │
│                                                                             │
│      // Customer factors                                                    │
│      baseProb *= (1 + (customer.loyaltyScore - 50) / 200);                  │
│      baseProb *= (1 - customer.priceSensitivity / 200);                     │
│                                                                             │
│      // Company factors                                                     │
│      baseProb *= (1 + (company.brandAwareness - 50) / 200);                 │
│      baseProb *= (1 + (company.customerSatisfaction - 50) / 200);           │
│                                                                             │
│      // Pricing factor                                                      │
│      float marketPrice = getMarketPrice(product);                           │
│      float priceRatio = companyPrice / marketPrice;                         │
│      baseProb *= (2 - priceRatio);  // Higher price = lower prob            │
│                                                                             │
│      return clamp(baseProb, 0.05f, 0.95f);                                  │
│  }                                                                          │
│  ```                                                                        │
│                                                                             │
│  Customer Satisfaction Algorithm:                                           │
│  ```cpp                                                                     │
│  float calculateCustomerSatisfaction(                                       │
│      const Order& order,                                                    │
│      const Company& company                                                 │
│  ) {                                                                        │
│      float satisfaction = 0.7f;  // Base satisfaction                       │
│                                                                             │
│      // Delivery time factor                                                │
│      if (order.actualDelivery <= order.promisedDelivery) {                   │
│          satisfaction += 0.15f;                                             │
│      } else {                                                               │
│          float daysLate = order.actualDelivery - order.promisedDelivery;    │
│          satisfaction -= daysLate * 0.05f;                                  │
│      }                                                                      │
│                                                                             │
│      // Product quality factor                                              │
│      satisfaction += (company.productQuality - 0.5f) * 0.2f;                │
│                                                                             │
│      // Price factor                                                        │
│      satisfaction += (company.priceCompetitiveness - 0.5f) * 0.1f;          │
│                                                                             │
│      return clamp(satisfaction, 0.0f, 1.0f);                                │
│  }                                                                          │
│  ```                                                                        │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 6.4 Inventory Management

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    INVENTORY SIMULATION                                      │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Stock Level Calculation:                                                   │
│  ```cpp                                                                     │
│  struct StockLevel {                                                        │
│      int currentStock;                                                      │
│      int reservedStock;      // For pending orders                          │
│      int availableStock;     // currentStock - reservedStock                │
│      int reorderPoint;                                                      │
│      int maxStockLevel;                                                     │
│  };                                                                         │
│                                                                             │
│  StockStatus getStockStatus(const StockLevel& stock) {                      │
│      if (stock.availableStock <= 0) return StockStatus::OUT_OF_STOCK;       │
│      if (stock.availableStock <= stock.reorderPoint * 0.5)                  │
│          return StockStatus::CRITICAL;                                      │
│      if (stock.availableStock <= stock.reorderPoint)                        │
│          return StockStatus::LOW;                                           │
│      return StockStatus::OK;                                                │
│  }                                                                          │
│  ```                                                                        │
│                                                                             │
│  Demand Forecasting (Simple Moving Average):                                │
│  ```cpp                                                                     │
│  int forecastDemand(                                                        │
│      int productId,                                                         │
│      int periodsAhead,                                                      │
│      int lookbackPeriods = 3                                                │
│  ) {                                                                        │
│      auto sales = getHistoricalSales(productId, lookbackPeriods);           │
│      int totalSales = sum(sales);                                           │
│      float averageSales = totalSales / (float)lookbackPeriods;              │
│                                                                             │
│      // Apply seasonality factor                                            │
│      float seasonality = getSeasonalityFactor(productId, currentMonth);     │
│                                                                             │
│      // Apply trend factor                                                  │
│      float trend = calculateTrend(sales);                                   │
│                                                                             │
│      return round(averageSales * seasonality * trend * periodsAhead);       │
│  }                                                                          │
│  ```                                                                        │
│                                                                             │
│  Economic Order Quantity (EOQ) - Optional:                                  │
│  ```                                                                        │
│  EOQ = √(2 * D * S / H)                                                     │
│  Where:                                                                     │
│    D = Annual demand                                                        │
│    S = Ordering cost per order                                              │
│    H = Holding cost per unit per year                                       │
│  ```                                                                        │
│                                                                             │
│  Supplier Reliability:                                                      │
│  ```cpp                                                                     │
│  struct DeliveryResult {                                                    │
│      bool delivered;                                                        │
│      int actualLeadTimeDays;                                                │
│      int quantityDelivered;                                                 │
│      float qualityRating;                                                   │
│  };                                                                         │
│                                                                             │
│  DeliveryResult simulateDelivery(const Supplier& supplier) {                │
│      // Reliability check                                                   │
│      bool delivered = random() < supplier.reliabilityScore;                 │
│      if (!delivered) return {false, 0, 0, 0};                               │
│                                                                             │
│      // Lead time variation (normal distribution)                           │
│      float leadTimeVariation = randomNormal(0, supplier.leadTimeStdDev);    │
│      int actualLeadTime = supplier.averageLeadTimeDays + leadTimeVariation; │
│                                                                             │
│      // Quality variation                                                   │
│      float quality = clamp(randomNormal(supplier.avgQuality, 0.1), 0, 1);   │
│                                                                             │
│      return {true, actualLeadTime, orderQuantity, quality};                 │
│  }                                                                          │
│  ```                                                                        │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 6.5 HR & Payroll Simulation

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    HR & PAYROLL SIMULATION                                   │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Employee Productivity:                                                     │
│  ```cpp                                                                     │
│  float calculateProductivity(const Employee& employee) {                    │
│      float baseProductivity = employee.skillLevel / 5.0f;                   │
│                                                                             │
│      // Satisfaction impact                                                 │
│      float satisfactionFactor = employee.satisfactionScore / 100.0f;        │
│      baseProductivity *= (0.7f + 0.3f * satisfactionFactor);                │
│                                                                             │
│      // Training impact                                                     │
│      for (const auto& training : employee.completedTrainings) {             │
│          baseProductivity *= (1 + training.productivityGain / 100.0f);      │
│      }                                                                      │
│                                                                             │
│      // Random variation (±10%)                                             │
│      float variation = randomFloat(0.9f, 1.1f);                             │
│                                                                             │
│      return clamp(baseProductivity * variation, 0.1f, 2.0f);                │
│  }                                                                          │
│  ```                                                                        │
│                                                                             │
│  Simplified Belgian Payroll (Monthly):                                      │
│  ```cpp                                                                     │
│  PayrollResult calculatePayroll(const Employee& employee) {                 │
│      float grossSalary = employee.baseSalary;                               │
│                                                                             │
│      // Bonus (if applicable)                                               │
│      float bonus = grossSalary * employee.bonusPotential * performanceFactor│
│      grossSalary += bonus;                                                  │
│                                                                             │
│      // Employee Social Security (approx 13.07%)                            │
│      float employeeSS = grossSalary * 0.1307f;                              │
│                                                                             │
│      // Withholding Tax (simplified progressive)                            │
│      float taxableIncome = grossSalary - employeeSS;                        │
│      float withholdingTax = calculateWithholdingTax(taxableIncome);         │
│                                                                             │
│      // Net Salary                                                          │
│      float netSalary = taxableIncome - withholdingTax;                      │
│                                                                             │
│      // Employer Social Security (approx 25%)                               │
│      float employerSS = grossSalary * 0.25f;                                │
│                                                                             │
│      // Total Employer Cost                                                 │
│      float totalEmployerCost = grossSalary + employerSS;                    │
│                                                                             │
│      return {grossSalary, bonus, employeeSS, withholdingTax,                │
│               netSalary, employerSS, totalEmployerCost};                     │
│  }                                                                          │
│  ```                                                                        │
│                                                                             │
│  Employee Satisfaction Factors:                                             │
│  ```cpp                                                                     │
│  float updateSatisfaction(Employee& employee, const Company& company) {     │
│      float satisfaction = employee.satisfactionScore;                       │
│                                                                             │
│      // Salary competitiveness                                              │
│      float marketRate = getMarketSalary(employee.jobTitle);                 │
│      float salaryRatio = employee.baseSalary / marketRate;                  │
│      satisfaction += (salaryRatio - 1) * 20;                                │
│                                                                             │
│      // Training opportunities                                              │
│      if (company.trainingInvestment > 0) {                                  │
│          satisfaction += 5;                                                 │
│      }                                                                      │
│                                                                             │
│      // Workload (based on employee count vs demand)                        │
│      float workload = calculateWorkload(employee.department);               │
│      satisfaction -= (workload - 1) * 15;  // Penalty for overload          │
│                                                                             │
│      // Company performance                                                 │
│      satisfaction += (company.profitMargin - 0.1) * 50;                     │
│                                                                             │
│      // Natural drift toward mean (70)                                      │
│      satisfaction = lerp(satisfaction, 70, 0.05f);                          │
│                                                                             │
│      return clamp(satisfaction, 0, 100);                                    │
│  }                                                                          │
│  ```                                                                        │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 6.6 Marketing Simulation

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    MARKETING SIMULATION                                      │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Campaign Effectiveness by Channel:                                         │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Channel      │ Cost/Reach │ Conversion │ Best For                 │   │
│  ├─────────────────────────────────────────────────────────────────────┤   │
│  │  Social Media │ €0.05      │ 2-5%       │ Brand awareness, young   │   │
│  │  Online Ads   │ €0.10      │ 3-8%       │ Targeted campaigns       │   │
│  │  TV           │ €5.00      │ 0.5-2%     │ Mass market, credibility │   │
│  │  Print        │ €0.50      │ 1-3%       │ Local, older demographic │   │
│  │  Email        │ €0.01      │ 5-15%      │ Existing customers       │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Campaign ROI Calculation:                                                  │
│  ```cpp                                                                     │
│  CampaignResult simulateCampaign(                                           │
│      const Campaign& campaign,                                              │
│      const Company& company                                                 │
│  ) {                                                                        │
│      // Base reach calculation                                              │
│      float costPerImpression = getChannelCPI(campaign.channel);             │
│      int estimatedReach = campaign.budget / costPerImpression;              │
│                                                                             │
│      // Brand awareness multiplier                                          │
│      float awarenessMultiplier = 0.5f + company.brandAwareness / 100.0f;    │
│      estimatedReach *= awarenessMultiplier;                                 │
│                                                                             │
│      // Campaign quality factor (creative, targeting)                       │
│      float qualityFactor = randomFloat(0.7f, 1.3f);                         │
│      estimatedReach *= qualityFactor;                                       │
│                                                                             │
│      // Conversion rate                                                     │
│      float baseConversion = getBaseConversion(campaign.channel);            │
│      baseConversion *= (0.5f + company.brandAwareness / 100.0f);            │
│                                                                             │
│      int conversions = estimatedReach * baseConversion;                     │
│                                                                             │
│      // Revenue attribution                                                 │
│      float avgOrderValue = company.averageOrderValue;                       │
│      float revenueAttributed = conversions * avgOrderValue;                 │
│                                                                             │
│      // ROI                                                                 │
│      float roi = (revenueAttributed - campaign.budget) / campaign.budget;   │
│                                                                             │
│      // Update brand awareness                                              │
│      float awarenessGain = campaign.budget / 10000.0f * 0.01f;              │
│      company.brandAwareness = clamp(company.brandAwareness + awarenessGain, │
│                                      0.0f, 1.0f);                           │
│                                                                             │
│      return {estimatedReach, conversions, revenueAttributed, roi};          │
│  }                                                                          │
│  ```                                                                        │
│                                                                             │
│  Brand Awareness Decay:                                                     │
│  ```cpp                                                                     │
│  void decayBrandAwareness(Company& company, int days) {                     │
│      // Awareness decays 1% per week without marketing                      │
│      float decayRate = 0.01f / 7;                                           │
│      company.brandAwareness *= pow(1 - decayRate, days);                    │
│  }                                                                          │
│  ```                                                                        │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 6.7 AI Agent Decision System

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    AI AGENT DECISION SYSTEM                                  │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Decision Styles:                                                           │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Style    │ Description              │ Risk Level │ Profit Focus   │   │
│  ├─────────────────────────────────────────────────────────────────────┤   │
│  │  Optimal  │ Statistically best       │ Medium     │ High           │   │
│  │  Balanced │ Mix of good/average      │ Medium     │ Medium         │   │
│  │  Risky    │ High risk/high reward    │ High       │ Very High      │   │
│  │  Poor     │ Consistently bad         │ High       │ Low            │   │
│  │  Random   │ Unpredictable            │ Variable   │ Variable       │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  CEO Agent Decisions:                                                       │
│  ```cpp                                                                     │
│  Decision makeCEODecision(                                                  │
│      const Company& company,                                                │
│      DecisionStyle style,                                                   │
│      const MarketConditions& market                                         │
│  ) {                                                                        │
│      // Analyze current situation                                           │
│      float cashRatio = company.cashOnHand / company.monthlyExpenses;        │
│      float profitMargin = company.netProfit / company.revenue;              │
│                                                                             │
│      // Determine decision based on style and situation                     │
│      switch (style) {                                                       │
│          case DecisionStyle::OPTIMAL:                                       │
│              if (cashRatio < 3) {                                           │
│                  return Decision::REDUCE_COSTS;                             │
│              } else if (profitMargin > 0.2) {                               │
│                  return Decision::EXPAND_MARKET;                            │
│              } else {                                                       │
│                  return Decision::IMPROVE_EFFICIENCY;                       │
│              }                                                              │
│                                                                             │
│          case DecisionStyle::RISKY:                                         │
│              if (random() < 0.3) {                                          │
│                  return Decision::MAJOR_INVESTMENT;  // High risk           │
│              }                                                              │
│              return Decision::AGGRESSIVE_PRICING;                           │
│                                                                             │
│          case DecisionStyle::POOR:                                          │
│              // Make consistently suboptimal choices                        │
│              if (cashRatio < 2) {                                           │
│                  return Decision::EXPAND_INVENTORY;  // Wrong!              │
│              }                                                              │
│              return Decision::CUT_MARKETING;  // Hurts long-term            │
│                                                                             │
│          case DecisionStyle::RANDOM:                                        │
│              return randomDecision();                                       │
│      }                                                                      │
│  }                                                                          │
│  ```                                                                        │
│                                                                             │
│  CFO Agent - Pricing Decision:                                              │
│  ```cpp                                                                     │
│  PricingDecision makePricingDecision(                                       │
│      const Product& product,                                                │
│      const MarketData& market,                                              │
│      DecisionStyle style                                                    │
│  ) {                                                                        │
│      float costPrice = product.costPrice;                                   │
│      float marketPrice = market.averagePrice;                               │
│      float competitorLowest = market.lowestCompetitorPrice;                 │
│                                                                             │
│      float targetMargin;                                                    │
│      switch (style) {                                                       │
│          case DecisionStyle::OPTIMAL:                                       │
│              // Price slightly below market if competitive, else cost+       │
│              targetMargin = 0.4f;                                           │
│              break;                                                         │
│          case DecisionStyle::RISKY:                                         │
│              // Low margin for market share                                 │
│              targetMargin = 0.15f;                                          │
│              break;                                                         │
│          case DecisionStyle::POOR:                                          │
│              // Price too high                                              │
│              targetMargin = 0.8f;                                           │
│              break;                                                         │
│      }                                                                      │
│                                                                             │
│      float newPrice = costPrice * (1 + targetMargin);                       │
│                                                                             │
│      // Add some randomness for non-optimal styles                          │
│      if (style != DecisionStyle::OPTIMAL) {                                 │
│          newPrice *= randomFloat(0.9f, 1.1f);                               │
│      }                                                                      │
│                                                                             │
│      return {newPrice, "Marge aangepast naar " +                            │
│              toPercentage(targetMargin) + "%"};                             │
│  }                                                                          │
│  ```                                                                        │
│                                                                             │
│  Decision Logging (for student learning):                                   │
│  ```cpp                                                                     │
│  struct AIDecisionLog {                                                     │
│      string agentRole;          // "CEO", "CFO", etc.                       │
│      string decisionType;       // "pricing", "hiring", etc.                │
│      string decisionStyle;      // "optimal", "risky", etc.                 │
│      json context;              // Input parameters                         │
│      json decision;             // Output/decision made                     │
│      string reasoning;          // Human-readable explanation               │
│      DateTime timestamp;                                                    │
│  };                                                                         │
│  ```                                                                        │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 6.8 Event System

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    SIMULATION EVENT SYSTEM                                   │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Event Categories:                                                          │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Category      │ Examples                              │ Frequency │   │
│  ├─────────────────────────────────────────────────────────────────────┤   │
│  │  Economic      │ Recession, boom, inflation            │ Monthly   │   │
│  │  Operational   │ Machine breakdown, supply delay       │ Weekly    │   │
│  │  Competitive   │ New competitor, price war             │ Bi-weekly │   │
│  │  Regulatory    │ Tax change, new labor laws            │ Quarterly │   │
│  │  Random        │ Natural disaster, viral marketing     │ Rare      │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Event Generation Algorithm:                                                │
│  ```cpp                                                                     │
│  void generateRandomEvent(Company& company) {                               │
│      // Determine event probability based on difficulty                     │
│      float baseProbability = company.difficulty.eventFrequency;             │
│                                                                             │
│      // Check if event should occur                                         │
│      if (random() > baseProbability) return;                                │
│                                                                             │
│      // Select event category                                               │
│      EventCategory category = weightedRandomSelect({                        │
│          {EventCategory::ECONOMIC, 0.25},                                   │
│          {EventCategory::OPERATIONAL, 0.35},                                │
│          {EventCategory::COMPETITIVE, 0.25},                                │
│          {EventCategory::REGULATORY, 0.10},                                 │
│          {EventCategory::RANDOM, 0.05}                                      │
│      });                                                                    │
│                                                                             │
│      // Select specific event within category                               │
│      Event event = selectEventFromCategory(category, company);              │
│                                                                             │
│      // Apply event effects                                                 │
│      applyEventEffects(company, event);                                     │
│                                                                             │
│      // Notify students                                                     │
│      notifyEvent(company.teamId, event);                                    │
│  }                                                                          │
│  ```                                                                        │
│                                                                             │
│  Sample Events:                                                             │
│  ```json                                                                    │
│  {                                                                          │
│      "id": "econ_recession",                                                │
│      "category": "economic",                                                │
│      "title": "Economische recessie",                                       │
│      "description": "De economie krimpt. Consumenten besteden minder.",     │
│      "impact": {                                                            │
│          "demand_multiplier": 0.85,                                         │
│          "duration_days": 30,                                               │
│          "recovery_pattern": "gradual"                                      │
│      },                                                                     │
│      "choices": [                                                           │
│          {                                                                  │
│              "id": "cut_prices",                                            │
│              "text": "Verlaag prijzen om marktaandeel te behouden",         │
│              "effect": {"profit_margin": -0.05, "market_share": +0.02}      │
│          },                                                                 │
│          {                                                                  │
│              "id": "focus_quality",                                         │
│              "text": "Focus op kwaliteit, handhaaf prijzen",                │
│              "effect": {"customer_satisfaction": +0.10, "sales": -0.10}     │
│          }                                                                  │
│      ]                                                                      │
│  }                                                                          │
│  ```                                                                        │
│                                                                             │
│  Teacher Event Injection:                                                   │
│  ```cpp                                                                     │
│  void injectEvent(                                                          │
│      int classId,                                                           │
│      const string& eventId,                                                 │
│      const optional<int>& targetCompanyId = nullopt                         │
│  ) {                                                                        │
│      Event event = getEventById(eventId);                                   │
│                                                                             │
│      if (targetCompanyId) {                                                 │
│          // Apply to specific company                                       │
│          applyEvent(getCompany(targetCompanyId), event);                    │
│      } else {                                                               │
│          // Apply to all companies in class                                 │
│          for (auto& company : getCompaniesInClass(classId)) {               │
│              applyEvent(company, event);                                    │
│          }                                                                  │
│      }                                                                      │
│                                                                             │
│      // Mark as teacher-triggered                                           │
│      event.triggeredBy = "teacher";                                         │
│      event.teacherId = currentUser.id;                                      │
│  }                                                                          │
│  ```                                                                        │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 6.9 KPI Calculation Engine

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    KPI CALCULATION ENGINE                                    │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  KPI Update Frequency:                                                      │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  KPI Type         │ Update Frequency │ Storage                      │   │
│  ├─────────────────────────────────────────────────────────────────────┤   │
│  │  Financial        │ Daily            │ Daily snapshots              │   │
│  │  Operational      │ Real-time        │ Current values only          │   │
│  │  Satisfaction     │ Weekly           │ Weekly snapshots             │   │
│  │  Market Share     │ Monthly          │ Monthly snapshots            │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Comprehensive KPI Calculation:                                             │
│  ```cpp                                                                     │
│  KPIData calculateAllKPIs(const Company& company) {                         │
│      KPIData kpis;                                                          │
│                                                                             │
│      // Financial KPIs                                                      │
│      auto incomeStmt = generateIncomeStatement(company, currentPeriod);     │
│      kpis.revenue = incomeStmt.revenue;                                     │
│      kpis.netProfit = incomeStmt.netProfit;                                 │
│      kpis.profitMargin = incomeStmt.profitMargin;                           │
│                                                                             │
│      auto balanceSheet = generateBalanceSheet(company, today);              │
│      kpis.cashOnHand = balanceSheet.cash;                                   │
│      kpis.totalAssets = balanceSheet.totalAssets;                           │
│      kpis.debtToEquity = balanceSheet.totalLiabilities /                    │
│                          max(balanceSheet.totalEquity, 1);                  │
│                                                                             │
│      // Operational KPIs                                                    │
│      kpis.inventoryValue = calculateInventoryValue(company);                │
│      kpis.inventoryTurnover = calculateInventoryTurnover(company);          │
│                                                                             │
│      // Customer KPIs                                                       │
│      kpis.customerSatisfaction = calculateAvgCustomerSatisfaction(company); │
│      kpis.customerRetention = calculateRetentionRate(company);              │
│      kpis.totalCustomers = countActiveCustomers(company);                   │
│                                                                             │
│      // Employee KPIs                                                       │
│      kpis.employeeSatisfaction = calculateAvgEmployeeSatisfaction(company); │
│      kpis.employeeTurnover = calculateEmployeeTurnover(company);            │
│      kpis.totalEmployees = countActiveEmployees(company);                   │
│                                                                             │
│      // Market KPIs                                                         │
│      kpis.marketShare = calculateMarketShare(company);                      │
│      kpis.brandAwareness = company.brandAwareness;                          │
│                                                                             │
│      // Composite Score (for gamification)                                  │
│      kpis.overallScore = calculateOverallScore(kpis);                       │
│                                                                             │
│      return kpis;                                                           │
│  }                                                                          │
│  ```                                                                        │
│                                                                             │
│  Overall Score Calculation (Gamification):                                  │
│  ```cpp                                                                     │
│  float calculateOverallScore(const KPIData& kpis) {                         │
│      float score = 0;                                                       │
│                                                                             │
│      // Profitability (30%)                                                 │
│      score += kpis.profitMargin * 100 * 0.30f;                              │
│                                                                             │
│      // Growth (20%)                                                        │
│      float growthRate = kpis.revenueGrowth;                                 │
│      score += clamp(growthRate * 100, -20.0f, 50.0f) * 0.20f;               │
│                                                                             │
│      // Stability (20%)                                                     │
│      float stability = 1 - abs(kpis.cashFlowVariance);                      │
│      score += stability * 100 * 0.20f;                                      │
│                                                                             │
│      // Customer Satisfaction (15%)                                         │
│      score += kpis.customerSatisfaction * 100 * 0.15f;                      │
│                                                                             │
│      // Employee Satisfaction (15%)                                         │
│      score += kpis.employeeSatisfaction * 100 * 0.15f;                      │
│                                                                             │
│      return clamp(score, 0, 100);                                           │
│  }                                                                          │
│  ```                                                                        │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

# PART 7: TECHNOLOGY STACK REPORT

## 7.1 Technology Selection Summary

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      TECHNOLOGY STACK OVERVIEW                               │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  LAYER              │ TECHNOLOGY              │ VERSION    │ STATUS  │   │
│  ├─────────────────────────────────────────────────────────────────────┤   │
│  │  C++ Build System   │ CMake                   │ 4.3.0      │ Current │   │
│  │  C++ Standard       │ C++20                   │ ISO/IEC    │ Current │   │
│  │  C++ UI Framework   │ Qt 6                    │ 6.8.2      │ Current │   │
│  │  C++ JSON           │ nlohmann/json           │ 3.12.0     │ Current │   │
│  │  C++ JWT            │ jwt-cpp                 │ 0.7.0      │ Current │   │
│  │  C++ SQLite         │ SQLiteCpp               │ 3.3.2      │ Current │   │
│  │  C++ HTTP Client    │ libcurl                 │ 8.12.0     │ Current │   │
│  │  C++ WebSocket      │ Qt WebSockets           │ 6.8.2      │ Current │   │
│  │  C++ Testing        │ Google Test             │ 1.17.0     │ Current │   │
│  ├─────────────────────────────────────────────────────────────────────┤   │
│  │  Backend Language   │ Go                      │ 1.24       │ Current │   │
│  │  Backend Framework  │ Gin                     │ 1.12.0     │ Current │   │
│  │  Backend ORM        │ GORM                    │ 1.25.12    │ Current │   │
│  │  Backend WS         │ Gorilla WebSocket       │ 1.5.3      │ Current │   │
│  │  Backend Testing    │ testify                 │ 1.10.0     │ Current │   │
│  ├─────────────────────────────────────────────────────────────────────┤   │
│  │  Database           │ PostgreSQL              │ 17.4       │ Current │   │
│  │  Cloud Platform     │ Google Cloud Platform   │ Latest     │ Current │   │
│  │  Container          │ Docker                  │ 28.0.0     │ Current │   │
│  │  CI/CD              │ GitHub Actions          │ Latest     │ Current │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 7.2 C++ Build System

### Recommendation: CMake 4.3.0

**Justification:**
- **Industry Standard:** CMake is the de facto standard for C++ build systems with excellent IDE support
- **Qt Integration:** Native support for Qt's moc, uic, and rcc tools
- **Cross-Platform:** Works on Windows, macOS, and Linux
- **Package Management:** Excellent integration with vcpkg and Conan
- **Modern Features:** Supports C++20 modules, presets, and improved dependency management

**Alternatives Considered:**
- **xmake:** Faster builds, Lua-based configuration, but smaller community
- **Meson:** Good for GNOME projects, less mature Windows support
- **Bazel:** Excellent for monorepos, steep learning curve

**Documentation:** https://cmake.org/documentation/

## 7.3 C++ UI Framework

### Recommendation: Qt 6.8.2 (with QML)

**Justification:**
- **Performance:** Hardware-accelerated rendering, excellent on Intel HD Graphics 4000
- **Modern UI:** QML enables declarative, reactive UI development
- **Rich Components:** Comprehensive set of widgets and controls
- **WebSocket Support:** Built-in Qt WebSockets module
- **i18n Support:** Excellent internationalization with Qt Linguist
- **Windows Integration:** Native Windows look and feel

**Performance Optimizations for Low-End Hardware:**
- Use `QSG_RENDER_LOOP=basic` environment variable
- Disable animations with `QML_NO_THREADED_RENDERER`
- Limit concurrent QML elements to < 500
- Use `Loader` for on-demand component loading

**Alternatives Considered:**
- **wxWidgets:** Native look, but dated API, limited modern features
- **Dear ImGui:** Excellent for tools, not suitable for consumer applications
- **Sciter:** Good HTML/CSS-like UI, licensing concerns
- **Chromium Embedded Framework:** Heavy resource usage, not suitable for target hardware

**Documentation:** https://doc.qt.io/qt-6/

## 7.4 C++ HTTP/WebSocket Client

### Recommendation: libcurl 8.12.0 + Qt WebSockets

**Justification:**
- **libcurl:** Industry standard, battle-tested, supports all required protocols
- **Qt WebSockets:** Native Qt integration, consistent with UI framework
- **Performance:** Efficient C implementation, low memory footprint
- **Features:** HTTP/2 support, connection pooling, automatic retries

**Configuration for Low-End Hardware:**
```cpp
// Limit concurrent connections
curl_easy_setopt(curl, CURLOPT_MAXCONNECTS, 5);

// Enable connection reuse
curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

// Set reasonable timeouts
curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);

// Enable compression
curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");
```

**Alternatives Considered:**
- **Boost.Beast:** Header-only, complex API, steep learning curve
- **cpp-httplib:** Simple, but less feature-rich
- **uWebSockets client:** Primarily server-focused

**Documentation:** https://curl.se/libcurl/c/

## 7.5 C++ JSON Library

### Recommendation: nlohmann/json 3.12.0

**Justification:**
- **Ease of Use:** Intuitive API, feels like native C++
- **Performance:** Good enough for this use case (not parsing huge JSON)
- **Standards:** Supports JSON Schema, JSON Pointer, JSON Patch
- **Integration:** Single-header option, easy to include
- **C++20 Support:** Modern C++ features, constexpr support

**Example Usage:**
```cpp
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Parsing
json j = json::parse(responseBody);
std::string name = j["user"]["display_name"];

// Serialization
json request = {
    {"company_id", 45},
    {"name", "Mijn Bedrijf"},
    {"initial_budget", 100000.00}
};
std::string body = request.dump();
```

**Alternatives Considered:**
- **RapidJSON:** Faster, but more verbose API
- **simdjson:** Extremely fast, but requires specific CPU features

**Documentation:** https://json.nlohmann.me/

## 7.6 C++ SQLite Wrapper

### Recommendation: SQLiteCpp 3.3.2

**Justification:**
- **RAII Design:** Automatic resource management
- **Modern C++:** Uses exceptions, STL containers
- **Type Safety:** Template-based column extraction
- **Thread Safety:** Built-in SQLite thread safety

**Example Usage:**
```cpp
#include <SQLiteCpp/SQLiteCpp.h>

SQLite::Database db("cache.db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

// Create table
db.exec("CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT)");

// Insert
SQLite::Statement insert(db, "INSERT INTO users (name) VALUES (?)");
insert.bind(1, "Jan");
insert.exec();

// Query
SQLite::Statement query(db, "SELECT id, name FROM users WHERE id = ?");
query.bind(1, 1);
while (query.executeStep()) {
    int id = query.getColumn(0);
    std::string name = query.getColumn(1);
}
```

**Alternatives Considered:**
- **SOCI:** More database backends, heavier dependency
- **Native C API:** More verbose, manual resource management

**Documentation:** https://github.com/SRombauts/SQLiteCpp

## 7.7 C++ JWT Library

### Recommendation: jwt-cpp 0.7.0 (Thalhammer)

**Justification:**
- **Header-Only:** No separate compilation needed
- **Modern C++:** C++17 support, clean API
- **Algorithms:** Supports HS256, RS256, ES256 (all needed)
- **Validation:** Built-in claim validation (exp, nbf, iss, etc.)

**Example Usage:**
```cpp
#include <jwt-cpp/jwt.h>

// Decode and verify
try {
    auto decoded = jwt::decode(token);
    auto verifier = jwt::verify()
        .allow_algorithm(jwt::algorithm::hs256{secret})
        .with_issuer("eduerp")
        .with_audience("eduerp-client");
    verifier.verify(decoded);
    
    // Extract claims
    auto userId = decoded.get_payload_claim("user_id").as_int();
} catch (const std::exception& e) {
    // Token invalid
}
```

**Alternatives Considered:**
- **cpp-jwt:** Similar features, smaller community

**Documentation:** https://github.com/Thalhammer/jwt-cpp

## 7.8 C++ Testing Framework

### Recommendation: Google Test 1.17.0

**Justification:**
- **Industry Standard:** Widely used, excellent documentation
- **Rich Features:** Assertions, parameterized tests, death tests
- **IDE Integration:** Works with Visual Studio, CLion, VS Code
- **CI/CD:** Easy integration with GitHub Actions

**Example Usage:**
```cpp
#include <gtest/gtest.h>

TEST(FinanceModuleTest, CalculateProfitMargin) {
    Company company;
    company.revenue = 100000;
    company.netProfit = 20000;
    
    float margin = calculateProfitMargin(company);
    
    EXPECT_FLOAT_EQ(margin, 0.20f);
}

TEST_F(SimulationTest, CompanyCreation) {
    auto company = createCompany("Test BV", 100000);
    EXPECT_EQ(company->name, "Test BV");
    EXPECT_EQ(company->initialBudget, 100000);
}
```

**Alternatives Considered:**
- **Catch2:** Header-only, simpler syntax
- **doctest:** Fastest compilation, minimal overhead

**Documentation:** https://google.github.io/googletest/

## 7.9 Backend Language & Framework

### Recommendation: Go 1.24 with Gin 1.12.0

**Justification:**
- **Performance:** Compiled language, excellent concurrency with goroutines
- **Simplicity:** Easy to learn, clean syntax
- **Ecosystem:** Excellent libraries for web services
- **Deployment:** Single binary, easy containerization
- **Developer Experience:** Fast compilation, great tooling

**Why Not C++ for Backend:**
- Development speed is more important than raw performance for this use case
- Go's garbage collection simplifies memory management
- Built-in concurrency is easier than C++ threads
- Faster iteration during development

**Gin Framework Benefits:**
- Fast HTTP router (zero-allocation)
- Middleware support
- JSON validation
- Excellent documentation

**Example Handler:**
```go
package main

import (
    "net/http"
    "github.com/gin-gonic/gin"
)

func main() {
    r := gin.Default()
    
    r.POST("/auth/login", handleLogin)
    r.GET("/users/me", authMiddleware(), getCurrentUser)
    
    r.Run(":8080")
}

func handleLogin(c *gin.Context) {
    var req LoginRequest
    if err := c.ShouldBindJSON(&req); err != nil {
        c.JSON(http.StatusBadRequest, ErrorResponse{Error: err.Error()})
        return
    }
    
    // Process login...
    c.JSON(http.StatusOK, LoginResponse{Token: token})
}
```

**Alternatives Considered:**
- **Node.js/Fastify:** Good ecosystem, but single-threaded
- **Python/FastAPI:** Easy development, slower performance
- **Rust/Axum:** Excellent performance, steeper learning curve

**Documentation:** 
- Go: https://golang.org/doc/
- Gin: https://gin-gonic.com/docs/

## 7.10 Backend ORM

### Recommendation: GORM 1.25.12

**Justification:**
- **Go Standard:** Most popular ORM for Go
- **Features:** Migrations, associations, hooks, preloading
- **PostgreSQL:** Excellent PostgreSQL driver support
- **Flexibility:** Can fall back to raw SQL when needed

**Example Usage:**
```go
import "gorm.io/gorm"

type User struct {
    gorm.Model
    Email       string `gorm:"uniqueIndex"`
    DisplayName string
    Role        string
    SchoolID    uint
    School      School
}

// Query
var user User
db.Where("email = ?", "jan@mijnschool.be").First(&user)

// Create
db.Create(&User{Email: "new@mijnschool.be", DisplayName: "Nieuwe Gebruiker"})

// Association
var school School
db.Preload("Users").First(&school, 1)
```

**Documentation:** https://gorm.io/docs/

## 7.11 Backend WebSocket

### Recommendation: Gorilla WebSocket 1.5.3

**Justification:**
- **Mature:** Stable, well-tested library
- **Features:** Full WebSocket protocol support
- **Integration:** Works well with Gin
- **Performance:** Efficient, low memory usage

**Documentation:** https://github.com/gorilla/websocket

## 7.12 Database

### Recommendation: PostgreSQL 17.4

**Justification:**
- **Features:** JSONB, full-text search, row-level security
- **Performance:** Excellent for OLTP workloads
- **Reliability:** ACID compliance, point-in-time recovery
- **Extensions:** Rich ecosystem (PostGIS, etc.)
- **Cloud:** Native Google Cloud SQL support

**Key Features for EduERP:**
- JSONB for flexible simulation data
- Row-level security for multi-tenancy
- Partitioning for audit logs
- Full-text search for user search

**Configuration for Performance:**
```sql
-- Connection pooling (via PgBouncer or application)
-- Shared buffers: 25% of RAM
-- Effective cache size: 50% of RAM
-- Work mem: 4MB
-- Maintenance work mem: 64MB
```

**Documentation:** https://www.postgresql.org/docs/17/

## 7.13 Google Cloud Platform Services

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      GCP SERVICE CONFIGURATION                               │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  SERVICE              │ PURPOSE                    │ COST TIER      │   │
│  ├─────────────────────────────────────────────────────────────────────┤   │
│  │  Cloud SQL (PostgreSQL)│ Primary database          │ db-f1-micro    │   │
│  │  Cloud Run            │ Backend API hosting       │ Free tier      │   │
│  │  Cloud Storage        │ File storage (avatars, etc)│ Standard      │   │
│  │  Secret Manager       │ OAuth secrets, DB creds   │ Free tier      │   │
│  │  Cloud Load Balancing │ HTTPS termination         │ Free tier      │   │
│  │  Cloud Monitoring     │ Logs and metrics          │ Free tier      │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Cost Optimization Strategy:                                                │
│  1. Use Cloud Run free tier (2 million requests/month)                      │
│  2. Use Cloud SQL smallest instance (db-f1-micro)                           │
│  3. Enable automatic shutdown for development                               │
│  4. Use Cloud Storage Nearline for infrequent access                        │
│  5. Set up billing alerts at €50, €100, €200                                │
│                                                                             │
│  Estimated Monthly Cost (Production):                                       │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Service              │ Estimated Cost                            │   │
│  ├─────────────────────────────────────────────────────────────────────┤   │
│  │  Cloud SQL            │ €25-50                                    │   │
│  │  Cloud Run            │ €0-10 (within free tier)                  │   │
│  │  Cloud Storage        │ €5-15                                     │   │
│  │  Network egress       │ €5-10                                     │   │
│  │  Total                │ €35-85/month                              │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 7.14 CI/CD Pipeline

### Recommendation: GitHub Actions

**Workflow Configuration:**
```yaml
# .github/workflows/build.yml
name: Build and Test

on:
  push:
    branches: [main, develop]
  pull_request:
    branches: [main]

jobs:
  build-windows:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v4
      
      - name: Setup Qt
        uses: jurplel/install-qt-action@v4
        with:
          version: '6.8.2'
          modules: 'qtwebsockets qtcharts'
      
      - name: Configure CMake
        run: cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
      
      - name: Build
        run: cmake --build build --config Release
      
      - name: Test
        run: ctest --test-dir build -C Release
      
      - name: Package
        run: cpack -G NSIS -B build
      
      - name: Upload Artifact
        uses: actions/upload-artifact@v4
        with:
          name: EduERP-Windows
          path: build/*.exe
```

**Documentation:** https://docs.github.com/en/actions

---

# PART 8: PHASED DEVELOPMENT PLAN

## 8.1 Development Timeline Overview

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      DEVELOPMENT TIMELINE                                    │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Phase 0: Foundation          ████████░░░░░░░░░░░░  Months 1-2             │
│  Phase 1: Authentication      ░░████████░░░░░░░░░░  Months 2-3             │
│  Phase 2: Admin & Teacher     ░░░░████████░░░░░░░░  Months 3-5             │
│  Phase 3: Core ERP (Single)   ░░░░░░████████████████  Months 5-8           │
│  Phase 4: Full ERP Modules    ░░░░░░░░░░████████░░  Months 8-10            │
│  Phase 5: Collaboration       ░░░░░░░░░░░░████████  Months 10-12           │
│  Phase 6: Messaging & Social  ░░░░░░░░░░░░░░██████  Months 12-14           │
│  Phase 7: Profiles & Custom   ░░░░░░░░░░░░░░░░████  Months 14-15           │
│  Phase 8: Internationalization░░░░░░░░░░░░░░░░░░██  Months 15-16           │
│  Phase 9: Polish & Deploy     ░░░░░░░░░░░░░░░░░░██  Months 16-18           │
│                                                                             │
│  Total Estimated Duration: 18 months                                        │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 8.2 Phase 0 — Foundation (Months 1-2)

**Objective:** Establish project infrastructure, build system, and core architecture

### Deliverables:
- [ ] CMake build system configured
- [ ] Qt 6 project structure set up
- [ ] CI/CD pipeline (GitHub Actions) operational
- [ ] Basic window and navigation shell
- [ ] Theming system (Light/Dark skeleton)
- [ ] Animation system foundation
- [ ] Logging infrastructure

### Sprint Breakdown:

**Sprint 0.1 (Weeks 1-2): Project Setup**
- Set up GitHub repository with proper structure
- Configure CMake with Qt6 integration
- Set up vcpkg for dependency management
- Create initial CI/CD pipeline
- Set up code formatting (clang-format) and linting (clang-tidy)

**Sprint 0.2 (Weeks 3-4): Core Infrastructure**
- Implement core types (Result, Option, UUID)
- Create logging system (spdlog or custom)
- Set up configuration management
- Implement thread pool for async operations

**Sprint 0.3 (Weeks 5-6): UI Foundation**
- Create main window with QML
- Implement navigation shell (sidebar + content area)
- Set up theme system (color palettes)
- Create basic component library (Button, Card, TextField)
- Implement animation configuration system

**Sprint 0.4 (Weeks 7-8): Polish & Testing**
- Write unit tests for core utilities
- Test build on target hardware (if available)
- Create developer documentation
- Set up code coverage reporting

**Demo:** Basic window with navigation, theme switching, animated transitions

---

## 8.3 Phase 1 — Authentication (Months 2-3)

**Objective:** Implement OAuth authentication and session management

### Deliverables:
- [ ] Backend server on GCP (minimal)
- [ ] PostgreSQL schema for users, schools, roles
- [ ] OAuth 2.0 integration with Google and Microsoft
- [ ] JWT session management
- [ ] Domain restriction enforcement
- [ ] Login screen in desktop app
- [ ] Secure token storage (Windows Credential Manager)

### Sprint Breakdown:

**Sprint 1.1 (Weeks 1-2): Backend Foundation**
- Set up Go project with Gin framework
- Configure PostgreSQL on Cloud SQL
- Create initial database migrations
- Implement basic health check endpoint

**Sprint 1.2 (Weeks 3-4): OAuth Backend**
- Implement OAuth 2.0 flow (authorization code + PKCE)
- Add Google OAuth provider integration
- Add Microsoft OAuth provider integration
- Implement domain validation
- Create JWT generation and validation

**Sprint 1.3 (Weeks 5-6): Client Authentication**
- Implement OAuth browser flow in Qt
- Create login screen UI
- Implement token storage (Windows Credential Manager)
- Add session refresh logic
- Create "logged in" home screen

**Sprint 1.4 (Weeks 7-8): Integration & Testing**
- End-to-end authentication testing
- Test domain restriction
- Test token refresh
- Write integration tests

**Demo:** User can log in with Google/Microsoft, sees personalized home screen

---

## 8.4 Phase 2 — Admin & Teacher Management (Months 3-5)

**Objective:** Build school administration and teacher dashboard features

### Deliverables:
- [ ] School administrator panel
- [ ] Class management (create, edit, delete)
- [ ] User management (bulk import from CSV)
- [ ] Teacher dashboard
- [ ] Team creation and role assignment
- [ ] Audit logging

### Sprint Breakdown:

**Sprint 2.1 (Weeks 1-2): Admin Backend**
- Implement school settings API
- Create user management endpoints
- Implement bulk user import
- Add audit logging

**Sprint 2.2 (Weeks 3-4): Class & Team Backend**
- Implement class CRUD operations
- Create team management endpoints
- Implement role assignment logic
- Add class membership tracking

**Sprint 2.3 (Weeks 5-6): Admin UI**
- Create admin dashboard view
- Build user management interface
- Implement CSV import dialog
- Create school settings panel

**Sprint 2.4 (Weeks 7-8): Teacher UI**
- Create teacher dashboard
- Build class overview with student list
- Implement team creation wizard
- Create activity monitoring view

**Demo:** Admin can create classes, import students, assign teachers. Teacher can view class and create teams.

---

## 8.5 Phase 3 — Core ERP Simulation (Single User) (Months 5-8)

**Objective:** Build single-player ERP simulation with core modules

### Deliverables:
- [ ] Company creation flow
- [ ] Finance & Accounting module
- [ ] Sales & CRM module
- [ ] Inventory & Supply Chain module
- [ ] Basic KPI dashboard
- [ ] Simulation save/load with SQLite cache

### Sprint Breakdown:

**Sprint 3.1 (Weeks 1-2): Company Creation**
- Implement company creation wizard
- Create industry template system
- Add initial budget configuration
- Implement company persistence

**Sprint 3.2 (Weeks 3-5): Finance Module**
- Implement chart of accounts
- Create journal entry system
- Build general ledger view
- Implement income statement generation
- Implement balance sheet generation
- Add financial reports export (PDF)

**Sprint 3.3 (Weeks 6-7): Sales Module**
- Create customer database
- Implement sales pipeline
- Build order management
- Add pricing strategies
- Create sales reports

**Sprint 3.4 (Weeks 8-9): Inventory Module**
- Implement product catalog
- Create stock management
- Build supplier management
- Add purchase order system
- Implement low-stock alerts

**Sprint 3.5 (Weeks 10-11): KPI Dashboard**
- Create KPI calculation engine
- Build dashboard UI with charts
- Implement historical data tracking
- Add alert system for thresholds

**Sprint 3.6 (Weeks 12-13): Save/Load & Polish**
- Implement SQLite local cache
- Add save/load functionality
- Test on target hardware
- Performance optimization

**Demo:** Student can create company, manage finances, sales, and inventory. See KPI dashboard.

---

## 8.6 Phase 4 — Full ERP Modules (Months 8-10)

**Objective:** Complete all ERP modules and add AI agents

### Deliverables:
- [ ] HR module with payroll
- [ ] Marketing module with campaigns
- [ ] Logistics & Operations module
- [ ] AI agent system with decision styles
- [ ] Random events system
- [ ] All 9 industry templates

### Sprint Breakdown:

**Sprint 4.1 (Weeks 1-2): HR Module**
- Implement employee management
- Create payroll calculation (Belgian simplified)
- Build org chart view
- Add training system

**Sprint 4.2 (Weeks 3-4): Marketing Module**
- Implement campaign creation
- Create ROI simulation
- Build brand awareness tracking
- Add competitor analysis panel

**Sprint 4.3 (Weeks 5-6): Logistics Module**
- Implement production planning
- Create delivery route simulation
- Add operational cost tracking
- Build quality control metrics

**Sprint 4.4 (Weeks 7-8): AI Agent System**
- Design AI decision framework
- Implement decision strategies (optimal, balanced, risky, poor, random)
- Create role-specific AI agents
- Add decision logging for transparency

**Sprint 4.5 (Weeks 9-10): Events & Templates**
- Implement event system
- Create event database
- Build all 9 industry templates
- Add template-specific configurations

**Demo:** Full ERP with all modules, AI agents can make decisions, events occur during simulation.

---

## 8.7 Phase 5 — Collaboration (Months 10-12)

**Objective:** Add real-time collaboration features

### Deliverables:
- [ ] WebSocket server implementation
- [ ] Real-time data synchronization
- [ ] Role-restricted views
- [ ] Team chat
- [ ] Conflict resolution UI
- [ ] Teacher spectator mode

### Sprint Breakdown:

**Sprint 5.1 (Weeks 1-2): WebSocket Infrastructure**
- Implement WebSocket server (Go)
- Create room/channel system
- Add authentication for WebSocket connections
- Implement heartbeat and reconnection

**Sprint 5.2 (Weeks 3-4): Client WebSocket**
- Add WebSocket client to Qt app
- Implement connection management
- Create message protocol
- Add offline queue for pending changes

**Sprint 5.3 (Weeks 5-6): Real-time Sync**
- Implement field locking mechanism
- Create change broadcasting
- Add conflict detection
- Build conflict resolution UI

**Sprint 5.4 (Weeks 7-8): Team Chat**
- Implement team chat backend
- Create chat UI
- Add typing indicators
- Implement message persistence

**Sprint 5.5 (Weeks 9-10): Role Views & Spectator**
- Implement role-based module visibility
- Create teacher spectator view
- Add activity indicators
- Build real-time activity feed

**Demo:** Multiple students can collaborate on same company in real-time, teacher can observe.

---

## 8.8 Phase 6 — Messaging, Friends, Streaks (Months 12-14)

**Objective:** Add social features

### Deliverables:
- [ ] Direct messaging system
- [ ] Friend requests and list
- [ ] Chat streak system
- [ ] Notification center
- [ ] Windows tray notifications

### Sprint Breakdown:

**Sprint 6.1 (Weeks 1-2): Messaging Backend**
- Implement message storage
- Create conversation endpoints
- Add unread message tracking
- Implement message search

**Sprint 6.2 (Weeks 3-4): Messaging UI**
- Create conversations list
- Build chat view
- Add message composition
- Implement read receipts

**Sprint 6.3 (Weeks 5-6): Friend System**
- Implement friend request flow
- Create friends list UI
- Add online/offline status
- Implement blocking

**Sprint 6.4 (Weeks 7-8): Streaks & Notifications**
- Implement streak calculation
- Create streak UI (flame icon)
- Build notification center
- Add Windows tray notifications

**Demo:** Students can message each other, maintain chat streaks, receive notifications.

---

## 8.9 Phase 7 — Profiles and Customization (Months 14-15)

**Objective:** Add user profile features and customization

### Deliverables:
- [ ] User profile with avatar, banner, bio
- [ ] Custom theme builder
- [ ] Font size settings
- [ ] Animation settings
- [ ] Energy saving mode

### Sprint Breakdown:

**Sprint 7.1 (Weeks 1-2): Profile Features**
- Implement avatar upload
- Add banner upload
- Create profile view
- Build profile edit UI

**Sprint 7.2 (Weeks 3-4): Customization**
- Implement theme builder
- Add color picker
- Create background pattern options
- Build settings UI

**Sprint 7.3 (Weeks 5-6): Accessibility**
- Implement font size options
- Add animation settings
- Create energy saving mode
- Test with screen readers

**Demo:** Students can customize profiles, create custom themes, adjust accessibility settings.

---

## 8.10 Phase 8 — Internationalization (Months 15-16)

**Objective:** Full multi-language support

### Deliverables:
- [ ] Dutch (nl-BE) string extraction
- [ ] English (en-GB) translation
- [ ] French (fr-BE) translation
- [ ] Number and date formatting per locale
- [ ] Language restriction enforcement

### Sprint Breakdown:

**Sprint 8.1 (Weeks 1-2): i18n Infrastructure**
- Set up Qt Linguist workflow
- Extract all strings to .ts files
- Implement runtime language switching
- Add locale-aware formatting

**Sprint 8.2 (Weeks 3-4): Translations**
- Complete Dutch translation review
- Create English translation
- Create French translation
- Test all three languages

**Sprint 8.3 (Weeks 5-6): Formatting**
- Implement Dutch number format (1.234,56)
- Add date formatting per locale
- Create currency formatting
- Test formatting in all modules

**Demo:** Application fully functional in Dutch, English, and French.

---

## 8.11 Phase 9 — Polish, Performance, Deployment (Months 16-18)

**Objective:** Final testing, optimization, and deployment

### Deliverables:
- [ ] Performance profiling on target hardware
- [ ] Memory optimization
- [ ] GDPR compliance audit
- [ ] Data export/deletion implementation
- [ ] GitHub Actions CI/CD pipeline
- [ ] Update notification system
- [ ] Full documentation
- [ ] Installer creation

### Sprint Breakdown:

**Sprint 9.1 (Weeks 1-2): Performance**
- Profile on Intel i3 / 8GB RAM hardware
- Identify bottlenecks
- Optimize rendering
- Reduce memory usage

**Sprint 9.2 (Weeks 3-4): GDPR Compliance**
- Implement data export (JSON/CSV)
- Create account deletion flow
- Add privacy policy
- Audit data retention

**Sprint 9.3 (Weeks 5-6): CI/CD & Updates**
- Complete GitHub Actions pipeline
- Implement update checker
- Create update notification UI
- Set up release process

**Sprint 9.4 (Weeks 7-8): Documentation & Installer**
- Write user manual (Dutch)
- Create developer documentation
- Build Windows installer (Inno Setup)
- Create deployment guide

**Sprint 9.5 (Weeks 9-10): Final Testing**
- End-to-end testing
- User acceptance testing with school
- Bug fixes
- Performance validation

**Demo:** Production-ready application, installer working, documentation complete.

---

## 8.12 Milestone Summary

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      MILESTONE CHECKPOINTS                                   │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  M1 (Month 2):  Foundation Complete                                         │
│  ✓ Build system working                                                     │
│  ✓ Basic UI shell operational                                               │
│                                                                             │
│  M2 (Month 3):  Authentication Working                                      │
│  ✓ Users can log in with OAuth                                              │
│  ✓ Session management functional                                            │
│                                                                             │
│  M3 (Month 5):  Admin Features Complete                                     │
│  ✓ School admin can manage users and classes                                │
│  ✓ Teacher can create teams                                                 │
│                                                                             │
│  M4 (Month 8):  Core ERP Functional                                         │
│  ✓ Single-player simulation works                                           │
│  ✓ Finance, Sales, Inventory modules complete                               │
│                                                                             │
│  M5 (Month 10): Full ERP Complete                                           │
│  ✓ All modules implemented                                                  │
│  ✓ AI agents functional                                                     │
│                                                                             │
│  M6 (Month 12): Collaboration Working                                       │
│  ✓ Real-time sync functional                                                │
│  ✓ Team chat operational                                                    │
│                                                                             │
│  M7 (Month 14): Social Features Complete                                    │
│  ✓ Messaging, friends, streaks working                                      │
│  ✓ Notifications functional                                                 │
│                                                                             │
│  M8 (Month 16): Internationalization Complete                               │
│  ✓ All three languages supported                                            │
│  ✓ Proper formatting per locale                                             │
│                                                                             │
│  M9 (Month 18): Production Ready                                            │
│  ✓ Performance validated on target hardware                                 │
│  ✓ GDPR compliant                                                           │
│  ✓ Installer and documentation complete                                     │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

# PART 9: RISK REGISTER

## 9.1 Top 10 Project Risks

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      RISK REGISTER                                           │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  RISK #1: Performance Issues on Target Hardware                             │
│  ═══════════════════════════════════════════════════                        │
│  Probability: HIGH    Impact: HIGH    Risk Level: CRITICAL                  │
│                                                                             │
│  Description: The application may not perform adequately on Intel Core i3   │
│  3rd-gen with 8GB RAM and integrated graphics, leading to poor user         │
│  experience and potential rejection by the school.                          │
│                                                                             │
│  Mitigation Strategies:                                                     │
│  1. Establish performance benchmarks early in development                   │
│  2. Profile regularly on target hardware (or equivalent VM)                 │
│  3. Implement performance budgets (memory, frame time)                      │
│  4. Use hardware-accelerated rendering where possible                       │
│  5. Implement "energy saving mode" for low-end hardware                     │
│  6. Lazy-load modules and components                                        │
│  7. Cache expensive calculations                                            │
│                                                                             │
│  Contingency: If performance remains unacceptable, consider:                │
│  - Reducing visual complexity                                               │
│  - Removing non-essential animations                                        │
│  - Implementing more aggressive caching                                     │
│                                                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  RISK #2: OAuth Integration Complexity                                      │
│  ═══════════════════════════════════════                                    │
│  Probability: MEDIUM    Impact: HIGH    Risk Level: HIGH                    │
│                                                                             │
│  Description: OAuth 2.0 / OpenID Connect integration with Google and        │
│  Microsoft may encounter issues with domain restrictions, token handling,   │
│  or browser integration on Windows.                                         │
│                                                                             │
│  Mitigation Strategies:                                                     │
│  1. Use well-tested libraries (jwt-cpp, Qt OAuth)                           │
│  2. Implement PKCE for security                                             │
│  3. Test thoroughly with both providers                                     │
│  4. Have fallback authentication method ready                               │
│  5. Implement comprehensive error handling                                  │
│  6. Test on school network (may have restrictions)                          │
│                                                                             │
│  Contingency: If OAuth fails, implement:                                    │
│  - Direct school LDAP/Active Directory integration                          │
│  - Manual account creation by admin                                         │
│                                                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  RISK #3: Real-time Synchronization Complexity                              │
│  ═══════════════════════════════════════════════════                        │
│  Probability: HIGH    Impact: HIGH    Risk Level: CRITICAL                  │
│                                                                             │
│  Description: WebSocket-based real-time collaboration is complex to         │
│  implement correctly. Issues with conflict resolution, data consistency,    │
│  or reconnection handling could severely impact user experience.            │
│                                                                             │
│  Mitigation Strategies:                                                     │
│  1. Start with simple polling, migrate to WebSockets                        │
│  2. Implement operational transformation for concurrent edits               │
│  3. Use field-level locking to prevent conflicts                            │
│  4. Implement comprehensive reconnection logic                              │
│  5. Add extensive logging for debugging                                     │
│  6. Test with simulated network failures                                    │
│  7. Implement offline mode as fallback                                      │
│                                                                             │
│  Contingency: If WebSocket proves too complex:                              │
│  - Use server-sent events for one-way updates                               │
│  - Implement turn-based mode as primary                                     │
│                                                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  RISK #4: GDPR / AVG Compliance                                             │
│  ═══════════════════════════════                                            │
│  Probability: MEDIUM    Impact: HIGH    Risk Level: HIGH                    │
│                                                                             │
│  Description: Processing student data requires strict compliance with       │
│  GDPR/AVG. Non-compliance could result in legal issues and loss of trust.   │
│                                                                             │
│  Mitigation Strategies:                                                     │
│  1. Implement data minimization (collect only what's needed)                │
│  2. Add comprehensive audit logging                                         │
│  3. Implement data export functionality                                     │
│  4. Implement account deletion (right to erasure)                           │
│  5. Create privacy policy in Dutch                                          │
│  6. Encrypt sensitive data at rest and in transit                           │
│  7. Conduct GDPR compliance audit before release                            │
│  8. Consult with school's data protection officer                           │
│                                                                             │
│  Contingency: If compliance issues arise:                                   │
│  - Engage legal counsel specializing in Belgian privacy law                 │
│  - Implement additional safeguards                                          │
│                                                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  RISK #5: Scope Creep                                                       │
│  ════════════════════                                                       │
│  Probability: HIGH    Impact: MEDIUM    Risk Level: HIGH                    │
│                                                                             │
│  Description: The project has many features. Adding new features during     │
│  development could delay delivery and compromise quality.                   │
│                                                                             │
│  Mitigation Strategies:                                                     │
│  1. Create detailed requirements document (this spec)                       │
│  2. Implement change control process                                        │
│  3. Prioritize features using MoSCoW method                                 │
│  4. Set clear milestones and deadlines                                      │
│  5. Regular progress reviews with stakeholders                              │
│  6. Use agile but maintain scope discipline                                 │
│  7. Defer non-essential features to future releases                         │
│                                                                             │
│  Contingency: If scope must increase:                                       │
│  - Extend timeline with stakeholder approval                                │
│  - Remove lower-priority features                                           │
│                                                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  RISK #6: Backend Scalability Issues                                        │
│  ═══════════════════════════════════                                        │
│  Probability: LOW    Impact: MEDIUM    Risk Level: MEDIUM                   │
│                                                                             │
│  Description: If the application becomes popular, the backend may not       │
│  handle the load, causing slow response times or outages.                   │
│                                                                             │
│  Mitigation Strategies:                                                     │
│  1. Design for horizontal scaling from the start                            │
│  2. Use stateless API design                                                │
│  3. Implement caching (Redis) for frequently accessed data                  │
│  4. Use connection pooling for database                                     │
│  5. Implement rate limiting                                                 │
│  6. Set up monitoring and alerting                                          │
│  7. Load test before production deployment                                  │
│                                                                             │
│  Contingency: If scaling issues occur:                                      │
│  - Upgrade Cloud SQL instance                                               │
│  - Add read replicas                                                        │
│  - Implement CDN for static assets                                          │
│                                                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  RISK #7: Third-Party Dependency Issues                                     │
│  ═════════════════════════════════════                                      │
│  Probability: MEDIUM    Impact: MEDIUM    Risk Level: MEDIUM                │
│                                                                             │
│  Description: Third-party libraries may have bugs, security vulnerabilities │
│  or become abandoned, affecting application stability and security.         │
│                                                                             │
│  Mitigation Strategies:                                                     │
│  1. Choose mature, actively maintained libraries                            │
│  2. Pin dependency versions in build configuration                          │
│  3. Regularly update dependencies                                           │
│  4. Use dependency scanning tools (Dependabot)                              │
│  5. Have alternatives identified for critical dependencies                  │
│  6. Vendor critical dependencies (include in repo)                          │
│                                                                             │
│  Contingency: If critical dependency fails:                                 │
│  - Fork and maintain if necessary                                           │
│  - Switch to alternative library                                            │
│  - Implement custom solution                                                │
│                                                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  RISK #8: Cross-Platform Build Issues                                       │
│  ═══════════════════════════════════                                        │
│  Probability: MEDIUM    Impact: MEDIUM    Risk Level: MEDIUM                │
│                                                                             │
│  Description: Building and packaging for Windows may encounter issues       │
│  with library dependencies, installer creation, or runtime errors.          │
│                                                                             │
│  Mitigation Strategies:                                                     │
│  1. Use CMake for cross-platform builds                                     │
│  2. Set up CI/CD for Windows early                                          │
│  3. Use vcpkg for dependency management                                     │
│  4. Test on clean Windows VMs                                               │
│  5. Create installer early and test frequently                              │
│  6. Document build process thoroughly                                       │
│                                                                             │
│  Contingency: If build issues persist:                                      │
│  - Use Docker for reproducible builds                                       │
│  - Engage Windows packaging expert                                          │
│                                                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  RISK #9: Network Connectivity Issues at School                             │
│  ═══════════════════════════════════════════                                │
│  Probability: MEDIUM    Impact: HIGH    Risk Level: HIGH                    │
│                                                                             │
│  Description: School networks may have firewalls, proxies, or content       │
│  filters that block the application's network traffic.                      │
│                                                                             │
│  Mitigation Strategies:                                                     │
│  1. Use standard HTTPS port (443)                                           │
│  2. Implement WebSocket over HTTPS (WSS)                                    │
│  3. Design robust offline mode                                              │
│  4. Test with school IT department early                                    │
│  5. Provide network configuration documentation                             │
│  6. Implement connection quality detection                                  │
│  7. Graceful degradation when offline                                       │
│                                                                             │
│  Contingency: If network is blocked:                                        │
│  - Work with school IT to whitelist domains                                 │
│  - Provide on-premise server option                                         │
│                                                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  RISK #10: Developer Availability / Knowledge Gaps                          │
│  ═══════════════════════════════════════════════════                        │
│  Probability: MEDIUM    Impact: HIGH    Risk Level: HIGH                    │
│                                                                             │
│  Description: The project requires expertise in C++, Qt, Go, and cloud      │
│  technologies. Knowledge gaps or developer unavailability could delay       │
│  the project.                                                               │
│                                                                             │
│  Mitigation Strategies:                                                     │
│  1. Start with smaller proof-of-concept projects                            │
│  2. Document architecture decisions thoroughly                              │
│  3. Create comprehensive developer documentation                            │
│  4. Use pair programming for knowledge transfer                             │
│  5. Build modular architecture to allow parallel work                       │
│  6. Identify backup developers/resources                                    │
│  7. Consider hiring consultants for specific areas                          │
│                                                                             │
│  Contingency: If knowledge gaps are significant:                            │
│  - Extend timeline for learning                                             │
│  - Simplify architecture                                                    │
│  - Outsource specific components                                            │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 9.2 Risk Monitoring

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      RISK MONITORING PROCESS                                 │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Regular Risk Review:                                                       │
│  - Weekly during active development                                         │
│  - Bi-weekly during testing phases                                          │
│  - Monthly during maintenance                                               │
│                                                                             │
│  Risk Metrics to Track:                                                     │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Metric                    │ Target    │ Action if Exceeded        │   │
│  ├─────────────────────────────────────────────────────────────────────┤   │
│  │  Application startup time  │ < 5 sec   │ Profile and optimize      │   │
│  │  Memory usage (idle)       │ < 200 MB  │ Investigate leaks         │   │
│  │  Frame rate (animations)   │ > 30 fps  │ Reduce complexity         │   │
│  │  API response time (p95)   │ < 500 ms  │ Scale backend             │   │
│  │  Test coverage             │ > 70%     │ Add more tests            │   │
│  │  Open critical bugs        │ 0         │ Stop release              │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Escalation Process:                                                        │
│  1. Developer identifies risk → Log in issue tracker                        │
│  2. Risk assessed within 24 hours                                           │
│  3. If HIGH/CRITICAL → Immediate team meeting                               │
│  4. Mitigation plan created within 48 hours                                 │
│  5. Progress tracked daily until resolved                                   │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

# PART 10: SCHOOL DEPLOYMENT GUIDE OUTLINE

## 10.1 Pre-Deployment Checklist

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      PRE-DEPLOYMENT CHECKLIST                                │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  □ Network Requirements Verified                                            │
│    - Firewall rules allow HTTPS (443) outbound                              │
│    - WebSocket connections allowed (WSS on 443)                             │
│    - Domains whitelisted: api.eduerp.example.com, storage...                │
│                                                                             │
│  □ Hardware Requirements Met                                                │
│    - Windows 10/11 on all student devices                                   │
│    - Minimum 8GB RAM recommended                                            │
│    - Intel Core i3 3rd gen or better                                        │
│    - 500MB free disk space per installation                                 │
│                                                                             │
│  □ Google Workspace for Education Configured                                │
│    - Admin access to Google Admin Console                                   │
│    - OAuth consent screen configured                                        │
│    - Domain verification completed                                          │
│                                                                             │
│  □ Microsoft Azure AD Configured (if using Microsoft login)                 │
│    - Azure AD admin access                                                  │
│    - Application registration created                                       │
│    - Redirect URIs configured                                               │
│                                                                             │
│  □ School IT Contact Identified                                             │
│    - Primary contact for deployment                                         │
│    - Backup contact identified                                              │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 10.2 OAuth App Registration

### 10.2.1 Google OAuth Setup

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      GOOGLE OAUTH SETUP                                      │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Step 1: Access Google Cloud Console                                        │
│  ─────────────────────────────────────                                      │
│  1. Go to https://console.cloud.google.com/                                 │
│  2. Sign in with school Google admin account                                │
│  3. Create new project "EduERP Authentication"                              │
│                                                                             │
│  Step 2: Configure OAuth Consent Screen                                     │
│  ────────────────────────────────────────                                   │
│  1. Navigate to "APIs & Services" > "OAuth consent screen"                  │
│  2. Select "Internal" (for Google Workspace organizations)                  │
│  3. Fill in application information:                                        │
│     - App name: "EduERP"                                                    │
│     - User support email: IT support email                                  │
│     - Developer contact: developer email                                    │
│  4. Add scopes:                                                             │
│     - openid                                                                │
│     - email                                                                 │
│     - profile                                                               │
│  5. Add test users (for initial testing)                                    │
│                                                                             │
│  Step 3: Create OAuth Credentials                                           │
│  ──────────────────────────────────                                         │
│  1. Navigate to "Credentials"                                               │
│  2. Click "Create Credentials" > "OAuth client ID"                          │
│  3. Select "Desktop app" as application type                                │
│  4. Name: "EduERP Desktop Client"                                           │
│  5. Download client credentials JSON                                        │
│  6. Store securely - will be needed for EduERP configuration                │
│                                                                             │
│  Step 4: Domain Restriction (Recommended)                                   │
│  ────────────────────────────────────────                                   │
│  1. In OAuth consent screen settings                                        │
│  2. Add authorized domains:                                                 │
│     - yourschool.be                                                         │
│     - student.yourschool.be (if applicable)                                 │
│                                                                             │
│  Credentials to Save:                                                       │
│  - Client ID (e.g., 123456789-abc.apps.googleusercontent.com)               │
│  - Client Secret (keep confidential!)                                       │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 10.2.2 Microsoft Azure AD Setup

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      MICROSOFT AZURE AD SETUP                                │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Step 1: Access Azure Portal                                                │
│  ────────────────────────────                                               │
│  1. Go to https://portal.azure.com/                                         │
│  2. Sign in with school Azure AD admin account                              │
│                                                                             │
│  Step 2: Register Application                                               │
│  ────────────────────────────                                               │
│  1. Navigate to "Azure Active Directory" > "App registrations"              │
│  2. Click "New registration"                                                │
│  3. Fill in details:                                                        │
│     - Name: "EduERP"                                                        │
│     - Supported account types: "Accounts in this organizational directory"  │
│     - Redirect URI: (leave empty for now)                                   │
│  4. Click "Register"                                                        │
│                                                                             │
│  Step 3: Configure Authentication                                           │
│  ────────────────────────────────                                           │
│  1. Go to "Authentication" in left menu                                     │
│  2. Click "Add a platform" > "Mobile and desktop applications"              │
│  3. Add custom redirect URI:                                                │
│     - com.eduerp://oauth/callback                                           │
│  4. Enable "Allow public client flows" (for PKCE)                           │
│                                                                             │
│  Step 4: Configure API Permissions                                          │
│  ───────────────────────────────────                                        │
│  1. Go to "API permissions"                                                 │
│  2. Click "Add a permission" > "Microsoft Graph" > "Delegated permissions"  │
│  3. Add permissions:                                                        │
│     - openid                                                                │
│     - email                                                                 │
│     - profile                                                               │
│     - User.Read                                                             │
│  4. Click "Grant admin consent"                                             │
│                                                                             │
│  Step 5: Get Credentials                                                    │
│  ────────────────────────                                                   │
│  1. Go to "Overview"                                                        │
│  2. Copy "Application (client) ID"                                          │
│  3. Go to "Certificates & secrets"                                          │
│  4. Create new client secret (note expiration date!)                        │
│                                                                             │
│  Credentials to Save:                                                       │
│  - Application (client) ID                                                  │
│  - Client Secret (keep confidential!)                                       │
│  - Directory (tenant) ID                                                    │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 10.3 EduERP Server Configuration

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      SERVER CONFIGURATION                                    │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  This section describes configuration for the EduERP backend server.        │
│  For schools using the hosted service, most of this is handled by the       │
│  EduERP development team. For self-hosted deployments, follow these steps.  │
│                                                                             │
│  Step 1: Google Cloud Project Setup                                         │
│  ───────────────────────────────────                                        │
│  1. Create new GCP project or use existing                                  │
│  2. Enable billing (set up budget alerts)                                   │
│  3. Enable required APIs:                                                   │
│     - Cloud SQL Admin API                                                   │
│     - Cloud Run API                                                         │
│     - Secret Manager API                                                    │
│     - Cloud Storage API                                                     │
│                                                                             │
│  Step 2: Database Setup (Cloud SQL)                                         │
│  ───────────────────────────────────                                        │
│  1. Navigate to "SQL" in Cloud Console                                      │
│  2. Click "Create instance" > "PostgreSQL"                                  │
│  3. Choose configuration:                                                   │
│     - Instance ID: eduerp-db-[school-name]                                  │
│     - Password: Generate strong password, store in Secret Manager           │
│     - Region: europe-west1 (Belgium)                                        │
│     - Database version: PostgreSQL 17                                       │
│     - Machine type: db-f1-micro (development) or db-g1-small (production)   │
│     - Storage: 10GB SSD, auto-expand enabled                                │
│     - Backups: Enabled, 7-day retention                                     │
│  4. Note the connection name for later use                                  │
│                                                                             │
│  Step 3: Secret Manager Setup                                               │
│  ────────────────────────────────                                           │
│  1. Navigate to "Secret Manager"                                            │
│  2. Create secrets:                                                         │
│     - db-password: PostgreSQL password                                      │
│     - jwt-secret: Random 256-bit key for JWT signing                        │
│     - google-oauth-client-id: From Google OAuth setup                       │
│     - google-oauth-client-secret: From Google OAuth setup                   │
│     - microsoft-oauth-client-id: From Azure AD setup                        │
│     - microsoft-oauth-client-secret: From Azure AD setup                    │
│                                                                             │
│  Step 4: Cloud Run Deployment                                               │
│  ────────────────────────────────                                           │
│  1. Build Docker image:                                                     │
│     docker build -t gcr.io/PROJECT_ID/eduerp-backend:latest .               │
│     docker push gcr.io/PROJECT_ID/eduerp-backend:latest                     │
│                                                                             │
│  2. Deploy to Cloud Run:                                                    │
│     gcloud run deploy eduerp-backend \                                      │
│       --image gcr.io/PROJECT_ID/eduerp-backend:latest \                     │
│       --platform managed \                                                  │
│       --region europe-west1 \                                               │
│       --allow-unauthenticated \                                             │
│       --set-secrets=DB_PASSWORD=db-password:latest \                        │
│       --set-secrets=JWT_SECRET=jwt-secret:latest \                          │
│       --set-env-vars=DB_CONNECTION_NAME=PROJECT:REGION:INSTANCE             │
│                                                                             │
│  3. Note the service URL for client configuration                           │
│                                                                             │
│  Step 5: Cloud Storage Bucket                                               │
│  ────────────────────────────────                                           │
│  1. Create bucket for file storage:                                         │
│     gsutil mb -l europe-west1 gs://eduerp-files-[school-name]               │
│  2. Configure CORS for client access                                        │
│  3. Set up lifecycle policy for old file deletion                           │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 10.4 Client Installation

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      CLIENT INSTALLATION                                     │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Method 1: MSI Installer (Recommended for School Deployment)                │
│  ═══════════════════════════════════════════════════════════════════════    │
│                                                                             │
│  For IT Administrators (Silent Installation):                               │
│  ```                                                                        │
│  msiexec /i EduERP-1.0.0.msi /quiet /norestart \                            │
│    INSTALLDIR="C:\Program Files\EduERP" \                                   │
│    ALLUSERS=1                                                               │
│  ```                                                                        │
│                                                                             │
│  For Individual Users:                                                      │
│  1. Download EduERP-1.0.0.msi from school portal                            │
│  2. Double-click to run installer                                           │
│  3. Follow installation wizard                                              │
│  4. Launch EduERP from Start Menu                                           │
│                                                                             │
│  Method 2: Portable ZIP (For Testing)                                       │
│  ═══════════════════════════════════════                                    │
│  1. Download EduERP-1.0.0-portable.zip                                      │
│  2. Extract to desired location                                             │
│  3. Run EduERP.exe                                                          │
│                                                                             │
│  System Requirements Check:                                                 │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Component        │ Minimum              │ Recommended              │   │
│  ├─────────────────────────────────────────────────────────────────────┤   │
│  │  OS               │ Windows 10 1903+     │ Windows 11               │   │
│  │  Processor        │ Intel Core i3 3rd gen│ Intel Core i5 8th gen    │   │
│  │  Memory           │ 8 GB RAM             │ 16 GB RAM                │   │
│  │  Storage          │ 500 MB available     │ 1 GB available           │   │
│  │  Display          │ 1366x768             │ 1920x1080                │   │
│  │  Internet         │ 2 Mbps               │ 10 Mbps                  │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Troubleshooting Installation Issues:                                       │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Issue                    │ Solution                                │   │
│  ├─────────────────────────────────────────────────────────────────────┤   │
│  │  "Missing DLL" error      │ Install Visual C++ Redistributable      │   │
│  │  "Cannot connect" error   │ Check firewall/proxy settings           │   │
│  │  Slow performance         │ Enable energy saving mode in settings   │   │
│  │  Crashes on startup       │ Check Windows Event Viewer logs         │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 10.5 First-Time Setup

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      FIRST-TIME SETUP                                        │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Step 1: Create Super Admin Account                                         │
│  ───────────────────────────────────                                        │
│  1. Access EduERP admin panel at https://admin.eduerp.example.com           │
│  2. Click "Initial Setup"                                                   │
│  3. Enter school information:                                               │
│     - School name                                                           │
│     - Contact email                                                         │
│     - Allowed email domains (e.g., @yourschool.be)                          │
│  4. Create super admin account                                              │
│                                                                             │
│  Step 2: Configure OAuth Providers                                          │
│  ───────────────────────────────────                                        │
│  1. Go to "Authentication" > "OAuth Settings"                               │
│  2. Add Google OAuth:                                                       │
│     - Paste Client ID from Google setup                                     │
│     - Paste Client Secret from Google setup                                 │
│  3. Add Microsoft OAuth (if applicable):                                    │
│     - Paste Application ID from Azure setup                                 │
│     - Paste Client Secret from Azure setup                                  │
│     - Enter Tenant ID                                                       │
│  4. Test authentication with test account                                   │
│                                                                             │
│  Step 3: Configure School Settings                                          │
│  ───────────────────────────────────                                        │
│  1. Go to "School Settings"                                                 │
│  2. Set default language (Nederlands)                                       │
│  3. Configure allowed languages                                             │
│  4. Set theme restrictions (optional)                                       │
│  5. Configure feature toggles:                                              │
│     - Enable/disable streaks                                                │
│     - Enable/disable friend system                                          │
│     - Allow cross-class messaging                                           │
│                                                                             │
│  Step 4: Create School Year                                                 │
│  ────────────────────────────────                                           │
│  1. Go to "Academic Years"                                                  │
│  2. Create new academic year (e.g., 2025-2026)                              │
│  3. Set start and end dates                                                 │
│                                                                             │
│  Step 5: Create First Class                                                 │
│  ────────────────────────────────                                           │
│  1. Go to "Classes" > "Create Class"                                        │
│  2. Enter class details:                                                    │
│     - Name (e.g., "5de jaar Economie A")                                    │
│     - Academic year                                                         │
│     - Assigned teacher                                                      │
│  3. Configure simulation settings                                           │
│                                                                             │
│  Step 6: Import Students                                                    │
│  ────────────────────────────────                                           │
│  1. Prepare CSV file with columns:                                          │
│     - email (must match school domain)                                      │
│     - display_name                                                          │
│     - class_id (optional)                                                   │
│  2. Go to "Users" > "Bulk Import"                                           │
│  3. Upload CSV file                                                         │
│  4. Review import preview                                                   │
│  5. Confirm import                                                          │
│                                                                             │
│  Step 7: Notify Users                                                       │
│  ────────────────────────────────                                           │
│  1. Download EduERP installer                                               │
│  2. Distribute to students and teachers                                     │
│  3. Provide login instructions                                              │
│  4. Schedule training session if needed                                     │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 10.6 Ongoing Maintenance

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      ONGOING MAINTENANCE                                     │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Daily Tasks:                                                               │
│  ─────────────                                                              │
│  □ Monitor application error logs                                           │
│  □ Check server health dashboard                                            │
│  □ Review authentication failure rates                                      │
│                                                                             │
│  Weekly Tasks:                                                              │
│  ──────────────                                                             │
│  □ Review user activity reports                                             │
│  □ Check storage usage                                                      │
│  □ Verify backup completion                                                 │
│  □ Review security alerts                                                   │
│                                                                             │
│  Monthly Tasks:                                                             │
│  ───────────────                                                            │
│  □ Update student accounts (new enrollments, departures)                    │
│  □ Review and rotate OAuth credentials (if needed)                          │
│  □ Check for EduERP updates                                                 │
│  □ Review access logs for anomalies                                         │
│  □ Test backup restoration process                                          │
│                                                                             │
│  Quarterly Tasks:                                                           │
│  ─────────────────                                                          │
│  □ Security audit                                                           │
│  □ Performance review                                                       │
│  □ Update documentation                                                     │
│  □ Review and update privacy policy                                         │
│  □ Disaster recovery drill                                                  │
│                                                                             │
│  Annual Tasks:                                                              │
│  ───────────────                                                            │
│  □ Full security penetration test                                           │
│  □ GDPR compliance audit                                                    │
│  □ Review and renew SSL certificates                                        │
│  □ Archive old academic year data                                           │
│  □ Budget review for cloud costs                                            │
│                                                                             │
│  Backup Strategy:                                                           │
│  ─────────────────                                                          │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Type        │ Frequency │ Retention │ Storage Location            │   │
│  ├─────────────────────────────────────────────────────────────────────┤   │
│  │  Database    │ Daily     │ 30 days   │ Cloud SQL automated         │   │
│  │  Full DB     │ Weekly    │ 12 weeks  │ Cloud Storage (separate)    │   │
│  │  Files       │ Daily     │ 30 days   │ Cloud Storage versioning    │   │
│  │  Config      │ On change │ 10 versions│ Secret Manager versions    │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Update Process:                                                            │
│  ────────────────                                                           │
│  1. Review release notes for new version                                    │
│  2. Test update in staging environment                                      │
│  3. Schedule maintenance window                                             │
│  4. Notify users of upcoming update                                         │
│  5. Deploy backend update (if applicable)                                   │
│  6. Distribute new client installer                                         │
│  7. Verify update successful                                                │
│                                                                             │
│  Support Contacts:                                                          │
│  ──────────────────                                                         │
│  - Technical Issues: it-support@yourschool.be                               │
│  - EduERP Questions: eduerp@yourschool.be                                   │
│  - Feature Requests: Submit via GitHub Issues                               │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 10.7 Troubleshooting Guide

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      TROUBLESHOOTING GUIDE                                   │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Issue: Students Cannot Log In                                              │
│  ─────────────────────────────────────                                      │
│  Possible Causes:                                                           │
│  1. Email domain not in allowed list                                        │
│     → Check school settings, add domain                                     │
│  2. OAuth credentials expired or incorrect                                  │
│     → Verify OAuth settings, regenerate if needed                           │
│  3. Student account not created                                             │
│     → Check user list, create account or re-import                          │
│  4. Network blocking OAuth                                                  │
│     → Check firewall rules, whitelist OAuth domains                         │
│                                                                             │
│  Issue: Application is Slow                                                 │
│  ────────────────────────────────                                           │
│  Possible Causes:                                                           │
│  1. Insufficient hardware resources                                         │
│     → Enable energy saving mode in settings                                 │
│  2. Network latency                                                         │
│     → Check network connection, use wired if possible                       │
│  3. Too many concurrent users                                               │
│     → Scale backend resources                                               │
│  4. Large data sets                                                         │
│     → Archive old simulation data                                           │
│                                                                             │
│  Issue: Real-time Collaboration Not Working                                 │
│  ─────────────────────────────────────────                                  │
│  Possible Causes:                                                           │
│  1. WebSocket blocked by firewall                                           │
│     → Allow WSS connections on port 443                                     │
│  2. Proxy interfering with WebSocket                                        │
│     → Configure proxy to support WebSocket                                  │
│  3. Server overload                                                         │
│     → Check server metrics, scale if needed                                 │
│                                                                             │
│  Issue: Data Loss or Corruption                                             │
│  ────────────────────────────────────                                       │
│  Immediate Actions:                                                         │
│  1. Stop all simulations to prevent further damage                          │
│  2. Identify affected users/companies                                       │
│  3. Restore from most recent backup                                         │
│  4. Notify affected users                                                   │
│  5. Investigate root cause                                                  │
│  6. Document incident for future prevention                                 │
│                                                                             │
│  Issue: GDPR Data Request                                                   │
│  ────────────────────────────────                                           │
│  Process:                                                                   │
│  1. Verify identity of requester                                            │
│  2. Generate data export via admin panel                                    │
│  3. Review export for third-party data                                      │
│  4. Provide export within 30 days                                           │
│  5. Document request and response                                           │
│                                                                             │
│  Issue: Account Deletion Request                                            │
│  ────────────────────────────────────                                       │
│  Process:                                                                   │
│  1. Verify identity of requester (or parental consent for minors)           │
│  2. Export data if requested                                                │
│  3. Delete account via admin panel                                          │
│  4. Verify deletion from all systems                                        │
│  5. Document deletion for audit                                             │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

# APPENDIX A: GLOSSARY

| Term | Definition |
|------|------------|
| **API** | Application Programming Interface - defines how software components communicate |
| **AVG** | Algemene Verordening Gegevensbescherming - Dutch term for GDPR |
| **CI/CD** | Continuous Integration/Continuous Deployment - automated build and deployment |
| **CRM** | Customer Relationship Management - managing interactions with customers |
| **ERP** | Enterprise Resource Planning - integrated business management software |
| **GDPR** | General Data Protection Regulation - EU data protection law |
| **JWT** | JSON Web Token - secure method for transmitting authentication info |
| **KPI** | Key Performance Indicator - measurable value showing business performance |
| **OAuth** | Open Authorization - protocol for secure delegated access |
| **PKCE** | Proof Key for Code Exchange - OAuth security extension |
| **QML** | Qt Meta Language - declarative UI language for Qt |
| **REST** | Representational State Transfer - architectural style for APIs |
| **RLS** | Row-Level Security - database access control mechanism |
| **SKU** | Stock Keeping Unit - unique identifier for products |
| **TLS** | Transport Layer Security - cryptographic protocol for secure communication |
| **VAT** | Value Added Tax - BTW in Dutch |
| **WCAG** | Web Content Accessibility Guidelines - accessibility standards |
| **WebSocket** | Protocol for full-duplex communication over TCP |

---

# APPENDIX B: REFERENCE DOCUMENTATION

## B.1 Official Documentation Links

### C++ Development
- CMake: https://cmake.org/documentation/
- Qt 6: https://doc.qt.io/qt-6/
- C++ Reference: https://en.cppreference.com/

### Backend Development
- Go: https://golang.org/doc/
- Gin: https://gin-gonic.com/docs/
- GORM: https://gorm.io/docs/
- PostgreSQL: https://www.postgresql.org/docs/17/

### Cloud Platform
- Google Cloud: https://cloud.google.com/docs
- Cloud SQL: https://cloud.google.com/sql/docs
- Cloud Run: https://cloud.google.com/run/docs

### Security
- OAuth 2.0: https://oauth.net/2/
- JWT: https://jwt.io/introduction
- GDPR: https://gdpr.eu/

## B.2 Educational Resources

### ERP Concepts
- SAP Learning Hub: https://learning.sap.com/
- Oracle NetSuite Documentation: https://docs.oracle.com/en/cloud/saas/netsuite/

### Belgian Accounting
- BTW (VAT) Guide: https://financien.belgium.be/nl/btw
- Social Security: https://www.socialsecurity.be/

---

**Document Version:** 1.0.0  
**Last Updated:** March 26, 2026  
**Author:** EduERP Development Team  
**License:** MIT License (Open Source)

---

*This document is a living specification. As the project evolves, updates should be made to reflect architectural decisions, new features, and lessons learned during development.*

