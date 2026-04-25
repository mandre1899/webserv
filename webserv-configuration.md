
# WebServ

This document describes the configuration options for the WebServ web server.

## 🌐 listen

Sets the address and port for IP on which the server will accept requests.

You can specify:
- Both address **and** port
- Only the port (**Default address:** `0.0.0.0`)

**Default:**
```
0.0.0.0:80
```

**Syntax:**
```
[listen address:]port;
```

**Context:** `server`

### Examples

```webserv
listen 127.0.0.1:8000;
listen 8000;
```

---
## server_name

**Syntax:** `server_name name;` 
**Context:** `server`  

**Description:**  
Set the name of a virtual server. For example:

```webserv
server {
    server_name example.com;
}
```

---

## / root

The `root` directive defines the **root directory for requests**. It specifies the folder from which static files will be served.

**Syntax:**
```
root path;
```

**Examples:**
```
root ./html;
```

**Context:** `http`, `server`, `location`

---

## 📄 index

Defines files that will be used as an index.

**Syntax:**
```
index file;
```

**Examples:**
```
index index.html;
```

**Context:** `http`, `server`, `location`

---

## 🚫 allow_methods

Limits allowed HTTP methods inside a `location`.

The `allow_methods` parameter can be one of the following:

- `GET`
- `POST`
- `DELETE`

**Syntax:**

```
allow_methods method;
```

**Default:** `allow_methods GET POST;`
**Context:** `location`

### Example

```webserv
allow_methods GET DELETE;
```

---

## 🔁 Return Specific Status Codes

Some website URIs require immediate return of a response with a specific error or redirect code, for example when a page has been moved temporarily or permanently.

The easiest way to do this is to use the `return` directive.

**Default:** —  
**Context:** `server`, `location`

### Example

```webserv
location /wrong/url {
    return 404;
}
```

The first parameter of `return` is a response code.  
The optional second parameter can be:

- The URL of a redirect (for codes `301`, `302`, `303`, and `307`)

### Redirect Example

```webserv
location /permanently/moved/url {
    return 301 http://www.example.com/moved/here;
}
```

---

### ⚙️ client_header_buffer_size  
Sets the buffer size for reading the client request header.

**Syntax:**

client_header_buffer_size size;

**Default:**

client_header_buffer_size 1k;

**Context:** http, server

---
### ⚙️ client_header_timeout  
Defines a timeout for reading the client request header.

**Syntax:**

client_header_timeout time;

**Default:**

client_header_timeout 30s;

**Context:** http, server

---

### ⚙️ client_body_timeout  
Defines a timeout for reading the client request body.  

**Syntax:**

client_body_timeout time;

**Default:**

client_body_timeout 30s;

**Context:** http, server, location

---
### ⚙️ client_max_body_size  
Sets the maximum allowed size of the client request body.

Setting the value to **0** disables the request body size check.

**Syntax:**

client_max_body_size size;

**Default:**

client_max_body_size 1m;

**Context:** http, server, location

---

## 📂 autoindex

Enables or disables directory listing output.

**Syntax:**

```
autoindex on | off;
```

**Default:**

```
autoindex off;
```

**Context:** `http`, `server`, `location`

---

## 📤 upload_store

Specifies where uploaded files will be saved.

**Syntax:**

```
upload_store directory;
```

### Example (Simple Flat Directory)

```webserv
upload_store /var/upload;
```
**Default:** —  
**Context:** `location`
---

## ❗ error_page

Defines the page that will be shown for the specified errors.

**Syntax:**

```
error_page code ... path;
```

### Example

```webserv
error_page 404             /404.html;
error_page 500 502 503 504 /50x.html;
```

**Context:** `http`, `server`, `location`
---

## ⚡ CGI Configuration

This `location` block configures webserv to serve **PHP files** using *CGI* with PHP.

**Syntax:**

```webserv
location /wordpress/ {
    cgi_path /bin/php;
    cgi_ext .php;
}
```

### Explanation

- `cgi_path /bin/php;` – Passes PHP requests to the PHP executable. 
- `cgi_ext .php;` – Specify the file extension

**Context:** `location`

---

### Symbol explanation

| Symbol | Meaning | Value |
| --- | --- | --- |
| k | kilobyte | 1,000 Byte|
| M | megabyte | 1,000,000 Byte|
| G | gigabyte | 1,000,000,000 Byte|
| s | second | unit of time |