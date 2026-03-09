
> **Subject:** Directory where the requested file should be located  
> (e.g., if URL `/kapouet` is rooted to `/tmp/www`, URL `/kapouet/pouic/toto/pouet` will search for `/tmp/www/pouic/toto/pouet`)

---
## 🌐 listen

Sets the address and port for IP on which the server will accept requests.

You can specify:
- Both address **and** port
- Only the port (**Default address:** `0.0.0.0`)

**Syntax:**

```
[listen address:]port;
```

**Context:** `server`

### Examples

```nginx
listen 127.0.0.1:8000;
listen 127.0.0.1;
listen 8000;
```

---
## server_name


**Syntax:** `server_name name;`  
**Default:** `server_name "";`  
**Context:** `server`  

**Beschreibung:**  
Legt die Namen eines virtuellen Servers fest. Zum Beispiel:

```nginx
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

**Default:**

```
root ./html;
```

**Context:** `http`, `server`, `location`

---



## 📄 index

Defines files that will be used as an index.

- The file name can contain variables.
- Files are checked in the specified order.
- The last element of the list can be a file with an absolute path.

**Syntax:**

```
index file ...;
```

**Default:**

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

**Default:** —  
**Context:** `location`

### Example

```nginx
allow_methods GET POST;
```

---

## 🔁 Return Specific Status Codes

Some website URIs require immediate return of a response with a specific error or redirect code, for example when a page has been moved temporarily or permanently.

The easiest way to do this is to use the `return` directive.

**Default:** —  
**Context:** `server`, `location`


### Example

```nginx
location /wrong/url {
    return 404;
}
```

The first parameter of `return` is a response code.  
The optional second parameter can be:

- The URL of a redirect (for codes `301`, `302`, `303`, and `307`)
- Or the text to return in the response body

### Redirect Example

```nginx
location /permanently/moved/url {
    return 301 http://www.example.com/moved/here;
}
```

---

### ⚙️ client_body_timeout  
Defines a timeout for reading the client request body.  
The timeout applies only to the period between two successive read operations, not to the transmission of the entire request body.

If a client does not transmit any data within this time, the request is terminated with a **408 (Request Time-out)** error.

**Syntax:**

client_body_timeout time;

**Default:**

client_body_timeout 60s;

**Context:** http, server, location

---
### ⚙️ client_header_buffer_size  
Sets the buffer size for reading the client request header.

For most requests, a **1K buffer** is sufficient. However, requests containing **long cookies** or requests from **WAP clients** may exceed this size.  
If the request line or a header field does not fit into this buffer, larger buffers configured with **large_client_header_buffers** are allocated.

If specified at the **server level**, the value from the **default server** may be used.

**Syntax:**

client_header_buffer_size size;

**Default:**

client_header_buffer_size 1k;

**Context:** http, server

---
### ⚙️ client_header_timeout  
Defines a timeout for reading the client request header.

If the client does not transmit the entire header within the specified time, the request is terminated with a **408 (Request Time-out)** error.

**Syntax:**

client_header_timeout time;

**Default:**

client_header_timeout 60s;

**Context:** http, server

---
### ⚙️ client_max_body_size  
Sets the maximum allowed size of the client request body.

If the request body exceeds the configured value, NGINX returns a **413 (Request Entity Too Large)** error.  
Note that browsers often cannot correctly display this error.

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
upload_store directory
```

**Default:** —  
**Context:** `location`

### Example (Simple Flat Directory)

```nginx
upload_store /var/upload;
```

---

## ❗ error_page

Defines the URI that will be shown for the specified errors.

A `uri` value can contain variables.

**Syntax:**

```
error_page code ... [=[response]] uri;
```


**Context:** `http`, `server`, `location`

### Example

```nginx
error_page 404             /404.html;
error_page 500 502 503 504 /50x.html;
```

This causes an internal redirect to the specified `uri` with the client request method changed to **GET**  
(for all methods other than `GET` and `HEAD`).

---

## ⚡ FastCGI Configuration

This `location` block configures Nginx to serve **PHP files** using **FastCGI** with PHP-FPM.

**Context:** `http`, `server`, `location`

**Syntax:**

```nginx
location /wordpress/ {
    fastcgi_path unix:/run/php/php8.2-fpm.sock; # custom
	fastcgi_index index.php;
}
```

### Explanation

- `fastcgi_path unix:/run/php/php8.2-fpm.sock;` – Passes PHP requests to the PHP-FPM socket.  
- `fastcgi_index index.php;` – Specifies the default index file for FastCGI requests.  

This setup allows Nginx to process PHP files in `/wordpress/` correctly.