import { useCallback, useEffect, useRef, useState } from "react";
import mqtt from "mqtt";
import type { IClientOptions } from "mqtt";
import {
  FALLBACK_BROKERS,
  MQTT_CONFIG,
  PRIMARY_BROKER,
} from "@/config/mqtt.config";

type MqttClient = ReturnType<typeof mqtt.connect>;

interface UseMqttConnectionReturn {
  isConnected: boolean;
  isConnecting: boolean;
  error: Error | null;
  subscribe: <T>(topic: string, callback: (payload: T) => void) => () => void;
}

export function useMqttConnection(): UseMqttConnectionReturn {
  const [isConnected, setIsConnected] = useState(false);
  const [isConnecting, setIsConnecting] = useState(false);
  const [error, setError] = useState<Error | null>(null);
  const clientRef = useRef<MqttClient | null>(null);
  const subscriptionsRef = useRef<Map<string, (payload: unknown) => void>>(
    new Map(),
  );

  useEffect(() => {
    let disposed = false;
    let timeoutId: ReturnType<typeof setTimeout> | null = null;
    const brokers = [PRIMARY_BROKER, ...FALLBACK_BROKERS];

    const connect = (brokerIndex: number) => {
      if (disposed) return;

      if (brokerIndex >= brokers.length) {
        setError(
          new Error(
            "Nenhum broker MQTT disponível. Verifique sua conexão e tente novamente.",
          ),
        );
        setIsConnecting(false);
        return;
      }

      const config = brokers[brokerIndex];
      const connectionUrl = `${config.protocol}://${config.host}:${config.port}`;
      const connectOptions: IClientOptions = {
        username: config.username,
        password: config.password,
        reconnectPeriod: 0,
        connectTimeout: MQTT_CONFIG.connectTimeout,
        keepalive: MQTT_CONFIG.keepalive,
        protocolVersion: MQTT_CONFIG.protocolVersion,
        clientId: `never-dice-${Math.random().toString(16).slice(2, 8)}`,
      };

      setIsConnecting(true);
      setError(null);

      const client = mqtt.connect(connectionUrl, connectOptions);
      clientRef.current = client;

      const tryNextBroker = () => {
        if (disposed || clientRef.current !== client) return;
        client.end(true);
        clientRef.current = null;
        setIsConnected(false);
        connect(brokerIndex + 1);
      };

      timeoutId = setTimeout(tryNextBroker, MQTT_CONFIG.connectTimeout + 1000);

      client.on("connect", () => {
        if (timeoutId) clearTimeout(timeoutId);
        setIsConnected(true);
        setIsConnecting(false);
        setError(null);
        subscriptionsRef.current.forEach((_callback, topic) => {
          client.subscribe(topic, { qos: MQTT_CONFIG.qos });
        });
      });

      client.on("close", () => {
        if (!disposed) setIsConnected(false);
      });

      client.on("error", tryNextBroker);

      client.on("message", (topic: string, message: Buffer) => {
        const callback = subscriptionsRef.current.get(topic);
        if (!callback) return;

        try {
          callback(JSON.parse(message.toString()));
        } catch {
          setError(new Error(`Mensagem MQTT inválida em ${topic}.`));
        }
      });
    };

    connect(0);

    return () => {
      disposed = true;
      if (timeoutId) clearTimeout(timeoutId);
      clientRef.current?.end(true);
      clientRef.current = null;
      setIsConnected(false);
    };
  }, []);

  const subscribe = useCallback(
    <T,>(topic: string, callback: (payload: T) => void) => {
      subscriptionsRef.current.set(topic, callback as (payload: unknown) => void);
      clientRef.current?.subscribe(topic, { qos: MQTT_CONFIG.qos });

      return () => {
        subscriptionsRef.current.delete(topic);
        clientRef.current?.unsubscribe(topic);
      };
    },
    [],
  );

  return {
    isConnected,
    isConnecting,
    error,
    subscribe,
  };
}
