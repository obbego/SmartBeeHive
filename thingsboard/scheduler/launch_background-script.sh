PATH = "YOUR_FILE_PATH"

# check if the python has been installed
if ! command -v python3 &> /dev/null
then
    echo "Python3 is not installed. Check it to continue"
    exit 1
fi

nohup python3 "$SCRIPT_PATH" & # execute the script
echo "Script $SCRIPT_PATH avviato con successo"