declare module "@3d-dice/dice-box-threejs" {
  interface DiceRoll {
    type: string;
    sides: number;
    id: number;
    value: number;
    reason: string;
  }

  interface DiceSet {
    num: number;
    type: string;
    sides: number;
    rolls: DiceRoll[];
    total: number;
  }

  interface DiceResult {
    notation: string;
    sets: DiceSet[];
    modifier: number;
    total: number;
  }

  interface CustomColorSet {
    name?: string;
    category?: string;
    foreground?: string | string[];
    background?: string | string[];
    outline?: string;
    texture?: string | string[];
    material?: string;
    description?: string;
    edge?: string | string[];
  }

  interface DiceBoxOptions {
    framerate?: number;
    sounds?: boolean;
    volume?: number;
    color_spotlight?: number;
    shadows?: boolean;
    theme_surface?: string;
    sound_dieMaterial?: string;
    theme_customColorset?: CustomColorSet | null;
    theme_colorset?: string;
    theme_texture?: string;
    theme_material?: string;
    gravity_multiplier?: number;
    light_intensity?: number;
    baseScale?: number;
    strength?: number;
    onRollComplete?: (result: DiceResult) => void;
    onRerollComplete?: (result: DiceResult[]) => void;
    onAddDiceComplete?: (result: DiceResult[]) => void;
    onRemoveDiceComplete?: (result: DiceResult[]) => void;
  }

  export default class DiceBox {
    constructor(container: string, options?: DiceBoxOptions);
    initialize(): Promise<void>;
    roll(dice: string): Promise<DiceResult>;
    updateConfig(options: Partial<DiceBoxOptions>): void;
  }
}
