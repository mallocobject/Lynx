wrk.method = "POST"
wrk.headers["Content-Type"] = "application/json"

request = function()
    wrk.body = string.format('{"action":"login","password":"123456","username":"liudan"}')
    return wrk.format()
end