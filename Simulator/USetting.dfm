object DeviceSetting: TDeviceSetting
  Left = 683
  Top = 215
  BorderStyle = bsDialog
  Caption = 'Device Configure'
  ClientHeight = 259
  ClientWidth = 407
  Color = clBtnFace
  ParentFont = True
  OldCreateOrder = True
  Position = poScreenCenter
  PixelsPerInch = 96
  TextHeight = 13
  object Bevel1: TBevel
    Left = 8
    Top = 8
    Width = 393
    Height = 209
    Shape = bsFrame
  end
  object Label1: TLabel
    Left = 46
    Top = 16
    Width = 22
    Height = 13
    Caption = 'Seq:'
  end
  object Label2: TLabel
    Left = 43
    Top = 40
    Width = 25
    Height = 13
    Caption = 'Alias:'
  end
  object Label3: TLabel
    Left = 38
    Top = 88
    Width = 30
    Height = 13
    Caption = 'Mode:'
  end
  object Label4: TLabel
    Left = 20
    Top = 117
    Width = 48
    Height = 13
    Caption = 'Configure:'
  end
  object Label5: TLabel
    Left = 38
    Top = 138
    Width = 30
    Height = 13
    Caption = 'Delay:'
  end
  object Label7: TLabel
    Left = 76
    Top = 139
    Width = 26
    Height = 13
    Caption = 'From:'
  end
  object Label8: TLabel
    Left = 180
    Top = 140
    Width = 16
    Height = 13
    Caption = 'To:'
  end
  object Label11: TLabel
    Left = 19
    Top = 187
    Width = 82
    Height = 13
    Caption = 'Ending Message:'
  end
  object Label12: TLabel
    Left = 53
    Top = 166
    Width = 46
    Height = 13
    Caption = 'Message:'
  end
  object Label13: TLabel
    Left = 362
    Top = 165
    Width = 33
    Height = 13
    Caption = '(ASCII)'
  end
  object Label14: TLabel
    Left = 362
    Top = 188
    Width = 33
    Height = 13
    Caption = '(ASCII)'
  end
  object Label6: TLabel
    Left = 43
    Top = 64
    Width = 22
    Height = 13
    Caption = 'Tag:'
  end
  object txtSeq: TEdit
    Left = 72
    Top = 12
    Width = 73
    Height = 21
    Enabled = False
    TabOrder = 0
  end
  object txtAlias: TEdit
    Left = 72
    Top = 36
    Width = 153
    Height = 21
    TabOrder = 1
  end
  object cboMode: TComboBox
    Left = 72
    Top = 84
    Width = 105
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    TabOrder = 2
    Items.Strings = (
      'Serial port'
      'TCP Client')
  end
  object txtConfigure: TEdit
    Left = 72
    Top = 112
    Width = 241
    Height = 21
    Enabled = False
    TabOrder = 3
  end
  object btnSetting: TButton
    Left = 320
    Top = 112
    Width = 41
    Height = 21
    Caption = '...'
    TabOrder = 4
    OnClick = btnSettingClick
  end
  object txtDelayFrom: TEdit
    Left = 104
    Top = 136
    Width = 49
    Height = 21
    Hint = 'Dealy a random seconds between from and to value.'
    ParentShowHint = False
    ShowHint = True
    TabOrder = 5
    Text = '0'
  end
  object udDelayFrom: TUpDown
    Left = 153
    Top = 136
    Width = 16
    Height = 21
    Associate = txtDelayFrom
    Min = 0
    Max = 30000
    Increment = 10
    Position = 0
    TabOrder = 6
    Wrap = False
  end
  object txtDelayTo: TEdit
    Left = 208
    Top = 136
    Width = 49
    Height = 21
    Hint = 'Dealy a random seconds between from and to value.'
    ParentShowHint = False
    ShowHint = True
    TabOrder = 7
    Text = '0'
  end
  object udDelayTo: TUpDown
    Left = 257
    Top = 136
    Width = 16
    Height = 21
    Associate = txtDelayTo
    Min = 0
    Max = 30000
    Increment = 10
    Position = 0
    TabOrder = 8
    Wrap = False
  end
  object BitBtn1: TBitBtn
    Left = 240
    Top = 224
    Width = 75
    Height = 25
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 9
    Glyph.Data = {
      F6000000424DF600000000000000760000002800000010000000100000000100
      04000000000080000000CE0E0000C40E00001000000000000000000000000000
      80000080000000808000800000008000800080800000C0C0C000808080000000
      FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00777788887777
      77777778888887777777778444448877777777422224488777777A2222224488
      7777A222222224487777A222AAA222488777A22488AA22448877A224877AA224
      4887AAA27777AA224488777777777AA224487777777777AA224877777777777A
      A224777777777777AA227777777777777AA27777777777777777}
  end
  object BitBtn2: TBitBtn
    Left = 320
    Top = 224
    Width = 75
    Height = 25
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 10
    Glyph.Data = {
      F6000000424DF600000000000000760000002800000010000000100000000100
      04000000000080000000CE0E0000C40E00001000000000000000000000000000
      8000008000000080800080000000800080008080000080808000C0C0C0000000
      FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00887788888888
      8888891178888897888889111788891178888911117891111788889111171111
      1788888911111111788888889111111788888888811111788888888889111178
      8888888891111178888888891117111788888891117891117888889117888911
      1788888918888891117888888888888919888888888888888888}
  end
  object txtRequestMessage: TEdit
    Left = 120
    Top = 160
    Width = 241
    Height = 21
    Hint = 
      'This message will be send while delay tick. Only work in active ' +
      'mode.'
    ParentShowHint = False
    ShowHint = True
    TabOrder = 11
  end
  object txtResponseMessage: TEdit
    Left = 120
    Top = 184
    Width = 241
    Height = 21
    Hint = 
      'This message will be send while device received a request messag' +
      'e. Only work in passivity mode.'
    TabOrder = 12
  end
  object txtTag: TEdit
    Left = 72
    Top = 60
    Width = 41
    Height = 21
    TabOrder = 13
  end
end
