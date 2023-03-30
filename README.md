## Compilation

```bash
make
```

## Run
By default, it would listen to port 8080, but it could be modified.
```main
./main
```
Could see the default page in http://localhost:8080/

## Add Routes
I added four common route, add_get, add_update, add_pos, add_delete. For other request methods, could use add_route to add.
```cpp
http_server server;
server.add_get("/main", some_function);
server.add_route("CONNECT", "/some-connection", some_other_function);
```

## Run
```cpp
server.run(8080);
```

## What Not Supported
octet stream

## Third Party Library
https://github.com/nodejs/http-parser
I also added some more parsing part on it to parse multipart data (though the content inside each multipart data is not really parsed)
