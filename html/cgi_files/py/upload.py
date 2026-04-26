import cgi
import os
import html

print("Content-Type: text/html")
print("\r\n\r\n")

form = cgi.FieldStorage()

upload_dir = "uploads"
os.makedirs(upload_dir, exist_ok=True)

print("<html><body>")
print("<h1>Python CGI Upload</h1>")

if "file" not in form:
    print("<p>No file field found.</p>")
else:
    fileitem = form["file"]

    if not fileitem.filename:
        print("<p>No file uploaded.</p>")
    else:
        filename = os.path.basename(fileitem.filename)
        safe_name = html.escape(filename)

        filepath = os.path.join(upload_dir, filename)

        with open(filepath, "wb") as f:
            f.write(fileitem.file.read())

        print(f"<p>Uploaded: {safe_name}</p>")
        print(f"<p>Saved to: {html.escape(filepath)}</p>")

print("</body></html>")
