<!--
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: asset.xsl,v 1.4 2003/09/26 05:26:35 brobertson Exp $
* $Name: not supported by cvs2svn $
*/
-->

<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:output method="html" indent="yes"
    doctype-public="-//W3C//DTD HTML 4.01//EN"
    doctype-system="http://www.w3.org/TR/1999/REC-html401-19991224/strict.dtd" />

<xsl:template match="/">

<html>
<head>
	<title>Asset Info for <xsl:value-of select="asset_info/package" />:<xsl:value-of select="asset_info/type_code" /></title>
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
			<h1><xsl:value-of select="asset_info/name" /></h1>
			<table border="1" width="50%" cellpadding="3">
				<tr>
					<td class="title">
						Code Name :
					</td>
					<td class="data">
						<xsl:value-of select="asset_info/type_code" />
					</td>
				</tr>
				<tr>
					<td class="title">
						Package :
					</td>
					<td class="data">
						<xsl:value-of select="asset_info/package" />
					</td>
				</tr>
				<tr>
					<td class="title">
						Version :
					</td>
					<td class="data">
						<xsl:value-of select="asset_info/version" />
					</td>
				</tr>
				<tr>
					<td class="title">
						Instantiable :
					</td>
					<td class="data">
						<xsl:value-of select="asset_info/version" />
					</td>
				</tr>
				<tr>
					<td class="title">
						Allowed to create :
					</td>
					<td class="data">
						<xsl:value-of select="asset_info/allowed_access" />
					</td>
				</tr>
				<tr>
					<td class="title">
						Type :
					</td>
					<td class="data">
						<xsl:value-of select="asset_info/parent_type" />
					</td>
				</tr>
				<tr>
					<td class="title">
						Description :
					</td>
					<td class="data">
						<xsl:value-of select="asset_info/description" />
					</td>
				</tr>
				<tr>
					<td class="title">
						Requirements :
					</td>
					<td class="data">
						<table width="100%">
							<tr>
								<td class="sub-title">
									Type
								</td>
								<td class="sub-title">
									Code
								</td>
								<td class="sub-title">
									Version
								</td>
							</tr>
						<xsl:for-each select="asset_info/requirements/requirement">
							<tr>
								<td class="sub-data">
									<xsl:value-of select="@type" />
								</td>
								<td class="sub-data">
									<xsl:value-of select="code" />
								</td>
								<td class="sub-data">
									<xsl:value-of select="version" />
								</td>
							</tr>
						</xsl:for-each>

						</table>
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
