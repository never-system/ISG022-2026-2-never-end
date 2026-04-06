import DiceBox from "@3d-dice/dice-box-threejs";
import { initializeDiceBox, randomColor } from ".";
import { DiceType } from "./models";
import { useEffect, useState } from "react";

export function DiceRoll() {
  console.log("DiceRoll component is rendering");

  const dice: DiceType = DiceType.D20;
  const [diceBox, setDiceBox] = useState<DiceBox | null>(null);

  useEffect(() => {
    const init = async () => {
      console.log("useEffect triggered");
      if (!diceBox) {
        console.log("initializing dice-box");
        setTimeout(async () => {
          const box = await initializeDiceBox("#dice-box");
          setDiceBox(box);
        }, 500);
      }
    };
    init();
  }, [diceBox]);

  const rollingDice = (
    quantity: number,
    dice: DiceType,
    preResults: number[] = [],
  ) => {
    const quantityResults = preResults.length;
    console.log(
      "rollingDice called with quantity:",
      quantity,
      "preResults:",
      preResults,
    );
    if (quantityResults > 0 && quantityResults !== quantity) {
      return;
    }
    if (diceBox) {
      diceBox.updateConfig({
        theme_customColorset: {
          background: randomColor,
          foreground: "#ffffff",
          texture: "ice", // marble | ice
          material: "wood", // metal | glass | plastic | wood
        },
      });
      console.log("rolling dice...");
      diceBox.roll(`${quantity}${dice}@${preResults.join(",")}`);
    }
  };

  return (
    <>
      <button onClick={() => rollingDice(5, dice, Array(5).fill(21))}>
        Roll
      </button>
      <div
        id="dice-box"
        style={{ width: "400px", height: "400px", border: "1px solid black" }}
      ></div>
    </>
  );
}
