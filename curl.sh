#!/bin/bash

# curl -v --url "smtp://smtp.qq.com:587" --ssl-reqd \
# --mail-from "lynxqqcom@qq.com" \
# --mail-rcpt "lynxqqcom@qq.com" \
# --upload-file "./test_mail.txt" \
# --user "lynxqqcom@qq.com:" \
# --login-options "AUTH=LOGIN"

curl -v --noproxy "*" -X POST \
  -H "Content-Type: application/json" \
  -H "Token: 9tOHuU1opvZ0xZpysqWDljuPO1w_BvPn" \
  -d '{"action":"verify","email":"lynxqqcom@qq.com","type":"register"}' \
  http://127.0.0.1:8080/verify