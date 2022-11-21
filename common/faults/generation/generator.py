""" generator.py: Fault code Generation (faults.c and faults.h). """
import sys

arg = -1
try:
    arg = sys.argv[1]
except:
    print("This has failed")
print(f"Attempting to generate code for {arg}")
