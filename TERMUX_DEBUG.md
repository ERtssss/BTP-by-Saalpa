# BTP ImGui debug — Termux

1. Give Termux storage access once:

```bash
termux-setup-storage
```

2. Run:

```bash
chmod +x termux_btp_debug.sh
./termux_btp_debug.sh
```

The live BTP log is shown in Termux and saved to `/sdcard/BTP-log.txt`.

Important messages to look for:
- `NativeGui initialize complete`
- `DrawText hook reached`
- `Overlay callback called`
- `GUI draw entered`
- `ImGui context created`
- `ImGui OpenGL3 backend initialized`
- `ImGui frame rendered`
