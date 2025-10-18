#!/bin/bash

CONFIG_FILE="observer.conf"
LOG_FILE="observer.log"

touch "$LOG_FILE"

log_print(){
        echo "$(date '+%Y-%m-%d %H:%M:%S') - $1" >> "$LOG_FILE"
}



check_sh(){
	local flag=1
    	local script_name="$1"
    	for pid in /proc/[0-9]*; do
        	if [ -d "$pid" ]; then
            		if [ -r "$pid/cmdline" ]; then
                		cmdline=$(tr -d '\0' < "$pid/cmdline" 2>/dev/null)
                		if [[ "$cmdline" == *"$script_name"* ]]; then
                    			flag=0
                		fi
            		fi
        	fi
    	done
    	return $flag
}

start_sh(){
	if [ ! -f "$1" ]; then
		log_print "ERROR: Файл $1 не найден"
		return 1
	fi

	chmod +x "$1" 2>/dev/null

	nohup "./$1" > "/dev/null" 2>&1 &
	local pid=$!

	if check_sh "$1"; then
       		log_print "PROCESS START: $script_name (PID: $pid)"
        	return 1
    	else
        	log_print "ERROR: Не удалось запустить $script_name"
        	return 0
    	fi
}

log_print "Запуск observer.sh ..."

if [ ! -f "$CONFIG_FILE" ]; then
        log_print "ERROR: Файл $CONFIG_FILE не найден"
        exit 1
fi

while read -r str; do

	log_print "CHECK: $str"
	
	if check_sh "$str"; then
		log_print "STATUS: $str запущен"
	else
		log_print "STATUS: $str не запущен"
		start_sh "$str"

	fi
done < "$CONFIG_FILE"

log_print "Выполнение проверки завершено"
log_print ""
