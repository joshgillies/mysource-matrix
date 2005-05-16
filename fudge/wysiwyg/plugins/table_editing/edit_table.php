<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: edit_table.php,v 1.12 2005/05/16 06:36:36 lwright Exp $
*
*/

/**
* Table Edit Popup for the WYSIWYG
*
* @author	Dmitry Baranovskiy	<dbaranovskiy@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix
*/

require_once dirname(__FILE__).'/../../wysiwyg_plugin.inc';
$wysiwyg = null;
$plugin = new wysiwyg_plugin($wysiwyg);
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
	<title>Tables</title>
	<style type="text/css">
		html, body {
			height: 620px;
			width: 850px;
			overflow: hidden;
		}

		body {
			overflow: hidden;
		}

		.colors td {
			border: solid 1px #000
		}

		label,input,select,textarea {
			display: block;
			width: 150px;
			float: left;
			margin-bottom: 10px;
		}

		label {
			text-align: right;
			font: 10pt "Lucida Grande", Tahoma, Arial, Helvetica;
			width: 75px;
			padding-right: 20px;
		}

		legend {
			font: bold 10pt "Lucida Grande", Tahoma, Arial, Helvetica;
		}

		br {
			clear: left;
		}

		#panels {
			position: absolute;
			width: 300px;
			top: 0px;
			left: 510px;
			height: 550px;
			margin: 10px;
			border: solid 1px #CCC;
			padding: 1px;
			overflow: auto;
			background: #FFF;
		}

		#footer {
			position: absolute;
			left: 10px;
			top: 520px;
			width: 500px;
			height: 40px;
			overflow: hidden;
			border: solid 1px #CCC;
		}

		#table_container {
			position: absolute;
			top: 10px;
			left: 10px;
			width: 500px;
			height: 500px;
			overflow: auto;
			background: url(images/grid.gif)
		}

		fieldset {
			padding: 0px 10px 5px 5px;
			border: solid 1px #725B7D;
		}
	</style>
	<script type="text/javascript" src="../../core/popup.js"></script>
	<script type="text/javascript" src="../../core/dialog.js"></script>
	<script type="text/javascript" src="table-editor.js"></script>
	<script type="text/javascript">
	//<![CDATA[

		var testtable = '<table id="test2" border="1" cellpadding="0" cellspacing="1" style="width:400px;background:#CCC;background-color:#CCC; background-image:url(images/i.gif)" summary="test2" onclick="if (this.id=\'test4\') {alert(\'s\');}">' +
						'<caption>Caption</caption>'+
						'<tr>'+
						'<td id="test_td0_0" colspan="3" align="center" style="border-color:#000;border-style:solid;border-width:1px;background-color:#6CF;">&nbsp;</td>'+
						'</tr>'+
						'<tr>'+
						'<td id="test_td1_0" rowspan="2" align="center" style="border-color:#000;border-style:solid;border-width:1px;background-color:#30F;">&nbsp;</td>'+
						'<td id="test_td1_1" headers="test_td1_0 test_td0_0 ">test</td>'+
						'<td id="test_td1_2" headers="test_td0_0 test_td1_0 ">&nbsp;</td>'+
						'</tr>'+
						'<tr>'+
						'<td id="test_td2_1" headers="test_td0_0 test_td1_0 ">&nbsp;</td>'+
						'<td id="test_td2_2" headers="test_td0_0 test_td1_0 ">&nbsp;</td>'+
						'</tr>'+
						'</table>';

		var bigtable = '<TABLE borderColor=#999999 cellSpacing=0 cellPadding=3 width="100%" border=1>'+
'<TBODY>'+
'<TR class=strat_map_table>'+
'<TH width="76%">'+
'<P align=justify>Summary</P></TH>'+
'<TH width="8%">'+
'<P align=center>Year</P></TH>'+
'<TH width="9%">'+
'<P align=center>Country</P></TH>'+
'<TH width="7%">'+
'<P align=center>File</P></TH></TR>'+
'<TR>'+
'<TD>'+
'<DIV align=justify>'+
'<P><STRONG>censored.</STRONG></P>'+
'<P>censored.</P></DIV></TD>'+
'<TD>'+
'<DIV align=center>2002</DIV></TD>'+
'<TD>'+
'<DIV align=center>UK</DIV></TD>'+
'<TD>'+
'<P align=center><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Shootings_Gangs_Violence_in_Manchester_Summary.pdf"><IMG alt="Executive Summary" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/es_pdf_icon.jpg" border=0></A></P>'+
'<P align=center><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Shootings_Gangs_Violence_in_Manchester_(02).pdf"><IMG alt="PDF Document" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/pdf_icon.jpg" border=0></A>&nbsp;</P></TD></TR>'+
'<TR>'+
'<TD>'+
'<DIV align=justify>'+
'<P><STRONG>censored</STRONG></P>'+
'<P>censored.<BR></P></DIV></TD>'+
'<TD>'+
'<DIV align=center>2002</DIV></TD>'+
'<TD>'+
'<DIV align=center>USA</DIV></TD>'+
'<TD>'+
'<P align=center>&nbsp;<A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Crime_Gun_Interdiction_Strategies_Summary.pdf"><IMG alt="Executive Summary" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/es_pdf_icon.jpg" border=0></A></P><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Crime_Gun_Interdiction_Strategies_(02).pdf">'+
'<P align=center><IMG alt="PDF Document" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/pdf_icon.jpg" border=0></A></P></TD></TR>'+
'<TR>'+
'<TD><STRONG>censored.</STRONG>'+
'<P>censored.<BR></P></TD>'+
'<TD>'+
'<DIV align=center>2002</DIV></TD>'+
'<TD>'+
'<DIV align=center>Australia</DIV></TD>'+
'<TD>'+
'<P align=center><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Firearms_Theft_Summary.pdf"><IMG alt="Executive Summary" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/es_pdf_icon.jpg" border=0></A></P><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Firearms_Theft_(02).pdf">'+
'<P align=center><IMG alt="PDF Document" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/pdf_icon.jpg" border=0></P>'+
'<P align=center></A>&nbsp;</P></TD></TR>'+
'<TR>'+
'<TD><STRONG>censored.</STRONG>'+
'<P>censored.<BR></P></TD>'+
'<TD>'+
'<DIV align=center>2001</DIV></TD>'+
'<TD>'+
'<DIV align=center>Australia</DIV></TD>'+
'<TD>'+
'<P align=center><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Firearms_and_Violent_Crime_Summary.pdf"><IMG alt="Executive Summary" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/es_pdf_icon.jpg" border=0></A></P><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Firearms_and_Violent_Crime_(01).pdf">'+
'<P align=center><IMG alt="PDF Document" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/pdf_icon.jpg" border=0></A></P></TD></TR>'+
'<TR>'+
'<TD><STRONG>censored.</STRONG>'+
'<P>censored.<BR></P></TD>'+
'<TD>'+
'<DIV align=center>2001</DIV></TD>'+
'<TD>'+
'<DIV align=center>USA</DIV></TD>'+
'<TD>'+
'<P align=center><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Boston_Gun_Projects_Operation_Ceasefire_Summary.pdf"><IMG alt="Executive Summary" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/es_pdf_icon.jpg" border=0></A></P><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Boston_Gun_Projects_Operation_Ceasefire_(01).pdf">'+
'<P align=center><IMG alt="PDF Document" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/pdf_icon.jpg" border=0></A></P></TD></TR>'+
'<TR>'+
'<TD><STRONG>censored.</STRONG>'+
'<P>censored.<BR></P></TD>'+
'<TD>'+
'<DIV align=center>2001</DIV></TD>'+
'<TD>'+
'<DIV align=center>USA</DIV></TD>'+
'<TD>'+
'<P align=center><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Gun_Use_By_Male_Juveniles_Summary.pdf"><IMG alt="Executive Summary" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/es_pdf_icon.jpg" border=0></A></P><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Gun_Use_by_Male_Juveniles_(01).pdf">'+
'<P align=center><IMG alt="PDF Document" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/pdf_icon.jpg" border=0></A></P></TD></TR>'+
'<TR>'+
'<TD><STRONG>censored.</STRONG>'+
'<P>censored<BR></P></TD>'+
'<TD>'+
'<DIV align=center>2000</DIV></TD>'+
'<TD>'+
'<DIV align=center>USA</DIV></TD>'+
'<TD>'+
'<P align=center><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Reducing_Illegal_Firearms_Trafficking_Summary.pdf"><IMG alt="Executive Summary" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/es_pdf_icon.jpg" border=0></A></P><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Reducing_Illegal_Firearms_Trafficking_(00).pdf">'+
'<P align=center><IMG alt="PDF Document" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/pdf_icon.jpg" border=0></A></P></TD></TR>'+
'<TR>'+
'<TD><STRONG>censored.</STRONG>'+
'<P>censored<BR></P></TD>'+
'<TD>'+
'<DIV align=center>2000</DIV></TD>'+
'<TD>'+
'<DIV align=center>USA</DIV></TD>'+
'<TD>'+
'<P align=center><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Fighting_Juvenile_Gun_Violence_Summary.pdf"><IMG alt="Executive Summary" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/es_pdf_icon.jpg" border=0></A></P><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Fighting_Juvenile_Gun_Violence_(00).pdf">'+
'<P align=center><IMG alt="PDF Document" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/pdf_icon.jpg" border=0></A></P></TD></TR>'+
'<TR>'+
'<TD><STRONG>censored.</STRONG>'+
'<P>censored</P>'+
'<UL>'+
'<LI>censored'+
'<LI>censored'+
'<LI>censored'+
'<LI>censored<BR></LI></UL></TD>'+
'<TD>'+
'<DIV align=center>2000</DIV></TD>'+
'<TD>'+
'<DIV align=center>Australia</DIV></TD>'+
'<TD>'+
'<P align=center><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Crime-Safety-Firearms_Summary.pdf"><IMG alt="Executive Summary" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/es_pdf_icon.jpg" border=0></A></P><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Crime-Safety-Firearms_(00).pdf">'+
'<P align=center><IMG alt="PDF Document" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/pdf_icon.jpg" border=0></A></P></TD></TR>'+
'<TR>'+
'<TD><STRONG>censored.</STRONG>'+
'<P>censored.<BR></P></TD>'+
'<TD>'+
'<DIV align=center>2000</DIV></TD>'+
'<TD>'+
'<DIV align=center>Australia</DIV></TD>'+
'<TD>'+
'<P align=center><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/International_Traffic_In_Firearms_Summary.pdf"><IMG alt="Executive Summary" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/es_pdf_icon.jpg" border=0></A></P><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/International_Traffic_in_Firearms_(00).pdf">'+
'<P align=center><IMG alt="PDF Document" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/pdf_icon.jpg" border=0></A></P></TD></TR>'+
'<TR>'+
'<TD><STRONG>censored.</STRONG>'+
'<P>censored.<BR></P></TD>'+
'<TD>'+
'<DIV align=center>2000</DIV></TD>'+
'<TD>'+
'<DIV align=center>Australia</DIV></TD>'+
'<TD>'+
'<P align=center><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Firearms_Used_In_Homicide_Summary.pdf"><IMG alt="Executive Summary" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/es_pdf_icon.jpg" border=0></A></P><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Firearms_Used_In_Homicide_Summary.pdf">'+
'<P align=center><IMG alt="PDF Document" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/pdf_icon.jpg" border=0></A></P></TD></TR>'+
'<TR>'+
'<TD><STRONG>censored.</STRONG>'+
'<P>censored.<BR></P></TD>'+
'<TD>'+
'<DIV align=center>1999</DIV></TD>'+
'<TD>'+
'<DIV align=center>USA</DIV></TD>'+
'<TD>'+
'<P align=center><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Promising_Strategies_To_Reduce_Gun_Violence_Summary.pdf"><IMG alt="Executive Summary" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/es_pdf_icon.jpg" border=0></A></P><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Promising_Strategies_to_Reduce_Gun_Violence_(99).pdf">'+
'<P align=center><IMG alt="PDF Document" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/pdf_icon.jpg" border=0></A></P></TD></TR>'+
'<TR>'+
'<TD>'+
'<P><STRONG>censored.</STRONG></P>'+
'<P>censored</P>'+
'<UL>'+
'<LI>censored'+
'<LI>censored'+
'<LI>censored'+
'<LI>censored </LI></UL>'+
'<P>censored.<BR></P></TD>'+
'<TD>'+
'<DIV align=center>1999</DIV></TD>'+
'<TD>'+
'<DIV align=center>Australia</DIV></TD>'+
'<TD>'+
'<P align=center><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Firearm_Use_In_Homicide_Summary.pdf"><IMG alt="Executive Summary" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/es_pdf_icon.jpg" border=0></A><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Firearms_Used_In_Homicide_Summary.pdf"></A></P><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Firearm_Use_in_Homicide_(99).pdf">'+
'<P align=center><IMG alt="PDF Document" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/pdf_icon.jpg" border=0></P>'+
'<P align=center></A>&nbsp;</P></TD></TR>'+
'<TR>'+
'<TD><STRONG>censored</STRONG>'+
'<P>censored.<BR></P></TD>'+
'<TD>'+
'<DIV align=center>1999</DIV></TD>'+
'<TD>'+
'<DIV align=center>Australia</DIV></TD>'+
'<TD>'+
'<P align=center><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Firearm_Homicides_NSW_Summary.pdf"><IMG alt="Executive Summary" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/es_pdf_icon.jpg" border=0></A></P><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Firearm_Homicides_NSW_(99).pdf">'+
'<P align=center><IMG alt="PDF Document" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/pdf_icon.jpg" border=0></A></P></TD></TR>'+
'<TR>'+
'<TD><STRONG>censored.</STRONG>'+
'<P>censored<BR></P></TD>'+
'<TD>'+
'<DIV align=center>1999</DIV></TD>'+
'<TD>'+
'<DIV align=center>Australia</DIV></TD>'+
'<TD>'+
'<P align=center><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/International_Traffic_in_Small_Arms.pdf"><IMG alt="Executive Summary" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/es_pdf_icon.jpg" border=0></A></P><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/International_Traffic_in_Small_Arms_(99).pdf">'+
'<P align=center><IMG alt="PDF Document" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/pdf_icon.jpg" border=0></A></P></TD></TR>'+
'<TR>'+
'<TD><STRONG>censored</STRONG>'+
'<P>censored<BR></P></TD>'+
'<TD>'+
'<DIV align=center>1998</DIV></TD>'+
'<TD>'+
'<DIV align=center>USA</DIV></TD>'+
'<TD>'+
'<P align=center><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Predicting_Criminal_Behavior_Summary.pdf"><IMG alt="Executive Summary" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/es_pdf_icon.jpg" border=0></A></P><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Predicting_Criminal_Behaviour_(98).pdf">'+
'<P align=center><IMG alt="PDF Document" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/pdf_icon.jpg" border=0></A></P></TD></TR>'+
'<TR>'+
'<TD><STRONG>censored</STRONG>'+
'<P>censored<BR></P></TD>'+
'<TD>'+
'<DIV align=center>1997</DIV></TD>'+
'<TD>'+
'<DIV align=center>Australia</DIV></TD>'+
'<TD>'+
'<P align=center><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Firearms_Homicide_in_Australia_Summary.pdf"><IMG alt="Executive Summary" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/es_pdf_icon.jpg" border=0></A></P><A href="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/firearms/documents/Firearms_Homicide_in_Australia_(97).pdf">'+
'<P align=center><IMG alt="PDF Document" src="http://intranettst.police.nsw.gov.au/organisational_units/operations_command/operations_support_command/crime_management_faculty/strategy_maps/images/icons/pdf_icon.jpg" border=0></A></P></TD></TR></TBODY></TABLE>';

		var table;

		function Init() {
			__dlg_init("editTableProperties");
			table = new TTable("table");
			table.Import(window.dialogArguments.table_structure);
			window.focus();
		};

		function onOK() {
			var message = "";
			var obj = "";
			if (document.getElementById("tid").value == "") {
				message = "It is bad style to leave ID blank. Would you like to fix it?";
				obj = document.getElementById("tid");
			}
			if (message == "" || (message != "" && confirm(message) == false)) {
				__dlg_close("editTableProperties", table.Export());
			} else {
				obj.select();
			}
			return false;

		};

		function onCancel() {
			__dlg_close("editTableProperties", null);
			return false;

		};
		//]]>
	</script>
</head>
<body onload="Init();">
	<div id="table_container" onmousemove="table.mouse.Move(event)" onmouseout="table.mouse.Out()"></div>
	<div id="mouse_pointers">
		<img alt="" src="images/mousec.gif" id="mcell" style="position:absolute;display:none" />
		<img alt="" src="images/mouser.gif" id="mrow" style="position:absolute;display:none" />
		<img alt="" src="images/mouseh.gif" id="mhead" style="position:absolute;display:none" />
	</div>
	<div id="panels">
	<fieldset id="global_panel">
		<legend><?php echo translate('selectors'); ?></legend>
		<div>
			<img alt="" src="images/mc.gif" onclick="table.selector = 'cell'" />
			<img alt="" src="images/mr.gif" onclick="table.selector = 'row'" />
		</div>
	</fieldset>
	<fieldset id="cell_panel">
		<legend><?php echo translate('cell'); ?></legend>
		<div>
			<img id="addcolspan" title="Add ColSpan" alt="Add ColSpan" src="images/thr.gif" onclick="table.addColSpan();"/>
			<img id="delcolspan" alt="Delete ColSpan" title="Delete ColSpan" src="images/thl.gif" onclick="table.delColSpan();"/>
			<img id="addrowspan" alt="Add RowSpan" title="Add RowSpan" src="images/tvd.gif" onclick="table.addRowSpan();"/>
			<img id="delrowspan" alt="Delete RowSpan" title="Delete RowSpan" src="images/tvu.gif" onclick="table.delRowSpan();"/>
			<img alt="divider" src="images/div.gif" />
			<img id="THead" alt="Head Cell" title="Head Cell" src="images/th.gif" onclick="table.th();" />
		</div>
		<label for="abbr"><?php echo translate('abbr'); ?></label>
		<input id="abbr" name="abbr" onkeyup="table.setAbbr(this.value)" /><br />
		<label for="axis"><?php echo translate('axis'); ?></label>
		<input id="axis" name="axis"  onkeyup="table.setAxis(this.value)"/><br />
		<label for="scope"><?php echo translate('scope'); ?></label>
		<select id="scope" name="scope" onchange="table.setScope(this.value)">
			<option value=""><?php echo strtolower(translate('empty')); ?></option>
			<option value="row"><?php echo strtolower(translate('row')); ?></option>
			<option value="col"><?php echo strtolower(translate('col')); ?></option>
		</select><br />
		<label for="headings"><?php echo translate('headings'); ?></label>
		<button id="headings" name="headings" onclick="table.toggleHeaders();"><?php echo translate('click_to_select'); ?></button>
	</fieldset>
	<fieldset id="align_panel">
		<legend><?php echo translate('align'); ?></legend>
		<img id="aleft" title="Align Left" alt="Align Left" src="images/al.gif" onclick="table.setAlign('left')"/>
		<img id="acenter" title="Align Center" alt="Align Center" src="images/ac.gif" onclick="table.setAlign('center')"/>
		<img id="aright" title="Align Right" alt="Align Right" src="images/ar.gif" onclick="table.setAlign('right')"/><br />
		<img id="atop" title="Align Top" alt="Align Top" src="images/at.gif" onclick="table.setVAlign('top')"/>
		<img id="amiddle" title="Align Middle" alt="Align Middle" src="images/am.gif" onclick="table.setVAlign('middle')"/>
		<img id="abottom" title="Align Bottom" alt="Align Bottom" src="images/ab.gif" onclick="table.setVAlign('bottom')"/>
	</fieldset>
	<fieldset id="table_panel">
		<legend><?php echo translate('table'); ?></legend>
		<div>
			<img alt="Add Row" title="Add Row" src="images/addrow.gif" onclick="table.addrow();"/>
			<img alt="Delete Row" title="Delete Row" src="images/delrow.gif" onclick="table.delrow();"/>
			<img alt="Add Column" title="Add Column" src="images/addcol.gif" onclick="table.addcol();"/>
			<img alt="Delete Column" title="Delete Column" src="images/delcol.gif" onclick="table.delcol();"/>
		</div>
		<label for="tid"><?php echo translate('id'); ?></label>
		<input id="tid" name="tid" onkeyup="table.setID(this.value)" /><br />
		<label for="caption"><?php echo translate('caption'); ?></label>
		<input id="caption" name="caption" onkeyup="table.setCaption(this.value)"/><br />
		<label for="cellspacing"><?php echo translate('cell_spacing'); ?></label>
		<input id="cellspacing" name="cellspacing" onkeyup="table.setCellSpacing(parseInt(this.value))" value="2"/><br />
		<label for="cellpadding"><?php echo translate('cell_padding'); ?></label>
		<input id="cellpadding" name="cellpadding" onkeyup="table.setCellPadding(parseInt(this.value))" value="2"/><br />
		<label for="summary"><?php echo translate('summary'); ?></label>
		<textarea id="summary" cols="10" rows="3" onkeyup="table.setSummary(this.value)"></textarea>
		<label for="width"><?php echo translate('width'); ?></label>
		<input id="width" name="width" onkeyup="table.setWidth(parseInt(document.getElementById('width').value) + document.getElementById('widthtype').value)" style="width:100px" value="100" />
		<select id="widthtype" name="widthtype" style="width:50px" onchange="table.setWidth(parseInt(document.getElementById('width').value) + document.getElementById('widthtype').value)">
			<option value="px">px</option>
			<option value="%" selected="selected">%</option>
			<option value="pt">pt</option>
			<option value="em">em</option>
			<option value="ex">ex</option>
		</select><br />
		<label for="frame"><?php echo translate('frame'); ?></label>
		<select id="frame" name="frame" onchange="table.setFrame(this.value)">
			<option value=""><?php echo strtolower(translate('empty')); ?></option>
			<option value="void"><?php echo translate('no_sides'); ?></option>
			<option value="above"><?php echo translate('the_top_side_only'); ?></option>
			<option value="below"><?php echo translate('the_bottom_side_only'); ?></option>
			<option value="hsides"><?php echo translate('the_top_and_bottom_sides_only'); ?></option>
			<option value="vsides"><?php echo translate('the_right_and_left_sides_only'); ?></option>
			<option value="lhs"><?php echo translate('the_left-hand_side_only'); ?></option>
			<option value="rhs"><?php echo translate('the_right-hand_side_only'); ?></option>
			<option value="box"><?php echo translate('all_four_sides'); ?></option>
		</select><br />
		<label for="rules"><?php echo translate('rules'); ?></label>
		<select id="rules" name="rules" onchange="table.setRules(this.value)">
			<option value=""><?php echo strtolower(translate('empty')); ?></option>
			<option value="none"><?php echo translate('no_rules'); ?></option>
			<option value="rows"><?php echo translate('rules_will_appear_between_rows_only'); ?></option>
			<option value="cols"><?php echo translate('rules_will_appear_between_columns_only'); ?></option>
			<option value="all"><?php echo translate('rules_will_appear_between_all_rows_and_columns'); ?></option>
		</select><br />
	</fieldset>
	<fieldset id="color_panel">
		<legend><?php echo translate('colour'); ?></legend>
		<div style="width:40px;height:40px;position:relative;" id="bgborder" onclick="table.toggleBgBorder();">
			<div id="border" style="width:30px;height:30px;position:absolute;left:10px;top:10px;border:inset 1px;background:url(images/empty.gif)">
				<div style="width:19px;height:19px;position:absolute;left:5px;top:5px;border:outset 1px;background:#FFF;font-size:5px"></div>
			</div>
			<div id="bg" style="width:30px;height:30px;position:absolute;left:0px;top:0px;border:inset 1px;background:url(images/empty.gif)"></div>
		</div>
		<div id="colors" style="margin-top:5px" class="colors">
			<script type="text/javascript">
			//<![CDATA[
				var Clr = Array("0", "3", "6", "9", "C", "F");
				var out = '<table border="0" cellpadding="0" cellspacing="0" style="border-collapse: collapse">';
				for (h1 = 0; h1 < Clr.length; h1++) {
					if (h1 == 0) out += '<tr>' + '<td style="width:10px;height:10px;font-size:4px;" title="none" onclick="table.setColor(null)"><img src="images/empty.gif" width="10" height="10" alt="none" /></td>';
					else out += '<tr>' + '<td style="width:10px;height:10px;font-size:4px;background:#' + Clr[h1] + Clr[h1] + Clr[h1] + '" title="' + Clr[h1] + Clr[h1] + Clr[h1] + Clr[h1] + Clr[h1] + Clr[h1] + '" onclick="table.setColor(\'' + Clr[h1] + Clr[h1] + Clr[h1] + '\')"></td>';
					for (h2 = 0; h2 < Clr.length; h2++)
						for (h3 = 0; h3 < Clr.length; h3++) {
							color = Clr[h1] + Clr[h2] + Clr[h3];
							title = Clr[h1] + Clr[h1] + Clr[h2] + Clr[h2] + Clr[h3] + Clr[h3]
							out += '<td style="width:10px;height:10px;font-size:4px;background:#' + color + '" title="' + title + '" onclick="table.setColor(\'' + color + '\')"></td>';
						}
					out += '</tr>';
					}
				out += '</table>';
				document.write(out);
			//]]>
			</script>
		</div>
	</fieldset>
	</div>
	<div id="footer">
		<table border="0" cellpadding="0" cellspacing="0" style="width:100%;height:100%">
			<tr>
				<td>
					<button accesskey="s" type="button" style="width:60px;height:30px" onclick="onOK()"><?php echo translate('ok'); ?></button>
				</td>
				<td align="right">
					<button type="button" style="width:60px;height:30px" onclick="onCancel()"><?php echo translate('cancel'); ?></button>
				</td>
			</tr>
		</table>
	</div>
	<img alt="" src="images/semigray.gif" style="display:none;" id="semigray" />
	<img alt="" src="images/semired.gif" style="display:none;" id="semired" />
	<img alt="" src="images/empty.gif" style="display:none;" id="empty" />
</body>
</html>
