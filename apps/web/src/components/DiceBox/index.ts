import DiceBox, { type DiceResult } from "@3d-dice/dice-box-threejs";

const defaultConfig = {
  framerate: 1 / 100,
  sounds: true,
  volume: 100,
  color_spotlight: 0xefdfd5,
  shadows: true,
  theme_surface: "blue-felt",
  sound_dieMaterial: "plastic",
  theme_customColorset: null,
  theme_colorset: "white",
  theme_texture: "blue-felt",
  theme_material: "glass",
  gravity_multiplier: 400,
  light_intensity: 0.7,
  baseScale: 100,
  strength: 1,
  onRollComplete: (result: DiceResult) => {
    console.log("Roll result:", result);
  },
};

export async function initializeDiceBox(containerId: string) {
  const Box = new DiceBox(containerId, defaultConfig);

  // Chame initialize() para configurar o renderer (como no exemplo)
  await Box.initialize();

  console.log(`DiceBox initialized with selector: ${containerId}`);
  return Box;
}

const colors = [
  "#00ffcb",
  "#ff6600",
  "#1d66af",
  "#7028ed",
  "#c4c427",
  "#d81128",
];

export const randomColor = colors[Math.floor(Math.random() * colors.length)];
