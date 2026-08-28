import mqtt from "mqtt";

const MQTT_PROTOCOL = (process.env.MQTT_PROTOCOL || "mqtts") as
  | "mqtts"
  | "mqtt";

const options = {
  protocol: MQTT_PROTOCOL,
  host: process.env.MQTT_HOST,
  port: Number(process.env.MQTT_PORT) || 8883,
  username: process.env.MQTT_USERNAME,
  password: process.env.MQTT_PASSWORD,
  rejectUnauthorized: process.env.NODE_ENV === "production" ? true : false,
  clientId: "never_dice_api",
};
export const mqttClient = mqtt.connect(options);

export const getClientId = () => mqttClient.options?.clientId;

const clientId = getClientId();

const log = (message: string) => {
  mqttClient.publish(`logs/${clientId}`, message);
};

mqttClient.on("connect", () => {
  console.log("Connected to MQTT broker");
  mqttClient.subscribe("#");
  log("Conected");
});

mqttClient.on("message", (topic, message) => {
  if (topic === `logs/${clientId}`) return;
  console.log(`Received message on topic ${topic}: ${message}`);
  log(`Received message on topic ${topic}: ${message}`);
});
