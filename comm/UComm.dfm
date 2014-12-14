object FComm: TFComm
  Left = 783
  Top = 211
  Width = 235
  Height = 64
  Caption = 'FComm'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object ClientSocket: TClientSocket
    Active = False
    ClientType = ctNonBlocking
    Port = 0
    OnConnecting = ClientSocketConnecting
    OnConnect = ClientSocketConnect
    OnDisconnect = ClientSocketDisconnect
    OnRead = ClientSocketRead
    OnError = ClientSocketError
    Left = 32
  end
  object tmrRecv: TTimer
    Interval = 10
    OnTimer = tmrRecvTimer
  end
  object CheckConnectTmr: TTimer
    Enabled = False
    OnTimer = CheckConnectTmrTimer
    Left = 122
  end
  object YbCommDevice1: TYbCommDevice
    PortNo = 2
    Baud = br115200
    ByteSize = 8
    InBufSize = 8192
    OutBufSize = 8192
    HwInSize = 1200
    HwOutSize = 1200
    QueueSize = 16
    PackageSize = 4096
    Left = 90
  end
  object ServerSocket: TServerSocket
    Active = False
    Port = 0
    ServerType = stNonBlocking
    OnClientConnect = ServerSocketClientConnect
    OnClientDisconnect = ServerSocketClientDisconnect
    OnClientRead = ClientSocketRead
    OnClientError = ClientSocketError
    Left = 61
  end
  object IdUDPServer1: TIdUDPServer
    Bindings = <
      item
        Port = 10090
      end>
    DefaultPort = 0
    OnUDPRead = IdUDPServer1UDPRead
    Left = 152
  end
  object IdUDPClient1: TIdUDPClient
    Port = 0
    Left = 184
  end
end
