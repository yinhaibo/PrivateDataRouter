object Form1: TForm1
  Left = 193
  Top = 126
  Width = 870
  Height = 526
  Caption = 'Distirbution Algorithm'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 336
    Top = 48
    Width = 71
    Height = 13
    Caption = 'Random Seed:'
  end
  object Label2: TLabel
    Left = 8
    Top = 8
    Width = 98
    Height = 13
    Caption = 'Uniform deistribution:'
  end
  object Label3: TLabel
    Left = 528
    Top = 8
    Width = 99
    Height = 13
    Caption = 'Possion deistribution:'
  end
  object Label4: TLabel
    Left = 368
    Top = 120
    Width = 73
    Height = 13
    Caption = 'Samples:10000'
  end
  object Memo1: TMemo
    Left = 8
    Top = 24
    Width = 305
    Height = 185
    Lines.Strings = (
      'Memo1')
    ScrollBars = ssVertical
    TabOrder = 0
  end
  object Memo2: TMemo
    Left = 528
    Top = 24
    Width = 321
    Height = 185
    Lines.Strings = (
      'Memo2')
    ScrollBars = ssVertical
    TabOrder = 1
  end
  object Button1: TButton
    Left = 336
    Top = 80
    Width = 145
    Height = 25
    Caption = 'Generate'
    TabOrder = 2
    OnClick = Button1Click
  end
  object Edit1: TEdit
    Left = 416
    Top = 40
    Width = 57
    Height = 21
    TabOrder = 3
    Text = '100'
  end
  object Chart1: TChart
    Left = 8
    Top = 224
    Width = 825
    Height = 249
    BackWall.Brush.Color = clWhite
    BackWall.Brush.Style = bsClear
    Title.Text.Strings = (
      'TChart')
    TabOrder = 4
    object Series1: TFastLineSeries
      Marks.ArrowLength = 8
      Marks.Visible = False
      SeriesColor = clRed
      Title = 'Uniform'
      LinePen.Color = clRed
      XValues.DateTime = False
      XValues.Name = 'X'
      XValues.Multiplier = 1
      XValues.Order = loAscending
      YValues.DateTime = False
      YValues.Name = 'Y'
      YValues.Multiplier = 1
      YValues.Order = loNone
    end
    object Series2: TFastLineSeries
      Marks.ArrowLength = 8
      Marks.Visible = False
      SeriesColor = clGreen
      Title = 'Possion'
      LinePen.Color = clGreen
      XValues.DateTime = False
      XValues.Name = 'X'
      XValues.Multiplier = 1
      XValues.Order = loAscending
      YValues.DateTime = False
      YValues.Name = 'Y'
      YValues.Multiplier = 1
      YValues.Order = loNone
    end
  end
end
