import { useCallback, useRef, useState } from "react";
import DiceBox from "@3d-dice/dice-box-threejs";
import { createDiceBox } from "@/services/diceBox";

interface UseDiceBoxReturn {
  diceBox: DiceBox | null;
  isInitializing: boolean;
  initialize: () => Promise<DiceBox | null>;
}

export function useDiceBox(): UseDiceBoxReturn {
  const [diceBox, setDiceBox] = useState<DiceBox | null>(null);
  const [isInitializing, setIsInitializing] = useState(false);
  const initializationRef = useRef<Promise<DiceBox | null> | null>(null);

  const initialize = useCallback(async () => {
    if (diceBox) return diceBox;
    if (initializationRef.current) return initializationRef.current;

    initializationRef.current = (async () => {
      setIsInitializing(true);

      try {
        const box = await createDiceBox("#dice-box");
        setDiceBox(box);
        return box;
      } catch {
        return null;
      } finally {
        setIsInitializing(false);
        initializationRef.current = null;
      }
    })();

    return initializationRef.current;
  }, [diceBox]);

  return { diceBox, isInitializing, initialize };
}
