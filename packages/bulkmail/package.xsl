<?xml version="1.0" encoding="UTF-8"?>

<!--
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: package.xsl,v 1.3 2008/11/24 22:28:17 mbrydon Exp $
*
*/
-->

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:output method="html" indent="yes"
    doctype-public="-//W3C//DTD HTML 4.01//EN"
    doctype-system="http://www.w3.org/TR/1999/REC-html401-19991224/strict.dtd" />

<xsl:template match="/">

<html>
<head>
	<title>Package Info for <xsl:value-of select="package_info/code_name" /></title>
	<!-- <link rel="stylesheet" href="discography.css" type="text/css" /> -->
	<style type="text/css">
		body, td, .title, .data, .sub-title, .sub-data {
			font-family: arial, verdana, sans-serif;
			font-size: 12px;
			color: #000000;
			background-color: #c0c0c0;
		}
		.title {
			font-weight: bold;
			text-align: right;
			vertical-align: top;
			white-space: nowrap;
		}
		.sub-title {
			font-size: 10px;
			font-weight: bold;
		}
		.sub-data {
			font-size: 10px;
		}
	</style>
</head>

<body>
<table border="0" cellspacing="0" cellpadding="0" width="100%" height="100%">
	<tr>
		<td align="center" valign="middle">
			<h1><xsl:value-of select="package_info/name" /></h1>
			<table border="1" width="50%" cellpadding="3">
				<tr>
					<td class="title">
						Code Name :
					</td>
					<td class="data">
						<xsl:value-of select="package_info/code_name" />
					</td>
				</tr>
				<tr>
					<td class="title">
						Version :
					</td>
					<td class="data">
						<xsl:value-of select="package_info/version" />
					</td>
				</tr>
				<tr>
					<td class="title">
						Description :
					</td>
					<td class="data">
						<xsl:value-of select="package_info/description" />
					</td>
				</tr>
			</table>


		</td>
	</tr>
</table>
</body>
</html>

</xsl:template>

</xsl:stylesheet>
