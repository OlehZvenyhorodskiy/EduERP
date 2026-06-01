package websocket

import (
	"encoding/json"
	"log"
	"net/http"
	"sync"
	"time"

	"github.com/gorilla/websocket"
)

var upgrader = websocket.Upgrader{
	ReadBufferSize:  1024,
	WriteBufferSize: 1024,
	CheckOrigin: func(r *http.Request) bool {
		return true // Desktop client — accept all origins
	},
}

// Message represents a WebSocket message per the spec.
type Message struct {
	Type    string          `json:"type"`
	Channel string          `json:"channel"`
	Data    json.RawMessage `json:"data"`
}

// Client represents a connected WebSocket client.
type Client struct {
	conn     *websocket.Conn
	send     chan []byte
	hub      *Hub
	userID   uint
	schoolID uint
	channels map[string]bool
	mu       sync.Mutex
}

// Hub manages all active WebSocket connections and channel subscriptions.
type Hub struct {
	clients    map[*Client]bool
	channels   map[string]map[*Client]bool
	broadcast  chan Message
	register   chan *Client
	unregister chan *Client
	mu         sync.RWMutex
}

// NewHub creates a new WebSocket hub.
func NewHub() *Hub {
	return &Hub{
		clients:    make(map[*Client]bool),
		channels:   make(map[string]map[*Client]bool),
		broadcast:  make(chan Message, 256),
		register:   make(chan *Client),
		unregister: make(chan *Client),
	}
}

// Run starts the hub's event loop.
func (h *Hub) Run() {
	for {
		select {
		case client := <-h.register:
			h.mu.Lock()
			h.clients[client] = true
			h.mu.Unlock()
			log.Printf("WebSocket: Client %d connected (total: %d)", client.userID, len(h.clients))

		case client := <-h.unregister:
			h.mu.Lock()
			if _, ok := h.clients[client]; ok {
				delete(h.clients, client)
				close(client.send)
				// Remove from all channels
				for ch := range client.channels {
					if subs, ok := h.channels[ch]; ok {
						delete(subs, client)
					}
				}
			}
			h.mu.Unlock()
			log.Printf("WebSocket: Client %d disconnected", client.userID)

		case msg := <-h.broadcast:
			h.mu.RLock()
			if subs, ok := h.channels[msg.Channel]; ok {
				raw, _ := json.Marshal(msg)
				for client := range subs {
					select {
					case client.send <- raw:
					default:
						close(client.send)
						delete(subs, client)
					}
				}
			}
			h.mu.RUnlock()
		}
	}
}

// Subscribe adds a client to a channel.
func (h *Hub) Subscribe(client *Client, channel string) {
	h.mu.Lock()
	defer h.mu.Unlock()
	if _, ok := h.channels[channel]; !ok {
		h.channels[channel] = make(map[*Client]bool)
	}
	h.channels[channel][client] = true
	client.channels[channel] = true
	log.Printf("WebSocket: Client %d subscribed to '%s'", client.userID, channel)
}

// BroadcastToChannel sends a message to all subscribers of a channel.
func (h *Hub) BroadcastToChannel(channel string, msgType string, data interface{}) {
	rawData, _ := json.Marshal(data)
	h.broadcast <- Message{
		Type:    msgType,
		Channel: channel,
		Data:    rawData,
	}
}

// HandleWebSocket upgrades HTTP to WebSocket and manages the connection lifecycle.
func HandleWebSocket(hub *Hub, userID uint, schoolID uint) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		conn, err := upgrader.Upgrade(w, r, nil)
		if err != nil {
			log.Printf("WebSocket upgrade error: %v", err)
			return
		}

		client := &Client{
			conn:     conn,
			send:     make(chan []byte, 256),
			hub:      hub,
			userID:   userID,
			schoolID: schoolID,
			channels: make(map[string]bool),
		}

		hub.register <- client

		go client.writePump()
		go client.readPump()
	}
}

func (c *Client) readPump() {
	defer func() {
		c.hub.unregister <- c
		c.conn.Close()
	}()

	c.conn.SetReadDeadline(time.Now().Add(60 * time.Second))
	c.conn.SetPongHandler(func(string) error {
		c.conn.SetReadDeadline(time.Now().Add(60 * time.Second))
		return nil
	})

	for {
		_, rawMsg, err := c.conn.ReadMessage()
		if err != nil {
			break
		}

		var msg Message
		if err := json.Unmarshal(rawMsg, &msg); err != nil {
			continue
		}

		switch msg.Type {
		case "subscribe":
			c.hub.Subscribe(c, msg.Channel)
		case "team:chat_message":
			c.hub.broadcast <- msg
		case "team:field_locked", "team:field_unlocked":
			c.hub.broadcast <- msg
		}
	}
}

func (c *Client) writePump() {
	ticker := time.NewTicker(30 * time.Second) // Heartbeat per spec
	defer func() {
		ticker.Stop()
		c.conn.Close()
	}()

	for {
		select {
		case message, ok := <-c.send:
			if !ok {
				c.conn.WriteMessage(websocket.CloseMessage, []byte{})
				return
			}
			c.conn.SetWriteDeadline(time.Now().Add(10 * time.Second))
			if err := c.conn.WriteMessage(websocket.TextMessage, message); err != nil {
				return
			}
		case <-ticker.C:
			c.conn.SetWriteDeadline(time.Now().Add(10 * time.Second))
			if err := c.conn.WriteMessage(websocket.PingMessage, nil); err != nil {
				return
			}
		}
	}
}
