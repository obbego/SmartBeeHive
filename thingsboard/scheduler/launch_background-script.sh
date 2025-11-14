SCRIPT_PATH="rpc-scheduler.py"

# check if the python has been installed
if ! command -v python3 &> /dev/null
then
    echo "Python3 is not installed. Check it to continue"
    exit 1
fi

# check if the file exists
if [ ! -f "$SCRIPT_PATH" ]; then
    echo "Errore: file $SCRIPT_PATH non trovato"
    exit 1
fi

nohup python3 "$SCRIPT_PATH" & # execute the script