import requests
import time
print("Content-type: text/plain")
print()

URL = "http://localhost:8081/cgi-bin/test.cgi"

while True:
    time.sleep(0.01)
    try:
        r = requests.get(URL)
        print("Status:", r.status_code, "Length:", len(r.content))
    except Exception as e:
        print("Error:", e)
