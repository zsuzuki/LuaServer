if LevelDB.Start("leveldb.db") == false then
    print "LevelDB Start Error"
    return
end

if LevelDB.Put("Test", "OK Status") == false then
    print "LevelDB Put Error"
    return
end

local R0, ok0 = LevelDB.Get("Test")
if ok0 then
    print("Search:", R0)
else
    print "not found Test"
end
local R1, ok1 = LevelDB.Get("Temp")
if ok1 then
    print("Search:", R1)
else
    print "not found Temp"
end
