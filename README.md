# webserv
HTTP server in C++ developed by [@ixu42](https://github.com/ixu42), [@nikkxll](https://github.com/nikkxll), [@madfcat](https://github.com/madfcat).

Name-based virtual hosting allows several `serverName`s to be served from the same `ipAddress`:`port`

## Config file syntax

Each config part is defined with a header: `[main]`, `[server]` or `[location]`.

`[main]` must be the first config part defined. Only one `[main]` is allowed.

Each `[location]` belongs to the first `[server]` defined before it. `[location]` is invalid without `[server]`.

Each server can have multiple location.

Empty lines are allowed.

### Defining main config

Main config is used to define CGI interpreters paths.

```
[main]
py /usr/bin/python3
php /usr/bin/php
```

### Defining a server

Allowed fields: `ipAddress`, `port`, `serverName`, `error`, `clientMaxBodySize`

If no `ipAddress` is provided, webserv will try to create server on all the interfaces available.

```
[server]
ipAddress 127.0.0.1
port 8006
serverName host2.website1.com
error 404 "user-pages copy/404.html"
```

#### Defining client max body size


`clientMaxBodySize` can be set in `G`, `M`, `KB`, `B`

```
clientMaxBodySize 100KB
```

#### Defining user error pages

Webserv has default error pages stored in `pages/` directory. For example, `pages/404.html`. The user definded error pages will take priority.

Custom error pages can define custom error pages like this:

```
error 404 "user-pages copy/404.html"
error 405 user-pages/405.html
error 500,501,505 other-user-pages/500.html
```

### Defining locations

The priority order: redirect -> user defined index -> index.html (default) -> directory listing

#### Redirect

##### Redirect to an external website

In this case `$request_uri` will be substituted to everything that will be after `/redirect/`

```
[location]
path /redirect/
redirect https://google.com/$request_uri
```


##### Redirect to the same website

```
[location]
path /assets-redirect
redirect /assets/
```

#### Directory listing 

All the url subdirectories will also have directory listing if the parent url directory location has `autoindex on`

```
[location]
path /
root website1/
autoindex on
```

#### Uploading files

Successful upload will redirect to the requested path automatically.

```
[location]
path /uploads/
root website2/uploads/
autoindex on
upload on
```

### Commenting

Each comment should be on a separate line

```
# Simple comment
```

###  Example config file
```
[main]
py /usr/bin/python3
php /usr/bin/php

# host1.website1.com @ 127.0.0.1:8006
[server]
ipAddress 127.0.0.1
port 8006
serverName host1.website1.com

error 404 user-pages/404.html
# error 405
[location]
path /
root website1/
autoindex on

[location]
path /pic-redirect
redirect /skull/

[location]
path /redirect/
redirect https://google.com/$request_uri

# host2.website1.com @ 127.0.0.1:8006
[server]
ipAddress 127.0.0.1
port 8006
serverName host2.website1.com

error 404 "user-pages copy/404.html"
[location]
path /
root "website1 copy/"
autoindex on

[location]
path /pic-redirect
redirect /assets/

[location]
path /redirect/
redirect https://google.com/$request_uri

# host1.website2.com @ 127.0.0.1:8007
[server]
ipAddress 127.0.0.1
port 8007
serverName host1.website2.com

[location]
path /
root website2/
autoindex on

[location]
path /uploads/
root website2/uploads/
autoindex on
upload on
```

### Usage

To run the program with default setting of the config

```
make && ./webserv
```

To run certain config file

```
make && ./webserv default/config_name.conf
```

To compile and run the program in DEBUG mode

```
make debug && ./webserv
```
---
Developed by Hive Helsinki students

ixu: ixu@student.hive.fi | dnikifor: dnikifor@student.hive.fi | vshchuki: vshchuki@student.hive.fi
