import sys
print("Content-type: text/plain")
print()
# Read all input from stdin and print it
print(sys.stdin.read())

# Or iterate line by line
for line in sys.stdin:
    print(line.strip()) # strip() removes the trailing newline   