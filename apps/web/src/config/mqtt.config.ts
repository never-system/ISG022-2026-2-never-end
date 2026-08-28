export interface MqttBrokerConfig {
  protocol: "wss" | "ws";
  host: string;
  port: number;
  username?: string;
  password?: string;
  name?: string;
}

export const PRIMARY_BROKER: MqttBrokerConfig = {
  protocol: "wss",
  host: "mqtt.astrum.app.br",
  port: 443,
  username: "esp32",
  password: "esp32",
  name: "Primary (astrum.app.br)",
};

export const FALLBACK_BROKERS: MqttBrokerConfig[] = [
  {
    protocol: "wss",
    host: "test.mosquitto.org",
    port: 8081,
    name: "Mosquitto Test",
  },
  {
    protocol: "wss",
    host: "broker.emqx.io",
    port: 8084,
    name: "EMQX Public",
  },
  {
    protocol: "wss",
    host: "broker.hivemq.com",
    port: 8884,
    name: "HiveMQ Public",
  },
];

export const MQTT_CONFIG = {
  reconnectPeriod: 5000,
  connectTimeout: 4000,
  keepalive: 60,
  protocolVersion: 4 as const,
  qos: 1 as const,
  topic: "dice/roll",
};
