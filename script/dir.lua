print "args"
local sdir = "."
for i, a in pairs(Args) do
    print(i, a)
    sdir = a
end

local ok, flist = GetFileList(sdir)
print "dir"
if ok then
    for i, f in ipairs(flist) do
        print(i, f)
    end
else
    print "failed"
end
print "done"
