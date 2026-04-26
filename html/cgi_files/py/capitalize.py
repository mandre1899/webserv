#!/usr/bin/env python3

import os
import sys
import cgi
import html

print("Content-Type: text/html")
print()

print("<h2>Debug</h2>")
print("<pre>")

print("REQUEST_METHOD =", os.environ.get("REQUEST_METHOD"))
print("CONTENT_LENGTH =", os.environ.get("CONTENT_LENGTH"))
print("CONTENT_TYPE =", os.environ.get("CONTENT_TYPE"))

# Parse normal forms + multipart/form-data
form = cgi.FieldStorage()

print("FIELDS:")
for key in form.keys():
    field = form[key]

    # Handle uploaded file
    if field.filename:
        print(f"{key} = uploaded file: {field.filename}")
    else:
        print(f"{key} = {field.value!r}")

print("</pre>")

# Read values
target = form.getvalue("target", "")
message = form.getvalue("message", "")

# Convert to uppercase
message = message.upper()

print("<h2>Result</h2>")
print(f"Target: {html.escape(target)}<br>")
print(f"Message: {html.escape(message)}<br>")

# Optional file upload handling
if "file" in form:
    fileitem = form["file"]

    if fileitem.filename:
        data = fileitem.file.read()
        print(f"Uploaded File: {html.escape(fileitem.filename)}<br>")
        print(f"Size: {len(data)} bytes<br>")
