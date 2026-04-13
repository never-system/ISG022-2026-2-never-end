import { t, UnwrapSchema } from "elysia";

const ProcessImageBody = t.Object({
  pulses: t.String(),
  rollTime: t.String(),
  entropy: t.String(),
  dice: t.String(),
  source: t.String(),
  photo: t.File(),
});

const openApiResult = t.Object({
  type: t.String(),
  value: t.Number(),
});

type ProcessImageRequest = UnwrapSchema<typeof ProcessImageBody>;
type ProcessImageResponse = UnwrapSchema<typeof openApiResult>;

export {
  ProcessImageBody,
  openApiResult,
  type ProcessImageRequest,
  type ProcessImageResponse,
};
