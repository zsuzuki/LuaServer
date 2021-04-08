local port = 12345
local domain = ""
local count = 0

-- サーバーの待ち受けループ
function waitLoop()
    repeat
        Server.Update()
        coroutine.yield(false)
    until false
    return true
end

-- GETメソッドのレスポンス生成
function makeGetResponse()
    local res = {}
    res.A = true
    res.B = "Hello,World"
    res.C = 55
    res.D = 10.26
    res.Count = count
    if count & 1 == 0 then
        res.Option = "Enable"
    end
    count = count + 1
    return res
end

--
local co = coroutine.create(waitLoop)
local finish = true
print "server start"
Server.Start("/demo", port, domain)
Server.MakeGetResponse = makeGetResponse
repeat
    finish = true
    if coroutine.status(co) ~= "dead" then
        coroutine.resume(co)
        finish = false
    end
until finish
Server.Stop()
print "done"
