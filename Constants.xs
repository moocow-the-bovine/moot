#/*-*- Mode: C -*- */

MODULE = Moot		PACKAGE = Moot   PREFIX = moot_

##=====================================================================
## Constants
##=====================================================================

##-- disable perl prototypes
PROTOTYPES: DISABLE

##--------------------------------------------------------------
## mootConfig.h
const char *
library_version()
 CODE:
   RETVAL=moot_version_string;
 OUTPUT:
   RETVAL

##--------------------------------------------------------------
## mootToken.h: mootTokenTypeE

mootTokenType
TokTypeUnknown()
CODE:
 RETVAL=moot::TokTypeUnknown;
OUTPUT:
 RETVAL

mootTokenType
TokTypeVanilla()
CODE:
 RETVAL=moot::TokTypeVanilla;
OUTPUT:
 RETVAL

mootTokenType
TokTypeLibXML()
CODE:
 RETVAL=moot::TokTypeLibXML;
OUTPUT:
 RETVAL

mootTokenType
TokTypeXMLRaw()
CODE:
 RETVAL=moot::TokTypeXMLRaw;
OUTPUT:
 RETVAL

mootTokenType
TokTypeComment()
CODE:
 RETVAL=moot::TokTypeComment;
OUTPUT:
 RETVAL

mootTokenType
TokTypeEOS()
CODE:
 RETVAL=moot::TokTypeEOS;
OUTPUT:
 RETVAL

mootTokenType
TokTypeEOF()
CODE:
 RETVAL=moot::TokTypeEOF;
OUTPUT:
 RETVAL

mootTokenType
TokTypeWB()
CODE:
 RETVAL=moot::TokTypeWB;
OUTPUT:
 RETVAL

mootTokenType
TokTypeSB()
CODE:
 RETVAL=moot::TokTypeSB;
OUTPUT:
 RETVAL

mootTokenType
TokTypeUser()
CODE:
 RETVAL=moot::TokTypeUser;
OUTPUT:
 RETVAL


##--------------------------------------------------------------
## mootHMM.h : mootHMM::verbosityLevel

int
vlSilent()
CODE:
 RETVAL = moot::vlSilent;
OUTPUT:
 RETVAL

int
vlErrors()
CODE:
 RETVAL = moot::vlErrors;
OUTPUT:
 RETVAL

int
vlWarnings()
CODE:
 RETVAL = moot::vlWarnings;
OUTPUT:
 RETVAL

int
vlProgress()
CODE:
 RETVAL = moot::vlProgress;
OUTPUT:
 RETVAL

int
vlEverything()
CODE:
 RETVAL = moot::vlEverything;
OUTPUT:
 RETVAL

##--------------------------------------------------------------
## wasteTypes.h: wasteScannerTypeE

wasteScannerType
wScanTypeEOF()
CODE:
 RETVAL = moot::wScanTypeEOF;
OUTPUT:
 RETVAL

wasteScannerType
wScanTypeWB()
CODE:
 RETVAL = moot::wScanTypeWB;
OUTPUT:
 RETVAL

wasteScannerType
wScanTypeSB()
CODE:
 RETVAL = moot::wScanTypeSB;
OUTPUT:
 RETVAL

wasteScannerType
wScanTypeLatin()
CODE:
 RETVAL = moot::wScanTypeLatin;
OUTPUT:
 RETVAL

wasteScannerType
wScanTypeGreek()
CODE:
 RETVAL = moot::wScanTypeGreek;
OUTPUT:
 RETVAL

wasteScannerType
wScanTypeAlpha()
CODE:
 RETVAL = moot::wScanTypeAlpha;
OUTPUT:
 RETVAL

wasteScannerType
wScanTypeNewline()
CODE:
 RETVAL = moot::wScanTypeNewline;
OUTPUT:
 RETVAL

wasteScannerType
wScanTypeSpace()
CODE:
 RETVAL = moot::wScanTypeSpace;
OUTPUT:
 RETVAL

wasteScannerType
wScanTypeNumber()
CODE:
 RETVAL = moot::wScanTypeNumber;
OUTPUT:
 RETVAL

wasteScannerType
wScanTypeRoman()
CODE:
 RETVAL = moot::wScanTypeRoman;
OUTPUT:
 RETVAL

wasteScannerType
wScanTypeHyphen()
CODE:
 RETVAL = moot::wScanTypeHyphen;
OUTPUT:
 RETVAL

wasteScannerType
wScanTypePunct()
CODE:
 RETVAL = moot::wScanTypePunct;
OUTPUT:
 RETVAL

wasteScannerType
wScanTypeLink()
CODE:
 RETVAL = moot::wScanTypeLink;
OUTPUT:
 RETVAL

wasteScannerType
wScanTypeXML()
CODE:
 RETVAL = moot::wScanTypeXML;
OUTPUT:
 RETVAL

wasteScannerType
wScanTypeComment()
CODE:
 RETVAL = moot::wScanTypeComment;
OUTPUT:
 RETVAL

wasteScannerType
wScanTypeOther()
CODE:
 RETVAL = moot::wScanTypeOther;
OUTPUT:
 RETVAL

##--------------------------------------------------------------
## wasteTypes.h: wasteLexerTypeE

wasteLexerType
wLexerTypeDot()
CODE:
 RETVAL = moot::wLexerTypeDot;
OUTPUT:
 RETVAL

wasteLexerType
wLexerTypeComma()
CODE:
 RETVAL = moot::wLexerTypeComma;
OUTPUT:
 RETVAL

wasteLexerType
wLexerTypeHyph()
CODE:
 RETVAL = moot::wLexerTypeHyph;
OUTPUT:
 RETVAL

wasteLexerType
wLexerTypeApostrophe()
CODE:
 RETVAL = moot::wLexerTypeApostrophe;
OUTPUT:
 RETVAL

wasteLexerType
wLexerTypeQuote()
CODE:
 RETVAL = moot::wLexerTypeQuote;
OUTPUT:
 RETVAL

wasteLexerType
wLexerTypeMonetary()
CODE:
 RETVAL = moot::wLexerTypeMonetary;
OUTPUT:
 RETVAL

wasteLexerType
wLexerTypePercent()
CODE:
 RETVAL = moot::wLexerTypePercent;
OUTPUT:
 RETVAL

wasteLexerType
wLexerTypePlus()
CODE:
 RETVAL = moot::wLexerTypePlus;
OUTPUT:
 RETVAL

wasteLexerType
wLexerTypeLBR()
CODE:
 RETVAL = moot::wLexerTypeLBR;
OUTPUT:
 RETVAL

wasteLexerType
wLexerTypeRBR()
CODE:
 RETVAL = moot::wLexerTypeRBR;
OUTPUT:
 RETVAL

wasteLexerType
wLexerTypeSlash()
CODE:
 RETVAL = moot::wLexerTypeSlash;
OUTPUT:
 RETVAL

wasteLexerType
wLexerTypeColon()
CODE:
 RETVAL = moot::wLexerTypeColon;
OUTPUT:
 RETVAL

wasteLexerType
wLexerTypeSemicolon()
CODE:
 RETVAL = moot::wLexerTypeSemicolon;
OUTPUT:
 RETVAL

wasteLexerType
wLexerTypeEOS()
CODE:
 RETVAL = moot::wLexerTypeEOS;
OUTPUT:
 RETVAL

wasteLexerType
wLexerTypePunct()
CODE:
 RETVAL = moot::wLexerTypePunct;
OUTPUT:
 RETVAL

wasteLexerType
wLexerTypeSpace()
CODE:
 RETVAL = moot::wLexerTypeSpace;
OUTPUT:
 RETVAL

wasteLexerType
wLexerTypeNewline()
 CODE:
 RETVAL = moot::wLexerTypeNewline;
OUTPUT:
 RETVAL

wasteLexerType
wLexerTypeNumber()
CODE:
 RETVAL = moot::wLexerTypeNumber;
OUTPUT:
 RETVAL

wasteLexerType
wLexerTypeRoman()
CODE:
 RETVAL = moot::wLexerTypeRoman;
OUTPUT:
 RETVAL

wasteLexerType
wLexerTypeAlphaLower()
CODE:
 RETVAL = moot::wLexerTypeAlphaLower;
OUTPUT:
 RETVAL

wasteLexerType
wLexerTypeAlphaUpper()
CODE:
 RETVAL = moot::wLexerTypeAlphaUpper;
OUTPUT:
 RETVAL

wasteLexerType
wLexerTypeAlphaCaps()
CODE:
 RETVAL = moot::wLexerTypeAlphaCaps;
OUTPUT:
 RETVAL

wasteLexerType
wLexerTypeAlphaTrunc()
CODE:
 RETVAL = moot::wLexerTypeAlphaTrunc;
OUTPUT:
 RETVAL

wasteLexerType
wLexerTypeOther()
CODE:
 RETVAL = moot::wLexerTypeOther;
OUTPUT:
 RETVAL
