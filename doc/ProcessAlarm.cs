  protected void ProcessAlarm()
    {
      TcpClient tcpClient = new TcpClient();
      GPS gps1 = new GPS();
      string tmpID = "";
      try
      {
        byte[] numArray = new byte[25601];
        tcpClient = this.GPSListener.AcceptTcpClient();
        tcpClient.ReceiveTimeout = 90000;
        tcpClient.ReceiveBufferSize = 25600;
        NetworkStream stream = tcpClient.GetStream();
        DateTime now = DateTime.Now;
        int num1 = 0;
        bool flag1 = false;
        bool flag2 = false;
        while (tcpClient.Connected)
        {
          if (Microsoft.VisualBasic.CompilerServices.Operators.CompareString(now.AddSeconds(90000.0).ToString("yyyy-MM-dd HH:mm:ss"), DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss"), false) < 0)
            break;
          try
          {
            if (!string.IsNullOrEmpty(gps1.IMEI) & gps1.ImagePackageTimeOut)
            {
              if (num1 > 2)
              {
                gps1.Dispose();
                gps1 = new GPS();
                gps1.ImageSavePath = Application.StartupPath + "\\AlarmImages\\";
                num1 = 0;
                flag1 = false;
              }
              else
              {
                byte[] bytes = Encoding.ASCII.GetBytes("**,imei:" + gps1.IMEI + ",R");
                stream.Write(bytes, 0, bytes.Length);
                gps1.ImagePackageReceiveStartTime = DateTime.Now;
                checked { ++num1; }
              }
            }
          }
          catch (Exception ex)
          {
            ProjectData.SetProjectError(ex);
            mdlMain.WriteLog("接收图像数据超时出错(ProcessGPSData)", ex.Message);
            ProjectData.ClearProjectError();
          }
          int count = stream.Read(numArray, 0, numArray.Length);
          if (count < 1)
            break;
          string[] strArray1 = Strings.Split(Encoding.ASCII.GetString(numArray, 0, count), ";", -1, CompareMethod.Binary);
          int num2 = 0;
          int num3 = checked (Information.UBound((Array) strArray1, 1) - 1);
          int index = num2;
          while (index <= num3)
          {
            now = DateTime.Now;
            strArray1[index] = Strings.Replace(strArray1[index], "\n", "", 1, -1, CompareMethod.Binary);
            if (!true)
              return;
            string str1 = strArray1[index];
            string tmpDate = "";
            string str2 = "";
            string tmpTEL = "";
            double tmpSpeed = 0.0;
            double tmpDirection = 0.0;
            double tmpAltitude = 0.0;
            double tmpOil = 0.0;
            double tmpOil2 = 0.0;
            double tmpTemperature = 0.0;
            bool flag3 = false;
            bool flag4 = false;
            bool flag5 = false;
            string str3 = "";
            string str4 = "";
            try
            {
              string[] strArray2 = str1.Split(',');
              double Lat;
              double Lng;
              double num4;
              double num5;
              if (strArray2.Length == 6 | strArray2.Length >= 12)
              {
                try
                {
                  tmpID = Strings.Trim(Strings.Replace(strArray2[0], "imei:", "", 1, -1, CompareMethod.Binary));
                  str2 = Strings.RTrim(strArray2[1]);
                  tmpTEL = strArray2[3];
                  if (strArray2.Length >= 12)
                  {
                    try
                    {
                      string str5 = "20" + strArray2[2].Substring(0, 2) + "-" + strArray2[2].Substring(2, 2) + "-" + strArray2[2].Substring(4, 2) + " " + strArray2[2].Substring(6, 2) + ":" + strArray2[2].Substring(8, 2) + ":00";
                      tmpDate = Conversions.ToString(Interaction.IIf(Information.IsDate((object) str5), (object) Conversions.ToDate(str5).ToString("yyyy-MM-dd HH:mm:ss"), (object) Strings.Format((object) DateAndTime.Now, "yyyy-MM-dd HH:mm:ss")));
                    }
                    catch (Exception ex)
                    {
                      ProjectData.SetProjectError(ex);
                      tmpDate = Strings.Format((object) DateAndTime.Now, "yyyy-MM-dd HH:mm:ss");
                      ProjectData.ClearProjectError();
                    }
                    try
                    {
                      Lat = !string.IsNullOrEmpty(strArray2[8].ToString()) ? Conversions.ToDouble(Interaction.IIf(Microsoft.VisualBasic.CompilerServices.Operators.CompareString(strArray2[8].ToUpper(), "S", false) == 0 | Microsoft.VisualBasic.CompilerServices.Operators.CompareString(strArray2[8].ToUpper(), "W", false) == 0, (object) -mdlMain.ConvertGPSPoint(Conversion.Val(strArray2[7])), (object) mdlMain.ConvertGPSPoint(Conversion.Val(strArray2[7])))) : 0.0;
                    }
                    catch (Exception ex)
                    {
                      ProjectData.SetProjectError(ex);
                      Lat = 0.0;
                      ProjectData.ClearProjectError();
                    }
                    try
                    {
                      Lng = !string.IsNullOrEmpty(strArray2[10].ToString()) ? Conversions.ToDouble(Interaction.IIf(Microsoft.VisualBasic.CompilerServices.Operators.CompareString(strArray2[10].ToUpper(), "S", false) == 0 | Microsoft.VisualBasic.CompilerServices.Operators.CompareString(strArray2[10].ToUpper(), "W", false) == 0, (object) -mdlMain.ConvertGPSPoint(Conversion.Val(strArray2[9])), (object) mdlMain.ConvertGPSPoint(Conversion.Val(strArray2[9])))) : 0.0;
                    }
                    catch (Exception ex)
                    {
                      ProjectData.SetProjectError(ex);
                      Lng = 0.0;
                      ProjectData.ClearProjectError();
                    }
                    try
                    {
                      if (!string.IsNullOrEmpty(strArray2[7].ToString()) & string.IsNullOrEmpty(strArray2[8].ToString()) & !string.IsNullOrEmpty(strArray2[9].ToString()) & string.IsNullOrEmpty(strArray2[10].ToString()))
                      {
                        uint lac = Convert.ToUInt32("0x" + strArray2[7].ToUpper(), 16);
                        mdlMain.LocationBasedService(Convert.ToUInt32("0x" + strArray2[9].ToUpper(), 16), lac, ref Lat, ref Lng);
                        flag3 = true;
                      }
                    }
                    catch (Exception ex)
                    {
                      ProjectData.SetProjectError(ex);
                      Lat = 0.0;
                      Lng = 0.0;
                      flag3 = true;
                      ProjectData.ClearProjectError();
                    }
                    try
                    {
                      tmpSpeed = Conversion.Val(strArray2[11]) * 1.85 >= 5.0 ? Conversion.Val(strArray2[11]) * 1.85 : 0.0;
                    }
                    catch (Exception ex)
                    {
                      ProjectData.SetProjectError(ex);
                      tmpSpeed = 0.0;
                      ProjectData.ClearProjectError();
                    }
                    try
                    {
                      tmpDirection = !string.IsNullOrEmpty(strArray2[12]) ? Conversion.Val(strArray2[12]) : 0.0;
                    }
                    catch (Exception ex)
                    {
                      ProjectData.SetProjectError(ex);
                      tmpDirection = 0.0;
                      ProjectData.ClearProjectError();
                    }
                    try
                    {
                      tmpAltitude = !string.IsNullOrEmpty(strArray2[13]) ? Conversion.Val(strArray2[13]) : 0.0;
                    }
                    catch (Exception ex)
                    {
                      ProjectData.SetProjectError(ex);
                      tmpAltitude = 0.0;
                      ProjectData.ClearProjectError();
                    }
                    try
                    {
                      if (str2.ToUpper().Contains("TRACKER") & str2.Length > "TRACKER".Length)
                      {
                        object[] objArray = (object[]) str2.ToUpper().Replace("TRACKER ", "").Split(' ');
                        if (objArray.Length > 0)
                          tmpOil = Conversions.ToDouble(NewLateBinding.LateGet(objArray[0], (System.Type) null, "Replace", new object[2]
                          {
                            (object) "%",
                            (object) ""
                          }, (string[]) null, (System.Type[]) null, (bool[]) null));
                        else
                          tmpOil = 0.0;
                        if (objArray.Length > 1)
                          tmpTemperature = Conversions.ToDouble(NewLateBinding.LateGet(objArray[1], (System.Type) null, "Replace", new object[2]
                          {
                            (object) "+",
                            (object) ""
                          }, (string[]) null, (System.Type[]) null, (bool[]) null));
                        else
                          tmpTemperature = 0.0;
                        str2 = "TRACKER";
                      }
                    }
                    catch (Exception ex)
                    {
                      ProjectData.SetProjectError(ex);
                      str2 = "TRACKER";
                      ProjectData.ClearProjectError();
                    }
                    try
                    {
                      if (str2.ToUpper().Contains("OIL") & str2.Length > "OIL".Length)
                      {
                        tmpOil = Conversions.ToDouble(str2.ToUpper().Replace("OIL ", "").Replace("%", ""));
                        str2 = "OIL";
                      }
                    }
                    catch (Exception ex)
                    {
                      ProjectData.SetProjectError(ex);
                      str2 = "OIL";
                      ProjectData.ClearProjectError();
                    }
                    try
                    {
                      if (str2.ToUpper().Contains("T:") & str2.Length > "T:".Length)
                      {
                        tmpTemperature = Conversions.ToDouble(str2.ToUpper().Replace("T:", ""));
                        str2 = "T:";
                      }
                    }
                    catch (Exception ex)
                    {
                      ProjectData.SetProjectError(ex);
                      str2 = "T:";
                      ProjectData.ClearProjectError();
                    }
                    if (strArray2.Length >= 18)
                    {
                      flag4 = Conversions.ToBoolean(strArray2[14]);
                      flag5 = Conversions.ToBoolean(strArray2[15]);
                      try
                      {
                        tmpOil = !string.IsNullOrEmpty(strArray2[16]) ? Conversion.Val(strArray2[16].Replace("%", "")) : 0.0;
                      }
                      catch (Exception ex)
                      {
                        ProjectData.SetProjectError(ex);
                        tmpOil = 0.0;
                        ProjectData.ClearProjectError();
                      }
                      try
                      {
                        tmpOil2 = !string.IsNullOrEmpty(strArray2[17]) ? Conversion.Val(strArray2[17].Replace("%", "")) : 0.0;
                      }
                      catch (Exception ex)
                      {
                        ProjectData.SetProjectError(ex);
                        tmpOil2 = 0.0;
                        ProjectData.ClearProjectError();
                      }
                      try
                      {
                        tmpTemperature = !string.IsNullOrEmpty(strArray2[18]) ? Conversion.Val(strArray2[18].Replace("+", "")) : 0.0;
                      }
                      catch (Exception ex)
                      {
                        ProjectData.SetProjectError(ex);
                        tmpTemperature = 0.0;
                        ProjectData.ClearProjectError();
                      }
                    }
                    if (Microsoft.VisualBasic.CompilerServices.Operators.CompareString(str2.ToUpper(), "WT", false) == 0)
                      str4 = Strings.Left(str2, checked (str2.Length - 2));
                    if (Microsoft.VisualBasic.CompilerServices.Operators.CompareString(Strings.Right(str2, 2).ToUpper(), "IN", false) == 0)
                    {
                      str4 = Strings.Left(str2, checked (str2.Length - 2));
                      str2 = "IN";
                    }
                    if (Microsoft.VisualBasic.CompilerServices.Operators.CompareString(Strings.Right(str2, 3).ToUpper(), "OUT", false) == 0)
                    {
                      str4 = Strings.Left(str2, checked (str2.Length - 3));
                      str2 = "OUT";
                    }
                    if (Microsoft.VisualBasic.CompilerServices.Operators.CompareString(str2.ToUpper(), "RFID", false) == 0)
                    {
                      tmpTEL = "";
                      str4 = strArray2[3];
                    }
                  }
                  else
                  {
                    try
                    {
                      string str5 = "20" + strArray2[2].Substring(0, 2) + "-" + strArray2[2].Substring(2, 2) + "-" + strArray2[2].Substring(4, 2) + " " + strArray2[2].Substring(6, 2) + ":" + strArray2[2].Substring(8, 2) + ":00";
                      tmpDate = Conversions.ToString(Interaction.IIf(Information.IsDate((object) str5), (object) Conversions.ToDate(str5).ToString("yyyy-MM-dd HH:mm:ss"), (object) Strings.Format((object) DateAndTime.Now, "yyyy-MM-dd HH:mm:ss")));
                    }
                    catch (Exception ex)
                    {
                      ProjectData.SetProjectError(ex);
                      tmpDate = Strings.Format((object) DateAndTime.Now, "yyyy-MM-dd HH:mm:ss");
                      ProjectData.ClearProjectError();
                    }
                    Lat = 0.0;
                    Lng = 0.0;
                    tmpSpeed = 0.0;
                    tmpDirection = 0.0;
                    tmpAltitude = 0.0;
                    tmpOil = 0.0;
                    tmpOil2 = 0.0;
                    tmpTemperature = 0.0;
                  }
                }
                catch (Exception ex)
                {
                  ProjectData.SetProjectError(ex);
                  Exception exception = ex;
                  if (Information.Err().Number != 5 & Information.Err().Number != 57)
                    mdlMain.WriteLog("(ProcessGPSData)(Process Alarm Message)", exception.Message + "[" + Information.Err().Number.ToString() + "] 数据内容为：" + strArray1[index].ToString());
                  ProjectData.ClearProjectError();
                }
                num4 = Lat;
                num5 = Lng;
                if (Microsoft.VisualBasic.CompilerServices.Operators.CompareString(str2.ToUpper(), "TRACKER", false) == 0 & tmpDirection == 1.0 | str2.ToUpper().Contains("VT") & Microsoft.VisualBasic.CompilerServices.Operators.CompareString(str2.ToUpper(), "VT", false) != 0)
                {
                  DataTable dataTable = new DataTable();
                  if (mdlDB.GetDataSet("Select * From Point Where PointID='" + mdlDB.SQLChar((object) tmpID) + "'").Tables[0].Rows.Count > 0)
                    dataTable = (DataTable) null;
                  else
                    break;
                }
                if (Microsoft.VisualBasic.CompilerServices.Operators.CompareString(str2.ToUpper(), "TRACKER", false) == 0 & tmpDirection == 1.0)
                {
                  string str5 = "";
                  try
                  {
                    if (Lat != 0.0 & Lng != 0.0)
                      str5 = mdlMain.GetGoogleMapAddress(num4, num5);
                  }
                  catch (Exception ex)
                  {
                    ProjectData.SetProjectError(ex);
                    ProjectData.ClearProjectError();
                  }
                  string s;
                  if (Lat == 0.0 & Lng == 0.0)
                    s = "address2,no gps signal,lat:,long:,T:,Speed:";
                  else if (str5.IndexOf("中国") >= 0)
                  {
                    if (flag3)
                      str5 += "(基站定位)";
                    string str6 = "";
                    int num6 = 0;
                    int num7 = checked (str5.Length - 1);
                    int startIndex = num6;
                    while (startIndex <= num7)
                    {
                      string str7 = Conversion.Hex(Strings.AscW(str5.Substring(startIndex, 1))).ToUpper();
                      string str8 = ("0000" + str7).Substring(checked (("0000" + str7).Length - 4), 4);
                      str6 += str8;
                      checked { ++startIndex; }
                    }
                    s = "address1," + str6 + ";lat:" + Lat.ToString("#,###.000000") + " long:" + Lng.ToString("#,###.000000") + ",T:" + string.Format("{0:yyyy-MM-dd HH:mm}", (object) Conversions.ToDate(tmpDate)) + ",Speed:" + Conversions.ToString(tmpSpeed);
                  }
                  else
                  {
                    if (flag3)
                      str5 += "(LBS)";
                    s = "address2," + str5 + ";lat:" + Lat.ToString("#,###.000000") + " long:" + Lng.ToString("#,###.000000") + ",T:" + string.Format("{0:yyyy-MM-dd HH:mm}", (object) Conversions.ToDate(tmpDate)) + ",Speed:" + Conversions.ToString(tmpSpeed);
                  }
                  byte[] bytes = Encoding.Default.GetBytes(s);
                  stream.Write(bytes, 0, bytes.Length);
                }
                if (tmpDirection == 1.0)
                  tmpDirection = 0.0;
                if (str2.ToUpper().Contains("VT") | str2.ToUpper().Contains("WT") & Microsoft.VisualBasic.CompilerServices.Operators.CompareString(str2.ToUpper(), "WT", false) != 0)
                {
                  gps1.Dispose();
                  gps1 = new GPS();
                  gps1.ImageSavePath = Application.StartupPath + "\\AlarmImages\\";
                  num1 = 0;
                  flag1 = false;
                  tmpID = Strings.Trim(Strings.Replace(strArray2[0], "imei:", "", 1, -1, CompareMethod.Binary));
                  gps1.IMEI = tmpID;
                  gps1.AlarmCase = "VR";
                  gps1.AlarmTime = tmpDate;
                  gps1.MapX = num4;
                  gps1.MapY = num5;
                  gps1.LocationX = Lat;
                  gps1.LocationY = Lng;
                  gps1.Speed = tmpSpeed;
                  gps1.Direction = tmpDirection;
                  gps1.Altitude = tmpAltitude;
                  gps1.Oil = tmpOil;
                  gps1.Oil2 = tmpOil2;
                  gps1.Temperature = tmpTemperature;
                  gps1.LBS = flag3;
                  gps1.ACCStatus = Conversions.ToString(flag4);
                  gps1.DoorStatus = Conversions.ToString(flag5);
                  gps1.TEL = tmpTEL;
                  gps1.ImagePackageReceivedNum = 0;
                  gps1.ImagePackageCount = checked ((int) Math.Round(Conversion.Val(str2.ToUpper().Replace(Conversions.ToString(Interaction.IIf(str2.ToUpper().Contains("WT"), (object) "WT", (object) "VT")), ""))));
                  gps1.ImagePackageReceiveStartTime = DateTime.Now;
                  gps1.ImagePackageData = "";
                  gps1.ReturnURLToGPS = Conversions.ToBoolean(Interaction.IIf(str2.ToUpper().Contains("VT"), (object) true, (object) false));
                }
                if (!string.IsNullOrEmpty(tmpID) & (Microsoft.VisualBasic.CompilerServices.Operators.CompareString(Strings.Left(str2.ToUpper(), 2), "VT", false) != 0 & Microsoft.VisualBasic.CompilerServices.Operators.CompareString(Strings.Left(str2.ToUpper(), 2), "VR", false) != 0 & Microsoft.VisualBasic.CompilerServices.Operators.CompareString(Strings.Left(str2.ToUpper(), 2), "WT", false) != 0 | Microsoft.VisualBasic.CompilerServices.Operators.CompareString(str2.ToUpper(), "WT", false) == 0))
                {
                  flag2 = true;
                  this.Alarm(tmpDate, tmpID, str2, num4, num5, Lat, Lng, tmpTEL, tmpSpeed, tmpDirection, tmpAltitude, tmpOil, tmpOil2, tmpTemperature, "", flag3, flag4, flag5);
                }
              }
              else
              {
                if (strArray2.Length == 1)
                  tmpID = Strings.Trim(Strings.Replace(strArray2[0], "imei:", "", 1, -1, CompareMethod.Binary));
                if (strArray2.Length == 3)
                  tmpID = Strings.Trim(Strings.Replace(strArray2[1], "imei:", "", 1, -1, CompareMethod.Binary));
                if (strArray2.Length == 5)
                  tmpID = Strings.Trim(Strings.Replace(strArray2[0], "imei:", "", 1, -1, CompareMethod.Binary));
                if (strArray2.Length <= 4)
                {
                  if (Microsoft.VisualBasic.CompilerServices.Operators.CompareString(strArray2[0].ToUpper().ToString(), "##", false) == 0)
                  {
                    byte[] bytes = Encoding.ASCII.GetBytes("LOAD");
                    stream.Write(bytes, 0, bytes.Length);
                  }
                  else if (strArray2.Length == 1 && strArray2[0].Length == 15 & Versioned.IsNumeric((object) strArray2[0]))
                  {
                    byte[] bytes = Encoding.ASCII.GetBytes("ON");
                    stream.Write(bytes, 0, bytes.Length);
                  }
                  try
                  {
                    if (strArray2.Length == 3)
                    {
                      now = DateTime.Now;
                      if (Microsoft.VisualBasic.CompilerServices.Operators.CompareString(strArray2[1].ToUpper().ToString(), "VR", false) == 0 & strArray2[2].Length >= 12)
                      {
                        tmpID = Strings.Trim(Strings.Replace(strArray2[0], "imei:", "", 1, -1, CompareMethod.Binary));
                        gps1.IMEI = tmpID;
                        if (gps1.ImagePackageReceivedNum == Convert.ToInt32(strArray2[2].Substring(0, 2), 16))
                        {
                          gps1.ImagePackageReceiveStartTime = DateTime.Now;
                          GPS gps2 = gps1;
                          gps2.ImagePackageData = gps2.ImagePackageData + strArray2[2].Substring(8, checked (strArray2[2].Length - 12));
                          gps1.ImagePackageReceivedNum = checked (gps1.ImagePackageReceivedNum + 1);
                          flag1 = false;
                        }
                        else if (!flag1)
                        {
                          byte[] bytes = Encoding.ASCII.GetBytes("**,imei:" + gps1.IMEI + ",R");
                          stream.Write(bytes, 0, bytes.Length);
                          flag1 = true;
                        }
                        if (gps1.ImagePackageReceivedNum > checked (gps1.ImagePackageCount - 1))
                        {
                          GPS gps2 = gps1;
                          string str5 = gps2.Save();
                          if (!string.IsNullOrEmpty(str5))
                          {
                            gps2.ImageName = Application.StartupPath + "\\AlarmImages\\" + str5;
                            str3 = gps2.ImageName;
                            flag2 = true;
                            this.Alarm(gps2.AlarmTime, gps2.IMEI, gps2.AlarmCase, gps2.MapX, gps2.MapY, gps2.LocationX, gps2.LocationY, gps2.TEL, gps2.Speed, gps2.Direction, gps2.Altitude, gps2.Oil, gps2.Oil2, gps2.Temperature, gps2.ImageName, gps2.LBS, Conversions.ToBoolean(gps2.ACCStatus), Conversions.ToBoolean(gps2.DoorStatus));
                            if (gps2.ReturnURLToGPS)
                            {
                              try
                              {
                                byte[] bytes = Encoding.ASCII.GetBytes("photo, " + gps2.AlarmCase + ": lat:" + gps2.LocationX.ToString().Replace(",", ".") + " long:" + gps2.LocationY.ToString().Replace(",", ".") + " Speed:" + gps2.Speed.ToString().Replace(",", ".") + " T:" + gps2.AlarmTime + " IMEI:" + gps2.IMEI + " " + gps2.ImageName);
                                stream.Write(bytes, 0, bytes.Length);
                              }
                              catch (Exception ex)
                              {
                                ProjectData.SetProjectError(ex);
                                ProjectData.ClearProjectError();
                              }
                            }
                          }
                          gps2.Dispose();
                          gps1 = new GPS();
                          gps1.ImageSavePath = Application.StartupPath + "\\AlarmImages\\";
                          num1 = 0;
                          flag1 = false;
                        }
                      }
                    }
                  }
                  catch (Exception ex)
                  {
                    ProjectData.SetProjectError(ex);
                    mdlMain.WriteLog("接收图像数据时出错(ProcessGPSData)", ex.Message + "  数据内容为：" + strArray1[index].ToString());
                    ProjectData.ClearProjectError();
                  }
                }
              }
              if (string.IsNullOrEmpty(Conversions.ToString(tmpDirection)))
                tmpDirection = 0.0;
              try
              {
                if (!flag2 & !string.IsNullOrEmpty(tmpID))
                {
                  if (Lat != 0.0 & Lng != 0.0)
                    mdlDB.ExecuteSQL(Conversions.ToString(Microsoft.VisualBasic.CompilerServices.Operators.ConcatenateObject(Microsoft.VisualBasic.CompilerServices.Operators.ConcatenateObject(Microsoft.VisualBasic.CompilerServices.Operators.ConcatenateObject(Microsoft.VisualBasic.CompilerServices.Operators.ConcatenateObject(Microsoft.VisualBasic.CompilerServices.Operators.ConcatenateObject(Microsoft.VisualBasic.CompilerServices.Operators.ConcatenateObject(Microsoft.VisualBasic.CompilerServices.Operators.ConcatenateObject(Microsoft.VisualBasic.CompilerServices.Operators.ConcatenateObject(Microsoft.VisualBasic.CompilerServices.Operators.ConcatenateObject(Microsoft.VisualBasic.CompilerServices.Operators.ConcatenateObject((object) ("Update [Point] Set MapX=" + num4.ToString("###0.000000").Replace(",", ".") + ",MapY=" + num5.ToString("###0.000000").Replace(",", ".") + ",LocationX=" + Lat.ToString("###0.000000").Replace(",", ".") + ",LocationY=" + Lng.ToString("###0.000000").Replace(",", ".") + ",Speed=" + Conversion.Val((object) tmpSpeed).ToString("###0.00").Replace(",", ".") + ",Direction=" + Conversion.Val((object) tmpDirection).ToString("###0.00").Replace(",", ".") + ",Altitude=" + Conversion.Val((object) tmpAltitude).ToString("###0.00").Replace(",", ".") + ",Oil=" + Conversion.Val((object) tmpOil).ToString("###0.00").Replace(",", ".") + ",Oil2=" + Conversion.Val((object) tmpOil2).ToString("###0.00").Replace(",", ".") + ",Temperature=" + Conversion.Val((object) tmpTemperature).ToString("###0.00").Replace(",", ".") + ",LBS="), Interaction.IIf(flag3, (object) 1, (object) 0)), (object) ",ACCStatus="), Interaction.IIf(flag4, (object) 1, (object) 0)), (object) ",DoorStatus="), Interaction.IIf(flag5, (object) 1, (object) 0)), (object) ",LastOnlineTime='"), (object) DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss")), (object) "' Where PointID='"), (object) tmpID), (object) "' ")));
                  else
                    mdlDB.ExecuteSQL("Update [Point] Set LastOnlineTime='" + DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss") + "' Where PointID='" + tmpID + "' ");
                }
              }
              catch (Exception ex)
              {
                ProjectData.SetProjectError(ex);
                Exception exception = ex;
                if (Information.Err().Number != 5 & Information.Err().Number != 57)
                  mdlMain.WriteLog("(ProcessGPSData)(Update GPS Online Time)", exception.Message + "[" + Information.Err().Number.ToString() + "]  " + strArray1[index].ToString() + ")");
                ProjectData.ClearProjectError();
              }
              try
              {
                if (this.hTable[(object) tmpID] == null)
                {
                  this.hTable.Add((object) tmpID, (object) tcpClient);
                }
                else
                {
                  this.hTable.Remove((object) tmpID);
                  this.hTable.Add((object) tmpID, (object) tcpClient);
                }
              }
              catch (Exception ex)
              {
                ProjectData.SetProjectError(ex);
                ProjectData.ClearProjectError();
              }
            }
            catch (Exception ex)
            {
              ProjectData.SetProjectError(ex);
              Exception exception = ex;
              if (Information.Err().Number != 5 & Information.Err().Number != 57)
                mdlMain.WriteLog("处理收到的数据时出错(ProcessGPSData)", exception.Message + "  数据内容为：" + strArray1[index].ToString());
              ProjectData.ClearProjectError();
            }
            flag2 = false;
            checked { ++index; }
          }
          if (!tcpClient.Connected)
            break;
          Thread.Sleep(1);
        }
      }
      catch (Exception ex)
      {
        ProjectData.SetProjectError(ex);
        Exception exception = ex;
        if (Information.Err().Number != 5 & Information.Err().Number != 57)
          mdlMain.WriteLog("(ProcessGPSData):", exception.Message.ToString() + "[" + Information.Err().Number.ToString() + "]");
        ProjectData.ClearProjectError();
      }
      finally
      {
        try
        {
          if (this.hTable[(object) tmpID] == null)
            this.hTable.Remove((object) tmpID);
        }
        catch (Exception ex)
        {
          ProjectData.SetProjectError(ex);
          ProjectData.ClearProjectError();
        }
        try
        {
          lock (Thread.CurrentThread)
          {
            if (gps1 != null)
              ;
            tcpClient.Close();
          }
        }
        catch (Exception ex)
        {
          ProjectData.SetProjectError(ex);
          ProjectData.ClearProjectError();
        }
      }
    }