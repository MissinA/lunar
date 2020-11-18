local lunar = require('lunar')
local htmlFile = io.open('C:/Users/chen/Desktop/lunar/build/windows/x86/release/hi.htm', "r+")
local html = string.format("data:text/html,%s", htmlFile:read("*a"))
htmlFile:close();

local wb = lunar.new("xxs", true, true)
wb:size(800, 240)

wb:bind("l_move", function(req)
    print('move')
    wb:move()
    return 0
end)

wb:navigate(html)
wb:run()