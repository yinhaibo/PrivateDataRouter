object CommDev_Setting: TCommDev_Setting
  Left = 330
  Top = 233
  BorderStyle = bsDialog
  Caption = 'Serial Config'
  ClientHeight = 228
  ClientWidth = 389
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnActivate = FormActivate
  PixelsPerInch = 96
  TextHeight = 13
  object GBoxBuffer: TGroupBox
    Left = 200
    Top = 104
    Width = 185
    Height = 89
    Caption = 'Buffer'
    TabOrder = 0
    object lbInbs: TLabel
      Left = 8
      Top = 24
      Width = 56
      Height = 13
      AutoSize = False
      Caption = 'Input Buffer'
    end
    object lbOutbs: TLabel
      Left = 8
      Top = 56
      Width = 56
      Height = 13
      AutoSize = False
      Caption = 'Output Buffer'
    end
    object EditInBufSize: TEdit
      Left = 64
      Top = 20
      Width = 105
      Height = 21
      TabOrder = 0
      Text = 'EditInBufSize'
    end
    object EditOutBufSize: TEdit
      Left = 64
      Top = 52
      Width = 105
      Height = 21
      TabOrder = 1
      Text = 'EditOutBufSize'
    end
  end
  object BnDefault: TBitBtn
    Left = 152
    Top = 200
    Width = 75
    Height = 25
    Cursor = crHandPoint
    Caption = '&Default'
    TabOrder = 1
    OnClick = BnDefaultClick
    Glyph.Data = {
      DE010000424DDE01000000000000760000002800000024000000120000000100
      0400000000006801000000000000000000001000000000000000000000000000
      80000080000000808000800000008000800080800000C0C0C000808080000000
      FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00333333444444
      33333333333F8888883F33330000324334222222443333388F3833333388F333
      000032244222222222433338F8833FFFFF338F3300003222222AAAAA22243338
      F333F88888F338F30000322222A33333A2224338F33F8333338F338F00003222
      223333333A224338F33833333338F38F00003222222333333A444338FFFF8F33
      3338888300003AAAAAAA33333333333888888833333333330000333333333333
      333333333333333333FFFFFF000033333333333344444433FFFF333333888888
      00003A444333333A22222438888F333338F3333800003A2243333333A2222438
      F38F333333833338000033A224333334422224338338FFFFF8833338000033A2
      22444442222224338F3388888333FF380000333A2222222222AA243338FF3333
      33FF88F800003333AA222222AA33A3333388FFFFFF8833830000333333AAAAAA
      3333333333338888883333330000333333333333333333333333333333333333
      0000}
    NumGlyphs = 2
    Spacing = 6
  end
  object BnOK: TBitBtn
    Left = 232
    Top = 200
    Width = 75
    Height = 25
    Cursor = crHandPoint
    Caption = '&OK'
    Default = True
    TabOrder = 2
    OnClick = BnOKClick
    Glyph.Data = {
      F6000000424DF600000000000000760000002800000010000000100000000100
      04000000000080000000CE0E0000C40E00001000000000000000000000000000
      80000080000000808000800000008000800080800000C0C0C000808080000000
      FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00777788887777
      77777778888887777777778444448877777777422224488777777A2222224488
      7777A222222224487777A222AAA222488777A22488AA22448877A224877AA224
      4887AAA27777AA224488777777777AA224487777777777AA224877777777777A
      A224777777777777AA227777777777777AA27777777777777777}
    Spacing = 6
  end
  object BnCancel: TBitBtn
    Left = 312
    Top = 200
    Width = 75
    Height = 25
    Cursor = crHandPoint
    Cancel = True
    Caption = '&Cancel'
    TabOrder = 3
    OnClick = BnCancelClick
    Glyph.Data = {
      F6000000424DF600000000000000760000002800000010000000100000000100
      04000000000080000000CE0E0000C40E00001000000000000000000000000000
      8000008000000080800080000000800080008080000080808000C0C0C0000000
      FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00887788888888
      8888891178888897888889111788891178888911117891111788889111171111
      1788888911111111788888889111111788888888811111788888888889111178
      8888888891111178888888891117111788888891117891117888889117888911
      1788888918888891117888888888888919888888888888888888}
    Spacing = 6
  end
  object GboxBase: TGroupBox
    Left = 8
    Top = 8
    Width = 185
    Height = 185
    Caption = 'Port Setting'
    TabOrder = 4
    object lbPort: TLabel
      Left = 8
      Top = 28
      Width = 50
      Height = 13
      AutoSize = False
      Caption = 'Port'
    end
    object lbBaud: TLabel
      Left = 8
      Top = 60
      Width = 50
      Height = 13
      AutoSize = False
      Caption = 'Baud'
    end
    object lbCheck: TLabel
      Left = 8
      Top = 92
      Width = 50
      Height = 13
      AutoSize = False
      Caption = 'Verify'
    end
    object lbBits: TLabel
      Left = 8
      Top = 124
      Width = 50
      Height = 13
      AutoSize = False
      Caption = 'Data Bit'
    end
    object lbStops: TLabel
      Left = 8
      Top = 156
      Width = 50
      Height = 13
      AutoSize = False
      Caption = 'Stop Bit'
    end
    object CbStopBits: TComboBox
      Left = 64
      Top = 152
      Width = 105
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      TabOrder = 4
      Items.Strings = (
        '1'
        '1.5'
        '2')
    end
    object CbSelBaud: TComboBox
      Left = 64
      Top = 56
      Width = 105
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      TabOrder = 1
      Items.Strings = (
        '110'
        '300'
        '600'
        '1200'
        '2400'
        '4800'
        '9600'
        '14400'
        '19200'
        '38400'
        '56000'
        '57600'
        '115200'
        '128000'
        '256000')
    end
    object CbParity: TComboBox
      Left = 64
      Top = 88
      Width = 105
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      TabOrder = 2
      Items.Strings = (
        'No parity'
        'Odd'
        'Even'
        'Mark'
        'Space')
    end
    object CbByteSize: TComboBox
      Left = 64
      Top = 120
      Width = 105
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      TabOrder = 3
      Items.Strings = (
        '5'
        '6'
        '7'
        '8')
    end
    object CbPort: TComboBox
      Left = 64
      Top = 24
      Width = 105
      Height = 21
      ItemHeight = 13
      TabOrder = 0
      Items.Strings = (
        'COM1'
        'COM2'
        'COM3'
        'COM4'
        'COM5'
        'COM6'
        'COM7'
        'COM8'
        'COM9'
        'COM10'
        'COM11'
        'COM12')
    end
  end
  object GBoxModem: TGroupBox
    Left = 200
    Top = 8
    Width = 185
    Height = 89
    Caption = 'Modern Setting'
    TabOrder = 5
    object lbFlow: TLabel
      Left = 8
      Top = 28
      Width = 56
      Height = 13
      AutoSize = False
      Caption = 'Flow Control'
    end
    object lbAAns: TLabel
      Left = 8
      Top = 60
      Width = 56
      Height = 13
      AutoSize = False
      Caption = 'Auto Response'
    end
    object CbFlow: TComboBox
      Left = 64
      Top = 24
      Width = 105
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      TabOrder = 0
      Items.Strings = (
        'None'
        'RTS/CTS'
        'Xon/Xoff'
        'RTS/CTS & Xon/Xoff')
    end
    object EditAutoAns: TEdit
      Left = 64
      Top = 56
      Width = 105
      Height = 21
      TabOrder = 1
      Text = 'EditAutoAns'
    end
  end
end
