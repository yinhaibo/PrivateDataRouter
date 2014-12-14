object SocketSetting: TSocketSetting
  Left = 724
  Top = 304
  BorderStyle = bsDialog
  Caption = 'Net Configure'
  ClientHeight = 121
  ClientWidth = 275
  Color = clBtnFace
  ParentFont = True
  OldCreateOrder = True
  Position = poScreenCenter
  PixelsPerInch = 96
  TextHeight = 13
  object lblIP: TLabel
    Left = 8
    Top = 8
    Width = 49
    Height = 13
    Alignment = taRightJustify
    AutoSize = False
    Caption = 'IP Address'
  end
  object Label1: TLabel
    Left = 12
    Top = 36
    Width = 45
    Height = 13
    Alignment = taRightJustify
    AutoSize = False
    Caption = 'Port:'
  end
  object Label2: TLabel
    Left = 12
    Top = 60
    Width = 45
    Height = 13
    Alignment = taRightJustify
    AutoSize = False
    Caption = 'Protocol:'
    Visible = False
  end
  object edtPort: TEdit
    Left = 60
    Top = 32
    Width = 53
    Height = 21
    TabOrder = 1
    Text = '8090'
  end
  object edtIP: TEdit
    Left = 60
    Top = 4
    Width = 133
    Height = 21
    TabOrder = 0
    Text = '127.0.0.1'
  end
  object rClient: TRadioButton
    Left = 176
    Top = 32
    Width = 113
    Height = 17
    Caption = 'As a Client'
    Checked = True
    TabOrder = 3
    TabStop = True
    Visible = False
    OnClick = rClientClick
  end
  object rServer: TRadioButton
    Left = 176
    Top = 56
    Width = 89
    Height = 17
    Caption = 'As a server'
    TabOrder = 4
    Visible = False
    OnClick = rServerClick
  end
  object cboSockType: TComboBox
    Left = 59
    Top = 56
    Width = 94
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 2
    Text = 'TCP'
    Visible = False
    Items.Strings = (
      'TCP'
      'UDP')
  end
  object OKBtn: TBitBtn
    Left = 64
    Top = 92
    Width = 75
    Height = 25
    Caption = '&Ok'
    ModalResult = 1
    TabOrder = 5
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
  object CancelBtn: TBitBtn
    Left = 144
    Top = 92
    Width = 75
    Height = 25
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 6
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
end
