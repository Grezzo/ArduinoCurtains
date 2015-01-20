rm -f out
mkfifo out
trap "rm -f out" EXIT

#-hupcl
#stty -f /dev/tty.usbmodem26211 cs8 115200 ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts

while true
do
  cat out | nc -l 1500 > >( # parse the netcat output, to build the answer redirected to the pipe "out".
    export REQUEST=
    while read line
    do
      line=$(echo "$line" | tr -d '[\r\n]')

      if echo "$line" | grep -qE '^GET /' # if line starts with "GET /"
      then
        REQUEST=$(echo "$line" | cut -d ' ' -f2) # extract the request
echo $REQUEST
      elif [ "x$line" = x ] # empty line / end of request
      then
        HTTP_200="HTTP/1.1 200 OK"
        HTTP_LOCATION="Location:"
        HTTP_404="HTTP/1.1 404 Not Found"
        # call a script here
        # Note: REQUEST is exported, so the script can parse it (to answer 200/403/404 status code + content)
        if echo $REQUEST | grep -qE '^/echo/'
        then
            printf "%s\n%s %s\n\n%s\n" "$HTTP_200" "$HTTP_LOCATION" $REQUEST ${REQUEST#"/echo/"} > out
        elif echo $REQUEST | grep -qE '^/close'
        then
            echo Closing...
            echo -n 0,100 > /dev/tty.usbmodem26211
            printf "%s\n%s %s\n\n%s\n" "$HTTP_200" "$HTTP_LOCATION" $REQUEST "Closing" > out
        elif echo $REQUEST | grep -qE '^/open'
        then
            echo Opening...
            echo -n 0,0 > /dev/tty.usbmodem26211
            printf "%s\n%s %s\n\n%s\n" "$HTTP_200" "$HTTP_LOCATION" $REQUEST "Opening" > out
        elif echo $REQUEST | grep -qE '^/moveto'
        then
            echo Moving...
            PERCENTAGE=$(echo "$REQUEST" | cut -c 18-20)
            echo $PERCENTAGE
            echo -n 0,$PERCENTAGE > /dev/tty.usbmodem26211
            printf "%s\n%s %s\n\n%s\n" "$HTTP_200" "$HTTP_LOCATION" $REQUEST "Moving" > out
        elif echo $REQUEST | grep -qE '^/alarm'
        then
            echo -n Setting alarms:
            NOWHOUR=$(echo "$REQUEST" | cut -c 13-14)
            NOWMIN=$(echo "$REQUEST" | cut -c 16-17)
            OPENHOUR=$(echo "$REQUEST" | cut -c 24-25)
            OPENMIN=$(echo "$REQUEST" | cut -c 27-28)
            CLOSEHOUR=$(echo "$REQUEST" | cut -c 36-37)
            CLOSEMIN=$(echo "$REQUEST" | cut -c 39-40)
            echo 1,$NOWHOUR:$NOWMIN,$OPENHOUR:$OPENMIN,$CLOSEHOUR:$CLOSEMIN
            echo -n 1,$NOWHOUR:$NOWMIN,$OPENHOUR:$OPENMIN,$CLOSEHOUR:$CLOSEMIN > /dev/tty.usbmodem26211
            printf "%s\n%s %s\n\n%s\n" "$HTTP_200" "$HTTP_LOCATION" $REQUEST "Alarms Set" > out
        elif echo $REQUEST | grep -qE '^/$'
        then
            cat index.html > out
        else
            printf "%s\n%s %s\n\n%s\n" "$HTTP_404" "$HTTP_LOCATION" $REQUEST "Resource $REQUEST NOT FOUND!" > out
        fi
      fi
    done
  )
done
