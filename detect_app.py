import time
import win32gui
import win32process
import psutil
import serial
from collections import defaultdict

FOCUSED_APPS = {
    "Code.exe",
    "notepad.exe",
    "WINWORD.EXE",
    "Notion.exe",
    "chrome.exe",
    "stm32cubeide.exe"
}

DISTRACTED_APPS = {
    "Discord.exe",
    "Spotify.exe",
    "steam.exe",
    "osu.exe",
    "VALORANT.exe",
    "steamwebhelper.exe",
    "cstrike.exe",
    "cs2.exe",
    "RainbowSix.exe",
    "Fortnite.exe",  
    "overwatch.exe"
}

#sampling + debounce
POLL_INTERVAL_SEC = 0.5
GRACE_PERIOD_SEC = 0.8

#TRUE: enable console logs
PRINT_EVENTS = True

#for debugging, leave at none for standard use
SUMMARY_EVERY_SEC = None

SERIAL_ENABLE = True
SERIAL_PORT = "COM3" #STM32 IS CONNECTED AT THIS PORT 
SERIAL_BAUD = 115200

ser = None
if SERIAL_ENABLE:
    ser = serial.Serial(SERIAL_PORT, SERIAL_BAUD, timeout=0.1)
    time.sleep(2)

    ser.reset_input_buffer()
    ser.reset_output_buffer()

#functions
def send_line(line: str):
    if not SERIAL_ENABLE or ser is None:
        return
    if not line.endswith("\n"):
        line += "\n"
    ser.write(line.encode("utf-8", errors="ignore"))

def get_foreground_app():
    hwnd = win32gui.GetForegroundWindow()
    if hwnd == 0:
        return None
    _, pid = win32process.GetWindowThreadProcessId(hwnd)
    try:
        proc = psutil.Process(pid)
        return proc.name()
    except psutil.NoSuchProcess:
        return None

def classify_app(app_name: str) -> str:
    if app_name in DISTRACTED_APPS:
        return "DISTRACTED"
    if app_name in FOCUSED_APPS:
        return "FOCUSED"
    return "UNKNOWN"

def now_s() -> float:
    return time.time()

def fmt_time(seconds: float) -> str:
    seconds = max(0.0, seconds)
    h = int(seconds // 3600)
    m = int((seconds % 3600) // 60)
    s = int(seconds % 60)
    if h > 0:
        return f"{h}h {m}m {s}s"
    if m > 0:
        return f"{m}m {s}s"
    return f"{s}s"

def read_stm32():
    try:
        line = ser.readline()
        if line:
            print("STM32:", line.decode(errors="ignore").rstrip())
    except Exception:
        pass

#stats tracking
total_time_by_state = defaultdict(float)
time_by_app = defaultdict(float)
switch_count_by_app = defaultdict(int)
switch_events = 0

distracted_episodes = 0
in_distracted = False

last_stable_app = None
last_stable_state = "UNKNOWN"

last_change_candidate_time = now_s()
candidate_app = None
candidate_state = None

last_tick = now_s()
last_summary = now_s()

last_sent_simple_state = None


def emit_event(event_type: str, app: str, state: str):
    ts = int(now_s())

    if PRINT_EVENTS:
        print(f"[{ts}] {event_type} app={app} state={state}")
        
    #send_line(f"{event_type} {app} {state}")

def send_simple_state_if_changed(state: str):
 #ignore unknown for timing
    global last_sent_simple_state
    if state not in ("FOCUSED", "DISTRACTED"):
        return
    if state != last_sent_simple_state:
        send_line(state)
        last_sent_simple_state = state

def print_summary():
    focused = total_time_by_state["FOCUSED"]
    distracted = total_time_by_state["DISTRACTED"]
    unknown = total_time_by_state["UNKNOWN"]
    total = focused + distracted + unknown

    print("=============================\n")
    print("STUDY PET SUMMARY")
    print(f"Total:       {fmt_time(total)}")
    print(f"Focused:     {fmt_time(focused)}")
    print(f"Distracted:  {fmt_time(distracted)}")
    print(f"Unknown:     {fmt_time(unknown)}")
    print(f"App switches:{switch_events}")
    print(f"Distracted episodes: {distracted_episodes}")

    top = sorted(time_by_app.items(), key=lambda x: x[1], reverse=True)[:8]
    print("\ntop apps by time:")
    for app, sec in top:
        print(f"  {app:18} {fmt_time(sec)}   switches={switch_count_by_app[app]}")
    print("=============================\n")

    # send summary to stm
    # send_line(
    #     f"SUMMARY total={int(total)} focused={int(focused)} distracted={int(distracted)} "
    #     f"unknown={int(unknown)} switches={switch_events} episodes={distracted_episodes}"
    # )

print("app detector running! (Ctrl+C to stop)")

try:
    while True:
        
        t = now_s()

        dt = t - last_tick
        last_tick = t

        if last_stable_app is not None:
            total_time_by_state[last_stable_state] += dt
            time_by_app[last_stable_app] += dt

        app = get_foreground_app()
        if app is None:
            time.sleep(POLL_INTERVAL_SEC)
            continue
        state = classify_app(app)

        if app != candidate_app:
            candidate_app = app
            candidate_state = state
            last_change_candidate_time = t

        stable_for = t - last_change_candidate_time
        change_is_stable = stable_for >= GRACE_PERIOD_SEC

        if change_is_stable:
            if last_stable_app is None:
                last_stable_app = candidate_app
                last_stable_state = candidate_state
                switch_count_by_app[last_stable_app] += 1

                emit_event("APP_ENTER", last_stable_app, last_stable_state)
                send_simple_state_if_changed(last_stable_state)

                in_distracted = (last_stable_state == "DISTRACTED")
                if in_distracted:
                    distracted_episodes += 1
                    emit_event("DISTRACTED_START", last_stable_app, last_stable_state)

            #stable app changed (log even if same state)
            elif candidate_app != last_stable_app:
                emit_event("APP_EXIT", last_stable_app, last_stable_state)

                last_stable_app = candidate_app
                last_stable_state = candidate_state

                switch_events += 1
                switch_count_by_app[last_stable_app] += 1
                emit_event("APP_ENTER", last_stable_app, last_stable_state)

                #send simple state to STM32 if changed
                send_simple_state_if_changed(last_stable_state)

                #track distracted episodes
                if last_stable_state == "DISTRACTED":
                    if not in_distracted:
                        distracted_episodes += 1
                        emit_event("DISTRACTED_START", last_stable_app, last_stable_state)
                    in_distracted = True
                else:
                    if in_distracted:
                        emit_event("DISTRACTED_END", last_stable_app, last_stable_state)
                    in_distracted = False

            #state classification changed while app same (idk if this happens remove if not)
            elif candidate_state != last_stable_state:
                last_stable_state = candidate_state
                emit_event("STATE_UPDATE", last_stable_app, last_stable_state)
                send_simple_state_if_changed(last_stable_state)

        if SUMMARY_EVERY_SEC is not None and (t - last_summary) >= SUMMARY_EVERY_SEC:
            last_summary = t
            print_summary()

        read_stm32()
        time.sleep(POLL_INTERVAL_SEC)
        
        

except KeyboardInterrupt:
    print_summary()
finally:
    if ser is not None:
        ser.close()
