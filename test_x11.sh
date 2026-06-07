#!/bin/bash
for ip in 192.168.1.157 172.17.240.1 192.168.80.1 host.docker.internal; do
  timeout 2 bash -c "echo > /dev/tcp/$ip/6000" 2>/dev/null && echo "$ip is OPEN" || echo "$ip is CLOSED"
done