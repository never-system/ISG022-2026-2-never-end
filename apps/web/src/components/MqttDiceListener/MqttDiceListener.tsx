import { useCallback, useEffect, useRef, useState } from "react";
import { useDiceBox } from "@/hooks/useDiceBox";
import { useMqttConnection } from "@/hooks/useMqttConnection";

interface DicePayload {
  dice: string;
  result: string;
  player: string;
}

interface Roll {
  id: string;
  player: string;
  dice: string;
  result: string;
  timestamp: Date;
}

const MAX_HISTORY_ITEMS = 50;

export function MqttDiceListener() {
  const { isConnected, isConnecting, error, subscribe } = useMqttConnection();
  const { diceBox, isInitializing, initialize } = useDiceBox();
  const [history, setHistory] = useState<Roll[]>([]);
  const [rolling, setRolling] = useState(false);
  const [currentRoll, setCurrentRoll] = useState<DicePayload | null>(null);
  const [queueSize, setQueueSize] = useState(0);

  const queueRef = useRef<DicePayload[]>([]);
  const diceBoxRef = useRef(diceBox);
  const rollingRef = useRef(rolling);

  const processRoll = useCallback(async (payload: DicePayload) => {
    let box = diceBoxRef.current;
    let attempts = 0;

    while (!box && attempts < 10) {
      await new Promise((resolve) => setTimeout(resolve, 200));
      box = diceBoxRef.current;
      attempts++;
    }

    if (!box) return;

    setCurrentRoll(payload);
    setRolling(true);

    try {
      box.updateConfig({
        theme_customColorset: {
          background: "#082f63",
          foreground: "#f7d779",
          outline: "#f8fafc",
          edge: "#d4a64c",
          texture: "stars",
          material: "glass",
        },
      });

      await box.roll(`1${payload.dice}@${payload.result}`);

      setHistory((items) =>
        [
          {
            id: `${Date.now()}-${Math.random()}`,
            ...payload,
            timestamp: new Date(),
          },
          ...items,
        ].slice(0, MAX_HISTORY_ITEMS),
      );
    } finally {
      window.setTimeout(() => {
        setRolling(false);
        setCurrentRoll(null);
      }, 1000);
    }
  }, []);

  useEffect(() => {
    diceBoxRef.current = diceBox;
  }, [diceBox]);

  useEffect(() => {
    rollingRef.current = rolling;
  }, [rolling]);

  useEffect(() => {
    if (!diceBox && !isInitializing) initialize().catch(() => undefined);
  }, [diceBox, initialize, isInitializing]);

  useEffect(() => {
    if (!isConnected) return;

    return subscribe<DicePayload>("dice/roll", (payload) => {
      if (!diceBoxRef.current || rollingRef.current) {
        queueRef.current.push(payload);
        setQueueSize(queueRef.current.length);
        return;
      }

      processRoll(payload);
    });
  }, [isConnected, processRoll, subscribe]);

  useEffect(() => {
    if (!diceBox || isInitializing || rolling || !queueRef.current[0]) return;

    const nextRoll = queueRef.current.shift();
    setQueueSize(queueRef.current.length);

    if (nextRoll) processRoll(nextRoll);
  }, [diceBox, isInitializing, processRoll, rolling]);

  useEffect(() => {
    const handler = (event: KeyboardEvent) => {
      if (event.key.toLowerCase() === "r" && history[0]) {
        const lastRoll = history[0];
        queueRef.current.push({
          dice: lastRoll.dice,
          result: lastRoll.result,
          player: lastRoll.player,
        });
        setQueueSize(queueRef.current.length);
      }

      if (event.key.toLowerCase() === "c") {
        setHistory([]);
      }
    };

    window.addEventListener("keydown", handler);
    return () => window.removeEventListener("keydown", handler);
  }, [history]);

  const stats = {
    total: history.length,
    average: history.length
      ? Math.round(
          history.reduce((sum, item) => sum + (parseInt(item.result, 10) || 0), 0) /
            history.length,
        )
      : 0,
    last: history[0]?.result ?? "-",
  };

  const connection = isConnected
    ? { label: "MQTT conectado", className: "is-online" }
    : isConnecting
      ? { label: "Conectando ao MQTT", className: "is-connecting" }
      : { label: "MQTT offline", className: "is-offline" };

  return (
    <main className="arcane-shell">
      <div className="ornate-frame" aria-hidden="true">
        <span />
        <span />
        <span />
        <span />
      </div>

      <section className="arcane-stage">
        <header className="arcane-title">
          <div className="title-ornament" aria-hidden="true" />
          <h1>NeverDice</h1>
          <div className="title-rule" aria-hidden="true" />
        </header>

        <section className="dice-orbit" aria-label="Area de rolagem de dados">
          <div className="moon" aria-hidden="true" />
          <div className="castle left" aria-hidden="true" />
          <div className="castle right" aria-hidden="true" />

          <div className="arena">
            <div
              id="dice-box"
              className="dice-canvas"
              aria-label="Canvas 3D de rolagem de dados"
            />

            {currentRoll && !rolling && (
              <div className="result-medallion" aria-live="polite">
                <span>Resultado</span>
                <strong>{currentRoll.result}</strong>
                <p>
                  {currentRoll.player} · {currentRoll.dice}
                </p>
              </div>
            )}
          </div>
        </section>

        <footer className="command-plaque">
          <div className={`mqtt-status ${connection.className}`}>
            <span aria-hidden="true" />
            {connection.label}
          </div>
          <div className="session-stats">
            <span>Total {stats.total}</span>
            <span>Media {stats.average || "-"}</span>
            <span>Ultimo {stats.last}</span>
          </div>
          {queueSize > 0 && <strong>{queueSize} na fila</strong>}
        </footer>
      </section>

      <aside className="chronicle-panel">
        <div className="chronicle-heading">
          <div>
            <p>Historico</p>
            <h2>Cronicas</h2>
          </div>
          <button type="button" onClick={() => setHistory([])}>
            Limpar
          </button>
        </div>

        <div className="chronicle-list">
          {!history[0] ? (
            <div className="empty-chronicle">
              <strong>Aguardando rolagens</strong>
              <span>Mensagens MQTT em dice/roll entram aqui.</span>
            </div>
          ) : (
            history.map((roll) => (
              <article className="chronicle-item" key={roll.id}>
                <div>
                  <strong>{roll.player}</strong>
                  <span>{roll.dice}</span>
                </div>
                <div>
                  <strong>{roll.result}</strong>
                  <time>
                    {roll.timestamp.toLocaleTimeString("pt-BR", {
                      hour: "2-digit",
                      minute: "2-digit",
                    })}
                  </time>
                </div>
              </article>
            ))
          )}
        </div>
      </aside>

      {error && (
        <div className="error-banner" role="alert">
          {error.message}
        </div>
      )}
    </main>
  );
}
