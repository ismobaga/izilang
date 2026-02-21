# http — HTTP Client

The `http` module provides a synchronous HTTP/1.1 client for making network requests. Plain HTTP is supported; HTTPS is not available in this version.

## Import

```izilang
// Module object
import "http";
var response = http.get("http://example.com/api/data");

// Namespaced wildcard
import * as http from "std.http";

// Named imports
import { get, post, request } from "http";
```

## Functions

### `get(url)`

Makes an HTTP GET request to `url`. Returns a [response map](#response-map).

```izilang
import "http";

var res = http.get("http://httpbin.org/get");
if (res.ok) {
    print("Status:", res.status);
    print("Body:", res.body);
} else {
    print("Error:", res.status, res.statusText);
}
```

### `post(url, body, contentType?)`

Makes an HTTP POST request to `url` with `body`. `contentType` defaults to `"application/x-www-form-urlencoded"`. Returns a [response map](#response-map).

```izilang
import "http";

var res = http.post(
    "http://httpbin.org/post",
    "name=IziLang&version=0.4",
    "application/x-www-form-urlencoded"
);
print("POST status:", res.status);
```

### `request(options)`

Makes a generic HTTP request. `options` is a map with the following keys:

| Key | Type | Required | Default | Description |
|-----|------|----------|---------|-------------|
| `url` | String | ✅ | — | Request URL |
| `method` | String | — | `"GET"` | HTTP method |
| `body` | String | — | `""` | Request body |
| `contentType` | String | — | `""` | Content-Type header |

```izilang
import "http";

var res = http.request({
    "method": "PUT",
    "url": "http://httpbin.org/put",
    "body": "{\"key\": \"value\"}",
    "contentType": "application/json"
});
print("PUT status:", res.status);
```

## Response Map

All request functions return a map with these fields:

| Field | Type | Description |
|-------|------|-------------|
| `status` | Number | HTTP status code (e.g., `200`, `404`) |
| `statusText` | String | HTTP reason phrase (e.g., `"OK"`) |
| `ok` | Bool | `true` if status is 200–299 |
| `body` | String | Response body as a string |
| `headers` | Map | Response headers (`name → value`) |

## Complete Example

```izilang
import * as http from "http";
import * as json from "std.json";

// GET and parse JSON response
var res = http.get("http://httpbin.org/json");
if (res.ok) {
    var data = json.parse(res.body);
    print("Parsed object:", data);
} else {
    print("Request failed:", res.status);
}

// POST JSON data
fn postJson(url, data) {
    return http.post(url, json.stringify(data), "application/json");
}

var postRes = postJson("http://httpbin.org/post", {"lang": "IziLang", "version": 0.4});
print("POST response status:", postRes.status);

// Error handling
try {
    var bad = http.get("http://nonexistent.example.invalid/");
} catch (e) {
    print("Network error:", e);
}
```

## Notes

- Only **plain HTTP** is supported. HTTPS will throw a runtime error.
- All requests are **synchronous** (blocking).
- DNS failures and connection errors throw a runtime error.
- The `Connection: close` header is sent automatically; persistent connections are not supported.
- Unsupported URL schemes (`ftp://`, `https://`, etc.) throw a runtime error.

## See Also

- [Standard Library Index](README.md)
- [json module](json.md) — Parse JSON response bodies
- [ipc module](ipc.md) — Local process communication without HTTP
