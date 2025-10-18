#!/bin/bash

SCRIPT_NAME=$(basename "$0")
if [ "$SCRIPT_NAME" = "template_task.sh" ]; then
    echo "я бригадир, сам не работаю"
    sleep 10
    exit 1
fi
REPORT_FILE="report_${SCRIPT_NAME%.*}.log"
PID=$$
CURRENT_DATETIME=$(date '+%Y-%m-%d %H:%M:%S')
touch "report_${SCRIPT_NAME%.*}.log"
echo "[$PID] $CURRENT_DATETIME Скрипт запущен" >> "$REPORT_FILE"

RANDOM_SECONDS=$(( RANDOM % 1771 + 30 ))
sleep $RANDOM_SECONDS
MINUTES=$(( RANDOM_SECONDS / 60 ))
CURRENT_DATETIME=$(date '+%Y-%m-%d %H:%M:%S')
echo "[$PID] $CURRENT_DATETIME Скрипт завершился, работал $MINUTES минут" >> "$REPORT_FILE"
