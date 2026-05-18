import { useState, useEffect, useCallback, useRef } from "react";
import { useMqttConnection } from "@/hooks/useMqttConnection";
import { useDiceBox } from "@/hooks/useDiceBox";

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

const useDesktopPreferences = () => {
  const [isDesktop, setIsDesktop] = useState(() =>
    window.matchMedia("(min-width: 1024px) and (hover: hover)").matches,
  );

  useEffect(() => {
    const mq = window.matchMedia("(min-width: 1024px) and (hover: hover)");
    const handler = (e: MediaQueryListEvent) => setIsDesktop(e.matches);
    mq.addEventListener?.("change", handler);
    return () => mq.removeEventListener?.("change", handler);
  }, []);
  return isDesktop;
};

export function MqttDiceListener() {
  const { isConnected, isConnecting, error, subscribe } = useMqttConnection();
  const { diceBox, isInitializing, initialize } = useDiceBox();
  const [history, setHistory] = useState<Roll[]>([]);
  const [rolling, setRolling] = useState(false);
  const [current, setCurrent] = useState<DicePayload | null>(null);
  const [queue, setQueue] = useState(0);

  const isDesktop = useDesktopPreferences();

  const qRef = useRef<DicePayload[]>([]);
  const boxRef = useRef(diceBox);
  const rollRef = useRef(rolling);

  const processRoll = useCallback(async (p: DicePayload) => {
    let box = boxRef.current,
      i = 0;
    while (!box && i < 10) {
      await new Promise((r) => setTimeout(r, 200));
      box = boxRef.current;
      i++;
    }
    if (!box) return;
    setCurrent(p);
    setRolling(true);
    try {
      box.updateConfig({
        theme_customColorset: {
          background: `hsl(${(Date.now() % 40) + 200},75%,12%)`,
          foreground: "#ffd7a0",
          texture: "stardust",
          material: "moonlight",
        },
      });
      await box.roll(`1${p.dice}@${p.result}`);
      setHistory((h) =>
        [
          { id: `${Date.now()}-${Math.random()}`, ...p, timestamp: new Date() },
          ...h,
        ].slice(0, 50),
      );
      setTimeout(() => {
        setRolling(false);
        setCurrent(null);
      }, 1100);
    } catch {
      setRolling(false);
      setCurrent(null);
    }
  }, []);

  useEffect(() => {
    boxRef.current = diceBox;
  }, [diceBox]);
  useEffect(() => {
    rollRef.current = rolling;
  }, [rolling]);
  useEffect(() => {
    if (!diceBox && !isInitializing) initialize().catch(() => {});
  }, [diceBox, isInitializing, initialize]);

  useEffect(() => {
    if (!isConnected) return;
    return subscribe("dice/roll", (p: DicePayload) => {
      if (!boxRef.current || rollRef.current) {
        qRef.current.push(p);
        setQueue(qRef.current.length);
      } else processRoll(p);
    });
  }, [isConnected, processRoll, subscribe]);

  useEffect(() => {
    if (diceBox && !isInitializing && qRef.current[0] && !rolling) {
      const next = qRef.current.shift();
      setQueue(qRef.current.length);
      if (next) processRoll(next);
    }
  }, [diceBox, isInitializing, processRoll, rolling]);

  useEffect(() => {
    if (!isDesktop) return;
    const handler = (e: KeyboardEvent) => {
      if (e.key.toLowerCase() === "r" && history[0]) {
        const last = history[0];
        qRef.current.push({
          dice: last.dice,
          result: last.result,
          player: last.player,
        });
        setQueue(qRef.current.length);
      }
      if (e.key.toLowerCase() === "c") {
        setHistory([]);
      }
    };
    window.addEventListener("keydown", handler);
    return () => window.removeEventListener("keydown", handler);
  }, [isDesktop, history]);

  const stats =
    history.length > 0
      ? {
          total: history.length,
          average: Math.round(
            history.reduce((a, b) => a + (parseInt(b.result) || 0), 0) /
              history.length,
          ),
          last: history[0]?.result ?? "-",
        }
      : { total: 0, average: 0, last: "-" };

  const conn = isConnected
    ? { e: "🧚", t: "Pó de Fada", c: "text-amber-300" }
    : isConnecting
      ? { e: "🌙", t: "Viajando...", c: "text-sky-300" }
      : { e: "🏴‍☠️", t: "Ilha Perdida", c: "text-rose-300" };

  return (
    <>
      <style>{`
        @keyframes twinkle{0%,100%{opacity:.2;transform:scale(1)}50%{opacity:1;transform:scale(1.2)}}
        @keyframes rise{to{transform:translateY(-100vh) scale(.5);opacity:0}}
        @keyframes glow{0%,100%{box-shadow:0 0 20px rgba(255,215,160,.1)}50%{box-shadow:0 0 40px rgba(255,215,160,.3)}}
        @keyframes draw{to{stroke-dashoffset:0}}
        @keyframes float{0%,100%{transform:translateY(0)}50%{transform:translateY(-8px)}}

        :root{--gold:#ffd7a0;--gold-dim:rgba(255,215,160,.15);--paper:#f5e6ca;--ink:#5a3921}

        .never{background:
          radial-gradient(1200px 600px at 50% -10%,#2a2a4a20,transparent 70%),
          radial-gradient(800px 400px at 100% 0%,#4a3a6a15,transparent 60%),
          #0a0a12;
          position:fixed;inset:0;pointer-events:none;z:0}
        .never::before{content:"";position:absolute;inset:0;
          background:radial-gradient(2px 2px at 20% 30%,var(--gold) 99%,transparent),
                     radial-gradient(2px 2px at 80% 20%,var(--gold) 99%,transparent),
                     radial-gradient(1px 1px at 40% 60%,var(--gold) 99%,transparent),
                     radial-gradient(1px 1px at 60% 80%,var(--gold) 99%,transparent),
                     radial-gradient(1.5px 1.5px at 15% 75%,var(--gold) 99%,transparent),
                     radial-gradient(1.5px 1.5px at 85% 45%,var(--gold) 99%,transparent);
          background-repeat:repeat;animation:twinkle 3s infinite ease-in-out;opacity:.6}

        @media(min-width:1024px){
          .never{
            background:
              radial-gradient(1800px 900px at 50% -20%,#2a2a4a30,transparent 75%),
              radial-gradient(1200px 600px at 100% 10%,#4a3a6a20,transparent 65%),
              radial-gradient(800px 400px at 0% 80%,#3a4a6a15,transparent 70%),
              #0a0a12;
          }
          .never::after{
            content:"";position:absolute;inset:0;
            background:
              radial-gradient(1px 1px at 10% 15%,var(--gold) 99%,transparent),
              radial-gradient(1px 1px at 25% 85%,var(--gold) 99%,transparent),
              radial-gradient(1px 1px at 45% 25%,var(--gold) 99%,transparent),
              radial-gradient(1px 1px at 70% 60%,var(--gold) 99%,transparent),
              radial-gradient(1px 1px at 90% 40%,var(--gold) 99%,transparent);
            animation:twinkle 4s infinite ease-in-out;opacity:.4;pointer-events:none
          }
        }

        .spark{position:absolute;width:2px;height:2px;background:var(--gold);border-radius:50%;animation:rise 6s linear forwards;opacity:.8;pointer-events:none}
        .glass{background:linear-gradient(135deg,#1a1a2ecc,#0f0f1acc);backdrop-filter:blur(12px);border:1px solid var(--gold-dim)}
        .paper{background:linear-gradient(145deg,var(--paper),#e8d4a8,#d4c196);box-shadow:inset 0 1px 0 #fff8,inset 0 -1px 0 #0002,0 4px 20px #5a392120}
        .reveal{animation:draw .5s ease-out;text-shadow:0 0 25px var(--gold),0 0 50px var(--gold-dim)}
        .hover-glow:hover{animation:glow 2s ease-in-out infinite;transform:translateY(-1px);transition:transform .2s}
        .scroll{scrollbar-width:thin;scrollbar-color:#d4a01740 transparent}
        .scroll::-webkit-scrollbar{width:4px}.scroll::-webkit-scrollbar-track{background:transparent}
        .scroll::-webkit-scrollbar-thumb{background:linear-gradient(#d4a017,#8b6f47);border-radius:2px}

        @media(hover:hover){
          .interactive{cursor:pointer;transition:all .2s ease}
          .interactive:hover{transform:translateX(4px);filter:brightness(1.05);box-shadow:0 4px 12px rgba(0,0,0,.15)}
        }
        @media(prefers-reduced-motion){*{animation:none!important;transition:none!important}}
      `}</style>

      <main className="never min-h-screen text-[#e8d4a8] font-serif overflow-hidden relative">
        <div
          className="fixed inset-0 pointer-events-none z-10"
          onClick={(e) => {
            if (Math.random() > 0.8) {
              const s = document.createElement("div");
              s.className = "spark";
              s.style.left = e.clientX + "px";
              s.style.top = e.clientY + "px";
              document.body.appendChild(s);
              setTimeout(() => s.remove(), 6000);
            }
          }}
        />

        <header className="sticky top-0 z-30 glass border-b border-[var(--gold-dim)]">
          <div className="max-w-7xl mx-auto px-4 lg:px-8 py-3 flex items-center justify-between">
            <div className="flex items-center gap-3">
              <span className={`text-xl lg:text-2xl animate-pulse ${conn.c}`}>
                {conn.e}
              </span>
              <div className="leading-tight">
                <div className="font-bold tracking-wider text-[var(--gold)] text-base lg:text-lg">
                  Never Dice
                </div>
                <div className="text-[10px] lg:text-xs uppercase tracking-widest opacity-70">
                  {conn.t}
                </div>
              </div>
            </div>
            <div className="flex items-center gap-3">
              {queue > 0 && (
                <span className="text-xs lg:text-sm text-amber-300 bg-[var(--gold-dim)] px-3 py-1.5 rounded-full border border-[var(--gold)]/30 animate-pulse">
                  {queue}✨ aguardando
                </span>
              )}
              {isDesktop && (
                <span className="hidden lg:inline text-[10px] text-[var(--gold)]/50 font-mono">
                  [R] repetir • [C] limpar
                </span>
              )}
            </div>
          </div>
        </header>

        <div className="max-w-7xl mx-auto px-4 lg:px-8 py-6 lg:py-8">
          <div className="grid grid-cols-1 lg:grid-cols-12 gap-6 lg:gap-8 items-start">
            <section className="lg:col-span-8 flex justify-center">
              <div
                className="relative w-full max-w-sm lg:max-w-md xl:max-w-lg aspect-square
                            rounded-2xl lg:rounded-3xl overflow-hidden border border-[var(--gold-dim)]
                            glass hover-glow transition-all shadow-2xl shadow-[var(--gold-dim)]/50"
              >
                <div className="absolute inset-4 lg:inset-6">
                  <div
                    id="dice-box"
                    className="w-full h-full rounded-xl lg:rounded-2xl border border-[var(--gold-dim)] bg-[#0f0f1a]"
                    aria-label="Canvas 3D de rolagem de dados"
                  />
                </div>

                {current && !rolling && (
                  <div
                    className="absolute inset-0 flex items-center justify-center z-10
                                bg-[#0f0f1a]/90 backdrop-blur-md"
                  >
                    <div
                      className="text-center p-6 lg:p-8 rounded-2xl paper max-w-[95%] lg:max-w-md
                                  border-2 border-[var(--gold)]/30 shadow-2xl shadow-[var(--gold-dim)]
                                  animate-[glow_2s_ease-in-out_infinite]"
                    >
                      <div className="text-[var(--ink)]/60 text-[10px] lg:text-xs tracking-[.4em] uppercase mb-2">
                        ✦ Destino Revelado ✦
                      </div>

                      <div className="text-[var(--ink)] font-bold text-lg lg:text-xl mb-1">
                        {current.player}
                      </div>

                      <div className="text-[var(--ink)]/50 text-sm italic mb-3 font-mono">
                        {current.dice}
                      </div>

                      <div
                        className="reveal text-6xl lg:text-7xl font-black text-[var(--ink)]
                                    drop-shadow-[0_0_35px_rgba(255,215,160,0.6)]"
                      >
                        {current.result}
                      </div>

                      <div className="text-[var(--ink)]/40 text-xs mt-4 tracking-widest">
                        · · · ✧ · · ·
                      </div>
                    </div>
                  </div>
                )}

                <div className="absolute inset-0 rounded-2xl lg:rounded-3xl border border-[var(--gold)]/5 pointer-events-none" />
              </div>
            </section>

            <aside className="lg:col-span-4 w-full space-y-4">
              <div className="paper rounded-xl border-2 border-[#8b6f47]/30 overflow-hidden hover-glow transition-all">
                <div className="px-4 py-3 border-b border-[#8b6f47]/20 flex justify-between items-center bg-[var(--paper)]/40">
                  <span className="text-[var(--ink)] font-bold text-sm tracking-wide">
                    📜 Crônicas
                  </span>
                  <div className="flex gap-1">
                    <button
                      onClick={() => setHistory([])}
                      className="text-[var(--ink)]/60 hover:text-[var(--ink)] text-xs transition"
                      title="Limpar histórico"
                    >
                      🗑️
                    </button>
                  </div>
                </div>
                <div className="max-h-72 lg:max-h-96 overflow-y-auto scroll p-3 space-y-2">
                  {!history[0] ? (
                    <div className="text-center py-8 text-[var(--ink)]/50 text-xs italic">
                      <div className="text-3xl mb-2 animate-[float_3s_ease-in-out_infinite]">
                        🗝️
                      </div>
                      <p>Páginas em branco</p>
                      <p className="opacity-70 mt-1">aguardando magia</p>
                    </div>
                  ) : (
                    history.map((r) => (
                      <div
                        key={r.id}
                        className="interactive p-3 rounded-lg bg-[var(--paper)]/40 hover:bg-[var(--paper)]/70
                                  transition border-l-2 border-[#8b6f47]/40 hover:border-[var(--ink)]"
                      >
                        <div className="flex justify-between items-start">
                          <span className="text-[var(--ink)] font-semibold text-sm truncate max-w-[120px]">
                            {r.player}
                          </span>
                          <span className="text-[var(--ink)]/50 text-[10px] font-mono whitespace-nowrap ml-2">
                            {r.timestamp.toLocaleTimeString("pt-BR", {
                              hour: "2-digit",
                              minute: "2-digit",
                            })}
                          </span>
                        </div>
                        <div className="flex justify-between items-end mt-2">
                          <span className="text-[var(--ink)]/60 text-xs font-mono">
                            {r.dice}
                          </span>
                          <span className="text-xl lg:text-2xl font-black text-[#2d5016]">
                            {r.result}
                          </span>
                        </div>
                      </div>
                    ))
                  )}
                </div>
                <div className="h-1 bg-gradient-to-r from-[#8b6f47]/20 via-[var(--gold)]/30 to-[#8b6f47]/20" />
              </div>

              {isDesktop && (
                <div className="paper rounded-xl border-2 border-[#8b6f47]/30 p-4 hidden lg:block">
                  <h4 className="text-[var(--ink)] font-bold text-sm mb-3 flex items-center gap-2">
                    📊 Estatísticas
                    <span className="text-[10px] font-normal text-[var(--ink)]/50 bg-[var(--gold-dim)] px-1.5 py-0.5 rounded">
                      sessão
                    </span>
                  </h4>
                  <div className="grid grid-cols-3 gap-2 text-xs">
                    <div className="bg-[var(--paper)]/40 rounded-lg p-2.5 text-center border border-[#8b6f47]/20">
                      <div className="text-[var(--ink)]/60 text-[10px] uppercase tracking-wide">
                        Total
                      </div>
                      <div className="font-bold text-[var(--ink)] text-lg">
                        {stats.total}
                      </div>
                    </div>
                    <div className="bg-[var(--paper)]/40 rounded-lg p-2.5 text-center border border-[#8b6f47]/20">
                      <div className="text-[var(--ink)]/60 text-[10px] uppercase tracking-wide">
                        Média
                      </div>
                      <div className="font-bold text-[var(--ink)] text-lg">
                        {stats.average || "–"}
                      </div>
                    </div>
                    <div className="bg-[var(--paper)]/40 rounded-lg p-2.5 text-center border border-[#8b6f47]/20">
                      <div className="text-[var(--ink)]/60 text-[10px] uppercase tracking-wide">
                        Último
                      </div>
                      <div className="font-bold text-[var(--ink)] text-lg">
                        {stats.last}
                      </div>
                    </div>
                  </div>
                </div>
              )}

              {isDesktop && (
                <div className="hidden lg:block text-[10px] text-[var(--ink)]/50 text-center italic bg-[var(--gold-dim)]/30 rounded-lg py-2 px-3 border border-[var(--gold)]/20">
                  <span className="bg-[var(--gold-dim)] px-2 py-0.5 rounded font-mono text-[var(--ink)]">
                    R
                  </span>{" "}
                  repetir último •
                  <span className="bg-[var(--gold-dim)] px-2 py-0.5 rounded font-mono text-[var(--ink)] ml-1">
                    C
                  </span>{" "}
                  limpar
                </div>
              )}
            </aside>
          </div>

          {error && (
            <div className="max-w-7xl mx-auto px-4 lg:px-8 mt-4">
              <div
                className="p-3.5 rounded-lg bg-[#2a1a1a]/90 border border-[#ff6b6b]/40
                            text-[#ffb4b4] text-xs flex items-center gap-2 glass"
              >
                <span className="text-lg">🏴‍☠️</span>
                <span>{error.message}</span>
              </div>
            </div>
          )}
        </div>

        <footer
          className="fixed bottom-0 inset-x-0 text-center py-2.5 text-[10px] lg:text-xs
                          tracking-widest text-[var(--gold)]/40 uppercase pointer-events-none
                          glass border-t border-[var(--gold-dim)]"
        >
          <span className="hidden sm:inline">segunda estrela à direita • </span>
          reto até amanhã ✧
        </footer>
      </main>
    </>
  );
}
