import DiceBox from "@3d-dice/dice-box-threejs";

const diceBoxConfig = {
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
  onRollComplete: () => undefined,
};

export async function createDiceBox(containerSelector: string): Promise<DiceBox> {
  const diceBox = new DiceBox(containerSelector, diceBoxConfig);
  await diceBox.initialize();
  return diceBox;
}
