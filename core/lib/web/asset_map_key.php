<!--
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
* $Id: asset_map_key.php,v 1.3 2003/11/26 00:51:14 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/
-->

<html>
	<head>
		<title>Asset Map Key</title>
		<style>
			body {
				background-color:	#FFFFFF;
			}

			body, p, td, ul, li, input, select, textarea{
				color:				#000000;
				font-family:		Arial, Verdana Helvetica, sans-serif;
				font-size:			10px;
			}

			h1 {
				font-size:			14px;
				font-weight:		bold;
			}

			.status-colour-div {
				padding:			2px;
				padding-left:		5px;
				color:				#000000;
				border:				1px solid #000000;
				font-size:			10px;
				font-weight:		bold;
				letter-spacing:		2px;
				text-align:			left;
				width:				300px;
				margin:				5px;
			}
		</style>
	</head>

	<body>
		<h1>Status Colours</h1>

		<div class="status-colour-div" style="background-color: #A59687;">archived</div>
		<div class="status-colour-div" style="background-color: #78C7EB;">under contruction</div>
		<div class="status-colour-div" style="background-color: #AF9CC5;">pending approval</div>
		<div class="status-colour-div" style="background-color: #F4D425;">approved</div>
		<div class="status-colour-div" style="background-color: #B1DC1B;">live</div>
		<div class="status-colour-div" style="background-color: #F25C86;">safe editing</div>
		<div class="status-colour-div" style="background-color: #CCCCCC;">safe edit pending approval</div>
		<div class="status-colour-div" style="background-color: #FF9A00;">safe edit approved</div>
	</body>
</html>