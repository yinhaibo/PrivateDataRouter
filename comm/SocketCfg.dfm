object OKRightDlg: TOKRightDlg
  Left = 425
  Top = 243
  BorderStyle = bsDialog
  Caption = 'Network Config'
  ClientHeight = 110
  ClientWidth = 310
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
    Caption = 'IP Addr:'
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
    Caption = 'Network Type:'
  end
  object OKBtn: TButton
    Left = 224
    Top = 8
    Width = 75
    Height = 25
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 5
    OnClick = OKBtnClick
  end
  object CancelBtn: TButton
    Left = 224
    Top = 42
    Width = 75
    Height = 25
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 6
    OnClick = CancelBtnClick
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
    Left = 16
    Top = 88
    Width = 113
    Height = 17
    Caption = 'Client Mode'
    Checked = True
    TabOrder = 3
    TabStop = True
    OnClick = rClientClick
  end
  object rServer: TRadioButton
    Left = 128
    Top = 88
    Width = 89
    Height = 17
    Caption = 'Server Mode'
    TabOrder = 4
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
    Items.Strings = (
      'TCP'
      'UDP')
  end
end
