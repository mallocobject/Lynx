wrk.method = "POST"
wrk.headers["Content-Type"] = "application/json"

request = function()
    wrk.body = string.format('{"action":"verify","email":"lynxqqcom@qq.com","type":"register"}')
    return wrk.format()
end

-- {action: "verify", email: "lynxqqcom@qq.com", type: "register"}