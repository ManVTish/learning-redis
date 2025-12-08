# Notes:
## TCP SERVER
#### Step 1: Open Socket handler
```int fd = socket(AF_INET, SOCK_STREAM, 0);```

<li>AF_INET: IPv4</li>
<li>AF_INET6: IPv6</li>
<li>SOCK_STREAM: TCP</li>
<li>SOCK_DGRAM: UDP</li>


#### Step 2: Set Socket Options
```int fd = socket(AF_INET, SOCK_STREAM, 0);```

#### Step 3: Bind Address and Port
```int fd = socket(AF_INET, SOCK_STREAM, 0);```

#### Step 4: Listen to connection
```int fd = socket(AF_INET, SOCK_STREAM, 0);```

#### Step 5: Accept & Close Connection
```int fd = socket(AF_INET, SOCK_STREAM, 0);```


## TCP CLIENT

