wrk.method = "POST"
wrk.headers["Content-Type"] = "application/json"

request = function()
    local a = 1
    local b = 2
    wrk.body = string.format('{"a": %d, "b": %.2f}', a, b)
    return wrk.format()
end