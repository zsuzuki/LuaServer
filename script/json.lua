local test<const> = {}

test.a = 1
test.b = true
test.c = "OK"

local js = ToJSON(test)
ChangeDir("script")
local f = io.open("test.json","w")
f:write(js,"\n")
f:close()
