local cmd = Execute.new()

function waitLoop()
    local ok = false
    local status = 0
    repeat
        if cmd:IsExecute() == false then
            ok, status = cmd:GetResult()
        end
    until ok
    print("STATUS:", ok, status)
    return true
end

--
local co = coroutine.create(waitLoop)
local finish = true
print "command start"
cmd:Execute(Args[1])
repeat
    finish = true
    if coroutine.status(co) ~= "dead" then
        coroutine.resume(co)
        finish = false
    end
until finish
print "done"
