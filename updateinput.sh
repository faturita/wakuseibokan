#xinput --set-prop 10 334 0


id=`xinput --list | grep Touchpad | cut -d = -f 2 | cut -d '[' -f 1`
prop=`xinput --list-props $id | grep Disable | head -1 | cut -d '(' -f 2 | cut -d ')' -f 1`

echo $id $prop 
xinput --set-prop $id $prop 0


