local httpclient = Client.new()
local waitResponse = false

httpclient.Port = 12345
httpclient.Server = "10.211.55.3"

-- 待ち受け
function waitLoop()
    repeat
        httpclient:Update()
        coroutine.yield(false)
    until waitResponse
    return true
end

-- サーバーからのレスポンス
function clientResponse(args)
    print "response"
    for k, v in pairs(args) do
        print(k, v)
    end
    waitResponse = true
end

local co = coroutine.create(waitLoop)
local finish = true
print "client start"
httpclient:Query("/demo", clientResponse, {test = "Client", sample = "HTTP"})
repeat
    finish = true
    if coroutine.status(co) ~= "dead" then
        coroutine.resume(co)
        finish = false
    end
until finish
print "done"
