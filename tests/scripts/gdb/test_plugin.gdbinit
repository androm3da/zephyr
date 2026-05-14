# Source the Zephyr GDB plugin using ZEPHYR_BASE env var
python
import os
gdb.execute('source ' + os.path.join(os.environ['ZEPHYR_BASE'], 'scripts', 'gdb', 'zephyr_gdb.py'))
end

# Break at the inspection point where threads and objects are in known state
b inspection_point
c

# Run all plugin commands
zephyr threads
zephyr current
zephyr mutex my_mutex
zephyr sem my_sem

# If we reach here without error, test passed
printf "GDB_PLUGIN:PASSED\n"
quit 0
