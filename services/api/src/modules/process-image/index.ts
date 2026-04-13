import { t, Elysia } from "elysia";
import { ProcessImageBody } from "./model";
import { ProcessImageService } from "./service";
import OpenAI from "openai";
import { S3Client } from "bun";
import { mqttClient } from "../../util/mqtt/mqtt";

export const processImage = (client: OpenAI, s3Client: S3Client) => {
  const service = new ProcessImageService(client, s3Client);

  return new Elysia().post(
    "/process-image",
    async ({ body }) => {
      const result = await service.processImage(body.photo);
      const resultJson = JSON.stringify({
        dice: result.type,
        result: result.value,
        player: body.source,
      });
      console.log(result);
      mqttClient.publish("dice/roll", resultJson);
      return result;
    },
    {
      beforeHandle({ body }) {
        if (!body.photo) {
          throw new Error("Image is required");
        }
        if (!body.source) {
          throw new Error("Source is required");
        }
      },
      body: ProcessImageBody,
    },
  );
};
