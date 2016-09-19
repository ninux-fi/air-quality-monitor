-- use pin 0 as the input pulse width counter
function register_post(pm10,pm25)
conn=net.createConnection(net.TCP, 0)  
--conn:on("receive", display) 
conn:on("receive", function(conn, payload) end)
conn:connect(8086,host)  
local post_request = build_post_request(pm10,pm25)
print ("Spedizione")
conn:send(post_request)
conn:on("disconnection", function(conn, payload) print('\nDisconnected') end)
end

function build_post_request(pm10,pm25)
    local data = ""
--    data = "t=100&pm10="..string.format("%.2f",pm 10).."&pm25="..string.format("%.2f",pm25)
    data = "aria,node=Firenze::Lippi pm10="..string.format("%.2f",pm10)..",pm25="..string.format("%.2f",pm25)
    print ("dati "..data)
--    request = "POST /ninuxaria/index_post.php".." HTTP/1.1\r\n"..
    request = "POST /write?db=db1".." HTTP/1.1\r\n"..
--      "Host: 10.150.28.10.\r\n"..
      "Host: 10.150.13.3:8086\r\n"..
      "Accept: */*\r\n"..
 --     "apiKey: e2sss3af-9ssd-43b0-bfdd-24a1dssssc46\r\n"..
--      "Cache-Control: no-cache\r\n"..
      "Content-Length: "..string.len(data).."\r\n"..
      "Content-Type: application/x-www-form-urlencoded\r\n\r\n"..data
    return request
end
--POST /esppost.php HTTP/1.0

--Host: serverconnect.site88.net

--Accept: */*

--Content-Length: "name1=value1&name2=value2".Length

--Content-Type: application/x-www-form-urlencoded


--name1=value1&name2=value2
function to_remote ()
    print (m.." / "..minuti)
    if m >= minuti then
        m=0
        a10 = 0
        a25 = 0
        for i=1,cmax do
            a10=a10+Vpm10[i]  --prova
            a25=a25+Vpm25[i] --ripeova
--            print (a10.."  "..Vpm10[i])
        end 
        print ("     PM10 = "..a10.."   PM2,5 = "..a25)
        register_post(a10,a25)
   end
    m=m+1
end

local function test()
    print ("Sono vivo")
end
-- ********************
-- Invio dati al Server
-- ********************
function register (pm10,pm25)
    print ("Spedizione")
    conn=net.createConnection(net.TCP, 0)
    conn:on("receive", function(conn, payload) end)
-- conn:connect(80,"172.16.1.110")
    conn:connect(80,host)
    conn:send("GET /ninuxaria/index.php?t=100&pm10="..string.format("%.2f",pm10).."&pm25="..string.format("%.2f",pm25)..
    " HTTP/1.1\r\nHost: 172.16.1.110\r\n".."Connection: keep-alive\r\nAccept: */*\r\n\r\n")
    conn:on("disconnection", function(conn, payload) print('\nDisconnected') end)
end
-- ********************
--  Calcolo PWM PM10
-- ********************
--          __             __
--   ______|  |___________|  |_______
--
function pin10u()
    pulseT = tmr.now()
    gpio.trig(1,"down",pin10d)
end
-- **********************************
function pin10d()
    pulse1= tmr.now() -- Time in ms 
    if (pulse1-pulseT) >0 then
        c10=c10+1
        Vpm10[c10]= (((pulse1 - pulseT)/1000)-2)/cmax     -- Valore in ug/m3
--        print ("PM10("..c10..")= "..((pulse1 - pulseT)/1000-2)) 
        if (c10 >= cmax) then c10=0 end
    end
    gpio.trig(1,"up",pin10u) -- Cavetto Bianco
end
-- ********************
--  Calcolo PWM PM2.5
-- ********************
--          __             __
--   ______|  |___________|  |_______
--
function pin25u()
    pulseT25 = tmr.now()
    gpio.trig(2,"down",pin25d)

end
-- *************************************
function pin25d()
    pulse25= tmr.now() -- Time in ms 
    if (pulse25-pulseT25) >0 then
        c25=c25+1
        Vpm25[c25]= (((pulse25 - pulseT25)/1000)-2)/cmax     -- Valore in ug/m3
--        print ("PM2,5("..c25.."=)"..((pulse25 - pulseT25)/1000-2)) 
        if (c25 >= cmax) then c25=0 end
    end
    gpio.trig(2,"up",pin25u) -- Cavetto Blu
end
-- ******************************************************
--                    Definition Part
-- ******************************************************
host="10.150.13.3"  -- Remote Host to sent data
meantime=60*1000     -- Mean time to evaluate PM minutesx60x1000
minuti= 1
m=1
--horair_constant=3600000/(meantime*minuti) 
print ("Indirizzo Server "..host.."  Calcolo Valori medi su  "..minuti.." minuti")
--  Init part
Vpm10={}
Vpm25={}
c10=0
c25=0
a10 = 0
a25 = 0
cmax= minuti*60
gpio.mode(1,gpio.INT,gpio.PULLUP)
gpio.trig(1,"up",pin10u) -- Cavetto Bianco
--gpio.trig(1,"down",pin10d)
gpio.mode(2,gpio.INT,gpio.PULLUP)
gpio.trig(2,"up",pin25u) -- Cavetto Blu
--gpio.trig(2,"down",pin25u)
tmr.alarm(1, meantime, tmr.ALARM_AUTO, to_remote)
--tmr.alarm(2,500,tmr.ALARM_AUTO,test)
