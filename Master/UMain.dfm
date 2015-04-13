object FMain: TFMain
  Left = 273
  Top = 116
  Width = 913
  Height = 602
  Caption = 'Master'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  Menu = mnuMain
  OldCreateOrder = False
  OnClose = FormClose
  PixelsPerInch = 96
  TextHeight = 13
  object Splitter1: TSplitter
    Left = 0
    Top = 321
    Width = 897
    Height = 3
    Cursor = crVSplit
    Align = alTop
    Beveled = True
    Color = clGray
    MinSize = 1
    ParentColor = False
  end
  object GroupBox1: TGroupBox
    Left = 0
    Top = 0
    Width = 897
    Height = 321
    Align = alTop
    Caption = 'Devices'
    TabOrder = 0
    DesignSize = (
      897
      321)
    object txtResult: TMemo
      Left = 8
      Top = 16
      Width = 884
      Height = 297
      Anchors = [akLeft, akTop, akRight, akBottom]
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Consolas'
      Font.Style = []
      Lines.Strings = (
        'txtResult')
      ParentFont = False
      TabOrder = 1
      Visible = False
      OnDblClick = txtResultDblClick
    end
    object gridDevices: TStringGrid
      Left = 9
      Top = 17
      Width = 883
      Height = 296
      Anchors = [akLeft, akTop, akRight, akBottom]
      ColCount = 7
      RowCount = 2
      TabOrder = 0
      OnTopLeftChanged = gridDevicesTopLeftChanged
    end
  end
  object Master: TGroupBox
    Left = 0
    Top = 324
    Width = 897
    Height = 219
    Align = alTop
    Anchors = [akLeft, akTop, akRight, akBottom]
    Caption = 'Master'
    TabOrder = 1
    object lblRxBytes: TLabel
      Left = 26
      Top = 174
      Width = 48
      Height = 13
      Caption = 'Rx(Bytes):'
    end
    object RxBytes: TLabel
      Left = 26
      Top = 195
      Width = 47
      Height = 13
      Caption = 'Tx(Bytes):'
    end
    object lblRxRate1: TLabel
      Left = 181
      Top = 175
      Width = 42
      Height = 13
      Caption = '0Bytes/s'
    end
    object lblTxRate1: TLabel
      Left = 181
      Top = 199
      Width = 42
      Height = 13
      Caption = '0Bytes/s'
    end
    object lblConnCntCH1: TLabel
      Left = 117
      Top = 150
      Width = 13
      Height = 13
      Alignment = taCenter
      AutoSize = False
      Caption = '0'
      Color = clMedGray
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -11
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblTxRate2: TLabel
      Left = 341
      Top = 199
      Width = 42
      Height = 13
      Caption = '0Bytes/s'
    end
    object lblRxRate2: TLabel
      Left = 341
      Top = 175
      Width = 42
      Height = 13
      Caption = '0Bytes/s'
    end
    object lblTxRate3: TLabel
      Left = 493
      Top = 199
      Width = 42
      Height = 13
      Caption = '0Bytes/s'
    end
    object lblRxRate3: TLabel
      Left = 493
      Top = 175
      Width = 42
      Height = 13
      Caption = '0Bytes/s'
    end
    object Shape1: TShape
      Left = 24
      Top = 168
      Width = 505
      Height = -1
    end
    object lblCh1: TLabel
      Left = 90
      Top = 150
      Width = 21
      Height = 13
      Caption = 'CH1'
    end
    object lblCH2: TLabel
      Left = 266
      Top = 150
      Width = 21
      Height = 13
      Caption = 'CH2'
    end
    object lblCH3: TLabel
      Left = 410
      Top = 150
      Width = 21
      Height = 13
      Caption = 'CH3'
    end
    object lblConnCntCH2: TLabel
      Left = 293
      Top = 150
      Width = 13
      Height = 13
      Alignment = taCenter
      AutoSize = False
      Caption = ' 0'
      Color = clMedGray
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -11
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblConnCntCH3: TLabel
      Left = 437
      Top = 150
      Width = 13
      Height = 13
      Alignment = taCenter
      AutoSize = False
      Caption = ' 0'
      Color = clMedGray
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -11
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object ShapeCH11: TShape
      Left = 136
      Top = 149
      Width = 13
      Height = 13
      Brush.Color = clOlive
      Shape = stCircle
    end
    object ShapeCH12: TShape
      Left = 152
      Top = 149
      Width = 13
      Height = 13
      Brush.Color = clOlive
      Shape = stCircle
    end
    object ShapeCH13: TShape
      Left = 168
      Top = 149
      Width = 13
      Height = 13
      Brush.Color = clOlive
      Shape = stCircle
    end
    object ShapeCH21: TShape
      Left = 308
      Top = 150
      Width = 13
      Height = 13
      Brush.Color = clOlive
      Shape = stCircle
    end
    object ShapeCH22: TShape
      Left = 324
      Top = 150
      Width = 13
      Height = 13
      Brush.Color = clOlive
      Shape = stCircle
    end
    object ShapeCH23: TShape
      Left = 340
      Top = 150
      Width = 13
      Height = 13
      Brush.Color = clOlive
      Shape = stCircle
    end
    object ShapeCH31: TShape
      Left = 452
      Top = 150
      Width = 13
      Height = 13
      Brush.Color = clOlive
      Shape = stCircle
    end
    object ShapeCH32: TShape
      Left = 468
      Top = 150
      Width = 13
      Height = 13
      Brush.Color = clOlive
      Shape = stCircle
    end
    object ShapeCH33: TShape
      Left = 484
      Top = 150
      Width = 13
      Height = 13
      Brush.Color = clOlive
      Shape = stCircle
    end
    object rbMasterClientMode: TRadioButton
      Left = 8
      Top = 16
      Width = 81
      Height = 17
      Caption = 'Client Mode'
      Checked = True
      TabOrder = 0
      TabStop = True
      OnClick = rbMasterClientModeClick
    end
    object rbMasterServerMode: TRadioButton
      Left = 344
      Top = 16
      Width = 81
      Height = 17
      Caption = 'Server Mode'
      TabOrder = 1
      OnClick = rbMasterServerModeClick
    end
    object txtRxBytesCH1: TEdit
      Left = 88
      Top = 172
      Width = 89
      Height = 21
      TabOrder = 2
      Text = '0'
    end
    object txtTxBytesCH1: TEdit
      Left = 88
      Top = 194
      Width = 89
      Height = 21
      TabOrder = 3
      Text = '0'
    end
    object btnClear: TButton
      Left = 568
      Top = 189
      Width = 75
      Height = 25
      Caption = '&Clear'
      TabOrder = 4
      OnClick = btnClearClick
    end
    object txtRxBytesCH2: TEdit
      Left = 248
      Top = 172
      Width = 89
      Height = 21
      TabOrder = 5
      Text = '0'
    end
    object txtTxBytesCH2: TEdit
      Left = 248
      Top = 194
      Width = 89
      Height = 21
      TabOrder = 6
      Text = '0'
    end
    object txtRxBytesCH3: TEdit
      Left = 400
      Top = 172
      Width = 89
      Height = 21
      TabOrder = 7
      Text = '0'
    end
    object txtTxBytesCH3: TEdit
      Left = 400
      Top = 194
      Width = 89
      Height = 21
      TabOrder = 8
      Text = '0'
    end
    object gboxServerMode: TGroupBox
      Left = 344
      Top = 33
      Width = 529
      Height = 104
      Caption = 'Server Mode'
      TabOrder = 9
      object lblChanneControl: TLabel
        Left = 16
        Top = 44
        Width = 78
        Height = 13
        Caption = 'Channel Control:'
      end
      object lblMasterPort: TLabel
        Left = 8
        Top = 18
        Width = 81
        Height = 13
        Alignment = taRightJustify
        AutoSize = False
        Caption = 'Master Port:'
        Enabled = False
      end
      object lblListenStatus: TLabel
        Left = 288
        Top = 14
        Width = 82
        Height = 13
        Caption = 'Wait to listen.......'
        Enabled = False
      end
      object Label1: TLabel
        Left = 95
        Top = 44
        Width = 21
        Height = 13
        AutoSize = False
        Caption = 'CH1'
        Color = clBlue
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWhite
        Font.Height = -11
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentColor = False
        ParentFont = False
      end
      object Label2: TLabel
        Left = 239
        Top = 44
        Width = 21
        Height = 13
        AutoSize = False
        Caption = 'CH2'
        Color = clBlue
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWhite
        Font.Height = -11
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentColor = False
        ParentFont = False
      end
      object Label3: TLabel
        Left = 375
        Top = 44
        Width = 21
        Height = 13
        AutoSize = False
        Caption = 'CH3'
        Color = clBlue
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWhite
        Font.Height = -11
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentColor = False
        ParentFont = False
      end
      object cboErrorCH1: TComboBox
        Left = 126
        Top = 40
        Width = 80
        Height = 21
        ItemHeight = 13
        TabOrder = 0
        Text = 'No Error(0)'
        OnChange = cboErrorCH1Change
        Items.Strings = (
          'No Error(0)'
          'Uniform(1/X)'
          'Poisson(%)')
      end
      object txtMasterPort: TEdit
        Left = 96
        Top = 12
        Width = 97
        Height = 21
        Enabled = False
        TabOrder = 1
      end
      object cboErrorCH2: TComboBox
        Left = 270
        Top = 40
        Width = 80
        Height = 21
        ItemHeight = 13
        TabOrder = 2
        Text = 'No Error'
        OnChange = cboErrorCH2Change
        Items.Strings = (
          'No Error(0)'
          'Uniform(1/X)'
          'Poisson(%)')
      end
      object cboErrorCH3: TComboBox
        Left = 406
        Top = 40
        Width = 80
        Height = 21
        ItemHeight = 13
        TabOrder = 3
        Text = 'No Error'
        OnChange = cboErrorCH3Change
        Items.Strings = (
          'No Error(0)'
          'Uniform(1/X)'
          'Poisson(%)')
      end
      object btnOpen: TBitBtn
        Left = 200
        Top = 8
        Width = 83
        Height = 25
        Caption = '&Listen'
        TabOrder = 4
        OnClick = btnOpenClick
        Glyph.Data = {
          36030000424D3603000000000000360000002800000010000000100000000100
          18000000000000030000120B0000120B00000000000000000000FFFFFFFFFFFF
          FEFFFFFFFFFFFFFFFFFAF9FAF3E0D1EED1B4EFD1B4F3E1D2FAF9FBFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBF9FAEBC49BEBA03FF1920FF5
          8E00F48B00F08A00E8952BE9BD91FAFAFBFFFFFFFFFFFFFFFFFFFEFEFEFFFFFF
          F8EFE9E7AC66F7A323FFB641FFC56FFFCB82FFC97EFFBE60FFA51DF58A00E399
          45F6EDE8FFFFFFFFFFFFFFFFFFFCFAFAEBB573FCB540FFD28BFEE3C6FFE8D3FF
          E5CCFEE2C7FEE1C7FDDDBBFFC169FB9300E39C49FBF9FBFFFFFFFFFFFFF1D3B3
          F9BA55FEDCA2FEEEDDFDE8D2F3CDABF9DDC2FFECD7FEE5C9FEE4CAFDE6D2FFC9
          78F69303EBC49BFFFFFFFCFAF9F3C380FFD88EFDF4EAFEF9F1F1CBA5EA8B12E7
          932FEDC095FDEDDCFFEFDDFDE9D3FEEAD4FFB745ECA445FBF9FAF8EDE1F8C97B
          FFEDC9FEFCFCFFFEFDF1CCA4FCAB2BFEAB23F09519E7A052F3D4BAFEF2E4FEF1
          E8FFD697F4A22CF6E6D8F8E8D3FBD48AFFF7E3FFFEFFFFFFFFF6D8B7FBB94FFF
          BD4DFFB83FFDAB29EC9525F3D3B6FFFBF8FFE5BEF7AB37F3DCC2F9EAD7FCDA95
          FFF9E7FEFFFFFFFFFFF9E0C4FCC86DFFCA6BFEC660FEBC4DF2A742F4DBC3FFFF
          FFFFEDCDF8B44AF5DDC6FBF1E7FADDA1FFF7DEFEFFFFFFFFFFFBE7CFFCD587FF
          D682FACA75F5CA93F8EADEFFFFFFFEFFFFFFEAC2F7BB5CF8EADDFDFBFAFAE2B4
          FFF1C7FFFEFFFEFFFFFDF2E1FEDE9FFDDEA8FBEBD7FEFFFFFFFFFFFEFFFFFFFE
          FEFEDB97F5C682FDFBFAFFFFFFFCEBD4FDEAB5FEF8E4FEFFFFFEFEFFFEF7EFFE
          FCFBFFFFFFFFFEFEFFFEFFFEFFFFFEF0D1FBCF7CF6DEBFFFFFFFFEFFFFFEFDFB
          FBE8C0FEF1C0FEFAE5FEFEFEFEFFFFFEFFFFFEFEFFFEFFFFFEFDFCFEF3D6FEDE
          96F6D3A1FDFBFAFFFFFFFFFFFFFFFFFFFDF9F4FDEBC5FDF1BFFEF7D3FEFAE4FE
          FAECFFFAEBFEF7DFFEEFC2FDE3A1F8DAACFCF5F1FFFFFFFEFFFFFEFFFFFFFFFF
          FFFFFFFEFDFBFEF1DAFEEFC7FEF0C1FDEEBDFDEDB9FDE9B5FDE7B7FBEBD0FEFC
          FBFFFFFFFEFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFDFDFBFEF8EEFE
          F6E5FDF5E5FDF7ECFFFCFBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF}
      end
      object cboErrorValCH1: TComboBox
        Left = 96
        Top = 64
        Width = 110
        Height = 21
        Hint = 
          'To control error rate while channel send out message. In Uniform' +
          ' mode, the rate control by  how many message will contains an er' +
          'ror. In possion mode, error rate control by a precent rate, the ' +
          'rate depends a possion curve.'
        ItemHeight = 13
        ParentShowHint = False
        ShowHint = True
        TabOrder = 5
        Text = 'cboErrorValCH1'
        OnChange = cboErrorValCH1Change
      end
      object cboErrorValCH2: TComboBox
        Left = 240
        Top = 64
        Width = 110
        Height = 21
        Hint = 
          'To control error rate while channel send out message. In Uniform' +
          ' mode, the rate control by  how many message will contains an er' +
          'ror. In possion mode, error rate control by a precent rate, the ' +
          'rate depends a possion curve.'
        ItemHeight = 13
        ParentShowHint = False
        ShowHint = True
        TabOrder = 6
        Text = 'cboErrorValCH2'
        OnChange = cboErrorValCH2Change
      end
      object cboErrorValCH3: TComboBox
        Left = 376
        Top = 64
        Width = 110
        Height = 21
        Hint = 
          'To control error rate while channel send out message. In Uniform' +
          ' mode, the rate control by  how many message will contains an er' +
          'ror. In possion mode, error rate control by a precent rate, the ' +
          'rate depends a possion curve.'
        ItemHeight = 13
        ParentShowHint = False
        ShowHint = True
        TabOrder = 7
        Text = 'cboErrorValCH1'
        OnChange = cboErrorValCH3Change
      end
    end
    object GroupBox2: TGroupBox
      Left = 4
      Top = 31
      Width = 325
      Height = 106
      Caption = 'Client Mode'
      TabOrder = 10
      object lblPeerIP: TLabel
        Left = 8
        Top = 24
        Width = 108
        Height = 13
        Alignment = taRightJustify
        AutoSize = False
        Caption = 'Remote Master Host:'
      end
      object lblPeerPort: TLabel
        Left = 8
        Top = 48
        Width = 104
        Height = 13
        Alignment = taRightJustify
        AutoSize = False
        Caption = 'Master Service Port:'
      end
      object lblConnectStatus: TLabel
        Left = 8
        Top = 72
        Width = 265
        Height = 13
        AutoSize = False
        Caption = 'Wait to connect.......'
      end
      object txtPeerIP: TEdit
        Left = 120
        Top = 20
        Width = 89
        Height = 21
        TabOrder = 0
      end
      object txtPeerPort: TEdit
        Left = 120
        Top = 44
        Width = 73
        Height = 21
        TabOrder = 1
      end
      object btnConnect: TBitBtn
        Left = 214
        Top = 16
        Width = 89
        Height = 25
        Caption = '&Start'
        TabOrder = 2
        OnClick = btnConnectClick
        Glyph.Data = {
          36030000424D3603000000000000360000002800000010000000100000000100
          18000000000000030000120B0000120B00000000000000000000FFFFFFFFFFFF
          FEFFFFFFFFFFFFFFFFFAF9FAF3E0D1EED1B4EFD1B4F3E1D2FAF9FBFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBF9FAEBC49BEBA03FF1920FF5
          8E00F48B00F08A00E8952BE9BD91FAFAFBFFFFFFFFFFFFFFFFFFFEFEFEFFFFFF
          F8EFE9E7AC66F7A323FFB641FFC56FFFCB82FFC97EFFBE60FFA51DF58A00E399
          45F6EDE8FFFFFFFFFFFFFFFFFFFCFAFAEBB573FCB540FFD28BFEE3C6FFE8D3FF
          E5CCFEE2C7FEE1C7FDDDBBFFC169FB9300E39C49FBF9FBFFFFFFFFFFFFF1D3B3
          F9BA55FEDCA2FEEEDDFDE8D2F3CDABF9DDC2FFECD7FEE5C9FEE4CAFDE6D2FFC9
          78F69303EBC49BFFFFFFFCFAF9F3C380FFD88EFDF4EAFEF9F1F1CBA5EA8B12E7
          932FEDC095FDEDDCFFEFDDFDE9D3FEEAD4FFB745ECA445FBF9FAF8EDE1F8C97B
          FFEDC9FEFCFCFFFEFDF1CCA4FCAB2BFEAB23F09519E7A052F3D4BAFEF2E4FEF1
          E8FFD697F4A22CF6E6D8F8E8D3FBD48AFFF7E3FFFEFFFFFFFFF6D8B7FBB94FFF
          BD4DFFB83FFDAB29EC9525F3D3B6FFFBF8FFE5BEF7AB37F3DCC2F9EAD7FCDA95
          FFF9E7FEFFFFFFFFFFF9E0C4FCC86DFFCA6BFEC660FEBC4DF2A742F4DBC3FFFF
          FFFFEDCDF8B44AF5DDC6FBF1E7FADDA1FFF7DEFEFFFFFFFFFFFBE7CFFCD587FF
          D682FACA75F5CA93F8EADEFFFFFFFEFFFFFFEAC2F7BB5CF8EADDFDFBFAFAE2B4
          FFF1C7FFFEFFFEFFFFFDF2E1FEDE9FFDDEA8FBEBD7FEFFFFFFFFFFFEFFFFFFFE
          FEFEDB97F5C682FDFBFAFFFFFFFCEBD4FDEAB5FEF8E4FEFFFFFEFEFFFEF7EFFE
          FCFBFFFFFFFFFEFEFFFEFFFEFFFFFEF0D1FBCF7CF6DEBFFFFFFFFEFFFFFEFDFB
          FBE8C0FEF1C0FEFAE5FEFEFEFEFFFFFEFFFFFEFEFFFEFFFFFEFDFCFEF3D6FEDE
          96F6D3A1FDFBFAFFFFFFFFFFFFFFFFFFFDF9F4FDEBC5FDF1BFFEF7D3FEFAE4FE
          FAECFFFAEBFEF7DFFEEFC2FDE3A1F8DAACFCF5F1FFFFFFFEFFFFFEFFFFFFFFFF
          FFFFFFFEFDFBFEF1DAFEEFC7FEF0C1FDEEBDFDEDB9FDE9B5FDE7B7FBEBD0FEFC
          FBFFFFFFFEFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFDFDFBFEF8EEFE
          F6E5FDF5E5FDF7ECFFFCFBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF}
      end
      object chkAutoReconn: TCheckBox
        Left = 200
        Top = 48
        Width = 97
        Height = 17
        Caption = '&Auto reconnect.'
        TabOrder = 3
        OnClick = chkAutoReconnClick
      end
    end
    object btnResult: TButton
      Left = 648
      Top = 188
      Width = 75
      Height = 25
      Caption = '&Result'
      TabOrder = 11
      OnClick = btnResultClick
    end
  end
  object mnuMain: TMainMenu
    Left = 208
    Top = 96
    object File1: TMenuItem
      Caption = '&File'
      object Saveconfigure1: TMenuItem
        Bitmap.Data = {
          36030000424D3603000000000000360000002800000010000000100000000100
          18000000000000030000120B0000120B00000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFBF6F1EDC39EECAA74ECA66FE8BA95DFB599DF
          B79DE0B394E0B291DEB292E4A77BE8A36BEECEB1FFFFFFFFFFFFFFFFFFEECBA8
          F7A15AFE842EFE781AF2C9B4DEAFA6F1A273E3BDB7DCB4B2D9B8BAEC9363FD85
          2CEB944BF2DFCEFFFFFFFFFFFFEFC8A0FEB578FF9748FF923BF6DCCDEFBAA3FF
          8B2EF1CFC1E6CDD0E7D1D5F3A97FFF9545F5A765EED5BDFFFFFFFFFFFFF0CEAA
          FDBA81FFA357FFA04DFAE4D6F4D4CAFCB075F4DBD5F0DDE0F0E0E6F9BC92FF9C
          4FF6AF6FF0DAC3FFFFFFFFFFFFF2D2B2FDBF8AFFAF6CFFAA5BFEDDBEFBF3F4FA
          F0EEFBEFECFBEEEBFBF2F0FDC393FFA45DF6B47BF1DBC6FFFFFFFFFFFFF3D6B7
          FDC595FEB77CFEB679FFB87DFFC18DFFC18CFFBF8AFFBD89FFBB85FFAF6FFFAE
          6DF6BA85F2DDC9FFFFFFFFFFFFF4DBC0FEC89BFEC798FDE0C4FEDEC0FEDABAFE
          D7B7FED6B3FED5AFFED4ACFED0A6FFB87EF7C08FF3E0CCFFFFFFFFFFFFF6E0C7
          FECEA4FEDCBEFDE7CBFEDCB5FED9B2FDD8AEFED4A9FED2A5FECE9DFDDBB6FFC7
          97F7C496F4E2D0FFFFFFFFFFFFF8E4CDFED5ACFEE1C6FEE7CBFEDCB4FEDAB0FE
          D8ABFED5A7FED1A2FECC9BFEDCB8FFCA9FF9CAA1F5E4D3FFFFFFFFFFFFF9E8D4
          FEDAB6FEE6CFFEF3E2FDEACFFEE9CBFDE7C6FEE3C1FDE0BCFEDCB5FEE5C9FFCF
          A7F9D1ACF7E7D6FFFFFFFFFFFFFAECDAFEDDBDFEEAD6FEF4E3FDEBCFFDE8CBFE
          E6C6FEE3C0FDE0BBFFDCB4FEE9CEFED6AFFAD7B5F7E9D9FFFFFFFFFFFFFCEFDD
          FDDFC0FEEBD7FEFAF1FFF6E4FEF5E1FFF1DBFEEFD7FEEBD1FEE8C9FDF0DCFFD9
          B5FBD9B9F9EBDCFFFFFFFFFFFFFDEFDCFEEDDBFEF2E7FEFBF2FEF8E7FEF9E8FE
          F7E7FEF6E4FEF3DFFEF1D8FEF6E9FEE9D6FAE5CFF9ECDBFEFFFFFFFFFFFEF5E9
          FDF0DDFDF3E2FEF2E1FEF2E2FDF0E0FDF0DFFCEFDFFCEFDFFBEDDDFBEAD7FAEB
          D9F9E3CBFDF7F0FFFFFFFFFFFFFEFFFFFFFCF8FEFBF6FFFBF6FFFAF6FEFAF5FE
          FAF5FEF9F4FEF9F4FEF9F4FEF8F3FDF8F2FEFCFAFFFFFFFFFFFF}
        Caption = '&Save configure'
        OnClick = Saveconfigure1Click
      end
      object TCPclientconfigure1: TMenuItem
        Caption = 'TCP client configure...'
      end
      object Export1: TMenuItem
        Caption = '&Export...'
      end
      object OpenLogFile1: TMenuItem
        Caption = '&Open Log File...'
      end
      object N1: TMenuItem
        Caption = '-'
      end
      object Exit1: TMenuItem
        Caption = '&Exit'
        OnClick = Exit1Click
      end
    end
    object Help1: TMenuItem
      Caption = '&Help'
      object About1: TMenuItem
        Caption = '&About'
      end
    end
  end
  object tmrOutputPri: TTimer
    Enabled = False
    OnTimer = tmrOutputPriTimer
    Left = 816
    Top = 336
  end
  object tmrWriteResult: TTimer
    Interval = 60000
    OnTimer = tmrWriteResultTimer
    Left = 848
    Top = 336
  end
end
