ulimit -S -c unlimited
cat /proc/sys/kernel/core_pattern
sudo sysctl -w kernel.core_pattern=./core
gdb -q ./test core
bt
# This last line check the line where the error was triggered

