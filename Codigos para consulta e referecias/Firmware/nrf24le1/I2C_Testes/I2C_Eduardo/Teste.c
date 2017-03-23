<?xml version="1.0" encoding="UTF-8" standalone="no" ?>
<ProjectOpt xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="project_opt.xsd">

  <SchemaVersion>1.0</SchemaVersion>

  <Header>### uVision Project, (C) Keil Software</Header>

  <Extensions>
    <cExt>*.c</cExt>
    <aExt>*.s*; *.src; *.a*</aExt>
    <oExt>*.obj</oExt>
    <lExt>*.lib</lExt>
    <tExt>*.txt; *.h; *.inc</tExt>
    <pExt>*.plm</pExt>
    <CppX>*.cpp</CppX>
  </Extensions>

  <DaveTm>
    <dwLowDateTime>0</dwLowDateTime>
    <dwHighDateTime>0</dwHighDateTime>
  </DaveTm>

  <Target>
    <TargetName>APP</TargetName>
    <ToolsetNumber>0x0</ToolsetNumber>
    <ToolsetName>MCS-51</ToolsetName>
    <TargetOption>
      <CLK51>24000000</CLK51>
      <OPTTT>
        <gFlags>1</gFlags>
        <BeepAtEnd>1</BeepAtEnd>
        <RunSim>1</RunSim>
        <RunTarget>0</RunTarget>
      </OPTTT>
      <OPTHX>
        <HexSelection>0</HexSelection>
        <FlashByte>65535</FlashByte>
        <HexRangeLowAddress>0</HexRangeLowAddress>
        <HexRangeHighAddress>0</HexRangeHighAddress>
        <HexOffset>0</HexOffset>
      </OPTHX>
      <OPTLEX>
        <PageWidth>120</PageWidth>
        <PageLength>65</PageLength>
        <TabStop>8</TabStop>
        <ListingPath>.\</ListingPath>
      </OPTLEX>
      <ListingPage>
        <CreateCListing>1</CreateCListing>
        <CreateAListing>1</CreateAListing>
        <CreateLListing>1</CreateLListing>
        <CreateIListing>0</CreateIListing>
        <AsmCond>1</AsmCond>
        <AsmSymb>1</AsmSymb>
        <AsmXref>0</AsmXref>
        <CCond>1</CCond>
        <CCode>0</CCode>
        <CListInc>0</CListInc>
        <CSymb>0</CSymb>
        <LinkerCodeListing>0</LinkerCodeListing>
      </ListingPage>
      <OPTXL>
        <LMap>1</LMap>
        <LComments>1</LComments>
        <LGenerateSymbols>1</LGenerateSymbols>
        <LLibSym>1</LLibSym>
        <LLines>1</LLines>
        <LLocSym>1</LLocSym>
        <LPubSym>1</LPubSym>
        <LXref>0</LXref>
        <LExpSel>0</LExpSel>
      </OPTXL>
      <OPTFL>
        <tvExp>1</tvExp>
        <tvExpOptDlg>0</tv