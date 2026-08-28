import { Elysia } from "elysia";
import { processImage } from "./modules/process-image";
import OpenAI from "openai";
import { S3Client } from "bun";

const openAIclient = new OpenAI({
  apiKey: process.env.OPENAI_API_KEY,
});

const s3Client = new S3Client({
  accessKeyId: process.env.S3_ACCESS_KEY_ID,
  secretAccessKey: process.env.S3_SECRET_ACCESS_KEY,
  bucket: process.env.S3_BUCKET_NAME,
  endpoint: process.env.S3_ENDPOINT,
});

const app = new Elysia()
  .get("/", () => "Hello Elysia")
  .use(processImage(openAIclient, s3Client))
  .listen(3000);

console.log(
  `🦊 Elysia is running at ${app.server?.hostname}:${app.server?.port}`,
);
