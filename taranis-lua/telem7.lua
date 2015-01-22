local function init()
  time = 0
  bat_time = 0
  bat_alarm = 0
  USR_SNDS = "/SOUNDS/en/"
  SYS_SNDS = USR_SNDS .. "system/"
  BMP_DIR  = "/SCRIPTS/BMP/justice/"
end

local function background()
  time = getTime()
  local bat_prcnt = getValue("rpm")

  -- play an alarm every 10s if battery is < 20%
  if time > bat_alarm + 1000 and bat_prcnt > 0 then
    if bat_prcnt > 15 and bat_prcnt <= 20 then
      playFile(USR_SNDS .. "BATT LOW.wav")
    elseif bat_prcnt <= 15 then
      playFile(USR_SNDS .. "BATT CRT.wav")
    end
    bat_alarm = time
  end

  -- play the battery % remaining every 15s
  if ((time > bat_time + 1500) and bat_prcnt > 0) then
    playFile(SYS_SNDS .. "00" .. bat_prcnt .. ".wav")
    playFile(SYS_SNDS .. "0130.wav") -- percent
    bat_time = time
  end
end

local function draw_sat_bars()
  -- draw GPS icon
  lcd.drawPixmap(3, 22, BMP_DIR .. "gps.bmp")

  -- get number of satellites in view and compute image number to dislpay
  local sat = getValue("temp1")
  if sat == nil then sat = 0 end
  local sat_imgnum = sat - 1
  if sat_imgnum < 0 then sat_imgnum = 0 end
  if sat_imgnum > 5 then sat_imgnum = 5 end

  -- draw image number and display the correct gauge
  lcd.drawNumber(47, 22, sat, LEFT+DBLSIZE)
  lcd.drawPixmap(18, 22, BMP_DIR .. "gps_" .. sat_imgnum .. ".bmp")
end

local function draw_fix_bars()
  local fix_type = getValue("temp2")
  lcd.drawPixmap(60, 22, BMP_DIR .. "satfix_" .. fix_type .. ".bmp")
end

local function draw_battery()
  local percent = getValue("rpm")
  lcd.drawPixmap(2, 42, BMP_DIR .. "batt.bmp")
  if percent < 1 then
    lcd.drawText(10, 48, "Connect!", 1)
  end

  if percent > 1 then
    lcd.drawNumber(20, 46, percent, LEFT+MIDSIZE)
    lcd.drawText(lcd.getLastPos(), 46, "%", MIDSIZE)
    lcd.drawGauge(7, 45, 49, 14, percent, 100)
    lcd.drawChannel (75,46, ("cell-sum"), LEFT+MIDSIZE)
  end
end

local function draw_rssi()
  lcd.drawText(70, 21, "RSSI", SMLSIZE)
  lcd.drawNumber(100, 27, getValue("rssi"), MIDSIZE)
  lcd.drawText(101, 31, "dB", SMLSIZE)
end

local function run(event)
  background()

  -- draw lines for telemetry info
  lcd.drawLine(0, 20, 211, 20,SOLID,0)
  lcd.drawLine(0, 39, 69, 39,SOLID,0)
  lcd.drawLine(69, 39, 110, 39,SOLID,0)
  lcd.drawLine(67, 20, 67, 38,SOLID,0)
  lcd.drawLine(111, 20, 111, 64,SOLID,0)
  
  local timer = model.getTimer(0)
  lcd.drawPixmap(144, 2, BMP_DIR .. "timer_1.bmp")
  lcd.drawTimer(162, 2, timer.value, DBLSIZE)

  draw_sat_bars()
  draw_fix_bars()
  draw_battery()
  draw_rssi()
  
end

return { init=init, background=background, run=run }
