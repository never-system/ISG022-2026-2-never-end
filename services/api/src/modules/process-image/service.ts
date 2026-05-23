import { S3Client } from "bun";
import OpenAI from "openai";
import { ProcessImageResponse } from "./model";

export class ProcessImageService {
  private openAIclient: OpenAI;
  private s3Client: S3Client;

  constructor(openAIclient: OpenAI, s3Client: S3Client) {
    this.openAIclient = openAIclient;
    this.s3Client = s3Client;
  }

  async uploadImage(image: File): Promise<string> {
    console.log("Uploading image to S3...");
    const buffer = await image.arrayBuffer();

    const key = `uploads/${Date.now()}-${image.name}`;
    const s3File = this.s3Client.file(key);

    try {
      await s3File.write(buffer, {
        type: image.type,
        contentDisposition: `inline; filename="${encodeURIComponent(image.name)}"`,
      });
      console.log("Image uploaded to S3 with key:", key);
    } catch (error) {
      console.error("Failed to upload image to S3:", error);
      throw error;
    }

    const imageUrl = s3File.presign({
      method: "GET",
      expiresIn: 3600,
      acl: "public-read",
    });

    console.log(imageUrl);

    return imageUrl;
  }

  async processImage(image: File): Promise<ProcessImageResponse> {
    const image_url = await this.uploadImage(image);
    const response = await this.openAIclient.responses.create({
      model: "gpt-4o-mini",
      input: [
        {
          role: "user",
          content: [
            {
              type: "input_text",
              text: `
              You are a specialized RPG dice reader. Analyze the image showing the top face of a die and respond ONLY with a valid JSON in the format: {"type": "dX", "value": N}. Do not include code blocks or any additional text—only raw JSON. Consider the perspective and orientation of the image, ignoring all side faces. Treat the image strictly as the top face of an RPG die, where "type" represents the die type (d4, d6, d8, d10, d12, or d20) and "value" is the number displayed on the top face. If the image is not clear enough to determine the die type or value, respond with {"type": "d20", "value": 0}.
              `,
            },
            {
              type: "input_image",
              image_url: image_url,
              detail: "high",
            },
          ],
        },
      ],
    });
    const parsedResult = JSON.parse(
      response.output_text,
    ) as ProcessImageResponse;
    return parsedResult;
  }
}
