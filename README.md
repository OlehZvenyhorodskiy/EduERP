# EduERP - Educational ERP Desktop Application & Services

**EduERP** is a multi-tier, distributed Enterprise Resource Planning (ERP) simulation desktop application and backend suite. It is designed to simulate a real-world ERP system for educational settings, specifically tailored for Belgian secondary schools.

## 🏗️ Architecture & Component Overview

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
                                      │ HTTPS (TLS 1.3) / WebSocket (WSS)
                                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                           APPLICATION & DATA TIER                           │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                    Backend Application Services                     │   │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐ │   │
│  │  │   REST API  │  │  WebSocket  │  │  PostgreSQL │  │    Local    │ │   │
│  │  │   (Go/Gin)  │  │ (Gorilla WS)│  │ Database 17 │  │   Storage   │ │   │
│  │  └─────────────┘  └─────────────┘  └─────────────┘  └─────────────┘ │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────────────┘
```

The system is split into two primary components:
1. **Desktop Client**: A modern, reactive GUI built with **Qt6 (QML)** and **C++20**, caching data locally via an offline-first **SQLite** engine.
2. **Backend Server**: A lightweight, high-performance API and real-time synchronization server built with **Go (Gin)** and **Gorilla WebSockets**, backed by a multi-tenant **PostgreSQL 17** database.

---

## 🌟 Key Features

* **Multi-Tenant Isolation**: High-security architecture separating school data at the database level using Tenant Isolation patterns.
* **Offline-First Synchronization**: Uses a local SQLite cache and background task queue to support seamless operation during network instability, syncing back to the cloud via exponential backoff reconnect strategies.
* **Real-time Collaboration**: WebSocket-based event streaming to push instant notifications, lock input fields during active editing by team members, and sync state changes across client machines.
* **Core ERP Modules**:
  * **Dashboard**: Key Performance Indicator (KPI) aggregator.
  * **Finance**: Ledger tracking, transactions, budgets, invoicing.
  * **Inventory**: Stock level management, ordering processes.
  * **HR**: User profiles, roles (CEO, CFO, Teacher, Student), and permission management.
  * **Sales & Marketing**: CRM, campaigns, order placement.
  * **Collaboration Chat**: Integrated multi-channel real-time team chat.

---

## 🛠️ Tech Stack

### Client Tier
* **C++20**: Core business logic, synchronization engine, storage manager, and networking.
* **Qt6 / QML**: Reactive visual layout, smooth desktop animations, and user forms.
* **SQLite 3**: Robust, encrypted local SQL cache.
* **Vcpkg**: C++ package management.

### Backend Tier
* **Go (Golang)**: Microservices and API endpoints.
* **Gin Gonic**: HTTP web server and REST router.
* **Gorilla WebSocket**: Multi-tenant real-time messaging gateway.
* **PostgreSQL 17**: Main persistent relational storage.
* **Docker / Docker Compose**: Standardized developer environments and container builds.

---

## 🚀 Getting Started

### Prerequisites
* **CMake 3.20+**
* **Go 1.21+**
* **Qt 6.6+**
* **C++20 Compatible Compiler** (MSVC 2022 / GCC 11 / Clang 14)
* **Docker** & **Docker Compose** (for running the database and backend services)

### Running the Backend Server
1. Navigate to the `server/` directory:
   ```bash
   cd server
   ```
2. Start the database and backend services using Docker Compose:
   ```bash
   docker-compose up -d
   ```
3. Run the Go server:
   ```bash
   go run main.go
   ```

### Building the Desktop Application
1. Initialize the package manager dependencies:
   ```bash
   vcpkg install
   ```
2. Create and enter the build directory:
   ```bash
   mkdir build && cd build
   ```
3. Configure the CMake project:
   ```bash
   cmake ..
   ```
4. Build the application:
   ```bash
   cmake --build . --config Release
   ```
5. Launch the executable from the build folder.

---

## 📄 License
This project is licensed under the MIT License.
