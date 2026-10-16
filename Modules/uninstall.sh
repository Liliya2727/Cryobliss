#!/system/bin/sh
# ============================================================
# Project Suisei - Module Uninstall Hook
# ============================================================



# ── Standard module file cleanup ──────────────────────────────────────────────
if [ -f $INFO ]; then
  while read LINE; do
    if [ "$(echo -n $LINE | tail -c 1)" == "~" ]; then
      continue
    elif [ -f "$LINE~" ]; then
      mv -f $LINE~ $LINE
    else
      rm -f $LINE
      while true; do
        LINE=$(dirname $LINE)
        [ "$(ls -A $LINE 2>/dev/null)" ] && break 1 || rm -rf $LINE
      done
    fi
  done < $INFO
  rm -f $INFO
fi

# ── Cleanup temp files ────────────────────────────────────────────────────────
rm -rf /data/local/tmp/logo.png
rm -rf /data/local/tmp/Anya.png

# ── Wipe Project Suisei persistent data (keys, config, etc.) ───────────────────
rm -rf /data/ProjectSuisei

# Managed to read this? Thanks for using Project Suisei
