#!/bin/bash

curl -v --url "smtp://smtp.qq.com:587" --ssl-reqd \
--mail-from "lynxqqcom@qq.com" \
--mail-rcpt "lynxqqcom@qq.com" \
--upload-file "./test_mail.txt" \
--user "lynxqqcom@qq.com:" \
--login-options "AUTH=LOGIN"