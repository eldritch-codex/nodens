#!/usr/bin/env bash
#
# Runs a command inside the Nodens Docker container.
#
# Zed runs on the host; only the build/run/debug toolchain lives in Docker. The
# container is created from the image declared in the GPU's devcontainer config
# (.devcontainer/nvidia or .devcontainer/dri) and the project is bind-mounted at the same absolute path as on the host, so
# build outputs, compile_commands.json and debugger source paths line up without
# any path remapping.
#
# Usage:
#   .zed/docker.sh <command> [args...]   # run a command inside the container
#   .zed/docker.sh --debug-adapter       # ensure the CodeLLDB adapter is running
#
# Environment overrides:
#   NODENS_GPU         nvidia or dri (default: nvidia when Docker has the NVIDIA
#                      runtime, otherwise dri)
#   NODENS_CONTAINER   container name (default: eldritchcodex-nodens)
#   NODENS_DEBUG_PORT  TCP port for the CodeLLDB adapter (default: 12345)
#
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# Pick the devcontainer config that matches the host GPU. Changing it later needs
# `docker rm` on the container, because runArgs are fixed when it is created.
GPU="${NODENS_GPU:-}"
if [ -z "$GPU" ]; then
  if docker info --format '{{json .Runtimes}}' 2>/dev/null | grep -q nvidia; then
    GPU=nvidia
  else
    GPU=dri
  fi
fi
DEVCONTAINER="$ROOT/.devcontainer/$GPU/devcontainer.json"
if [ ! -f "$DEVCONTAINER" ]; then
  echo "error: no devcontainer config for NODENS_GPU=$GPU (expected $DEVCONTAINER)" >&2
  exit 1
fi

CONTAINER="${NODENS_CONTAINER:-eldritchcodex-nodens}"
DEBUG_PORT="${NODENS_DEBUG_PORT:-12345}"
ADAPTER_DIR="/opt/codelldb"

# The devcontainer file is JSONC, so strip comments and trailing commas before
# parsing. The image is the single source of truth for the toolchain; the
# runArgs (e.g. --gpus=all) carry the host-specific GPU configuration.
mapfile -t DEVCONTAINER_VALUES < <(python3 - "$DEVCONTAINER" <<'PY'
import json, re, sys

src = open(sys.argv[1]).read()
src = re.sub(r'//[^\n]*', '', src)
src = re.sub(r',(\s*[}\]])', r'\1', src)
cfg = json.loads(src)

print(cfg.get('image', ''))
for arg in cfg.get('runArgs', []):
    if not arg.startswith('--name'):
        print(arg)
PY
)

IMAGE="${DEVCONTAINER_VALUES[0]:-}"
RUN_ARGS=("${DEVCONTAINER_VALUES[@]:1}")

if [ -z "$IMAGE" ]; then
  echo "error: could not read \"image\" from $DEVCONTAINER" >&2
  exit 1
fi

# Zed's bundled CodeLLDB adapter is self-contained (libc plus its own bundled
# libpython), so it can run inside the container. Mount it read-only and let Zed
# connect to it over TCP.
ADAPTER_HOST_DIR=""
for candidate in "$HOME"/.local/share/zed/debug_adapters/CodeLLDB/CodeLLDB_v*/extension; do
  [ -d "$candidate" ] && ADAPTER_HOST_DIR="$candidate"
done

container_running() {
  [ "$(docker inspect -f '{{.State.Running}}' "$CONTAINER" 2>/dev/null || echo false)" = "true" ]
}

ensure_container() {
  if ! docker inspect "$CONTAINER" >/dev/null 2>&1; then
    echo "Creating container '$CONTAINER' from $IMAGE" >&2
    local args=(
      --detach
      --name "$CONTAINER"
      --volume "$ROOT:$ROOT"
      --volume /tmp/.X11-unix:/tmp/.X11-unix
      --env "DISPLAY=${DISPLAY:-}"
      --env "WAYLAND_DISPLAY=${WAYLAND_DISPLAY:-}"
      --env "NVIDIA_DRIVER_CAPABILITIES=graphics,display,utility"
      --publish "127.0.0.1:${DEBUG_PORT}:${DEBUG_PORT}"
      --user eldritchcodex
      --workdir "$ROOT"
    )
    if [ -n "${XDG_RUNTIME_DIR:-}" ]; then
      args+=(--volume "$XDG_RUNTIME_DIR:$XDG_RUNTIME_DIR" --env "XDG_RUNTIME_DIR=$XDG_RUNTIME_DIR")
    fi
    if [ -n "$ADAPTER_HOST_DIR" ]; then
      args+=(--volume "$ADAPTER_HOST_DIR:$ADAPTER_DIR:ro")
    fi
    args+=("${RUN_ARGS[@]}")
    # Override the entrypoint so the container is kept alive deterministically
    # (mirrors what the devcontainer CLI does).
    docker run "${args[@]}" --entrypoint /bin/sh "$IMAGE" -c "sleep infinity" >/dev/null
  elif ! container_running; then
    echo "Starting container '$CONTAINER'" >&2
    docker start "$CONTAINER" >/dev/null
  fi
}

ensure_debug_adapter() {
  [ -n "$ADAPTER_HOST_DIR" ] || return 0
  if ! docker exec "$CONTAINER" pgrep -f 'codelldb --port' >/dev/null 2>&1; then
    docker exec --detach --user eldritchcodex --workdir "$ROOT" "$CONTAINER" \
      "$ADAPTER_DIR/adapter/codelldb" --port "$DEBUG_PORT" --multi-session >/dev/null
  fi
}

ensure_container

if [ "${1:-}" = "--debug-adapter" ]; then
  ensure_debug_adapter
  exit 0
fi

exec_args=(--interactive)
if [ -t 0 ]; then
  exec_args+=(--tty)
fi

exec docker exec "${exec_args[@]}" --user eldritchcodex --workdir "$ROOT" "$CONTAINER" "$@"
