<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: char_map.php,v 1.4 2006/12/06 05:11:10 bcaldwell Exp $
*
*/

/**
* Character Map popup for the WYSIWYG
*
* @author	Darren McKee	<dmckee@squiz.net>
* @version	$Revision: 1.4 $
* @package	WYSIWYG
*/




	$entities = Array(
					'&Agrave;',
					'&agrave;',
					'&Aacute;',
					'&aacute;',
					'&Acirc;',
					'&acirc;',
					'&Atilde;',
					'&atilde;',
					'&Auml;',
					'&auml;',
					'&Aring;',
					'&aring;',
					'&AElig;',
					'&aelig;',
					'&Ccedil;',
					'&ccedil;',
					'&ETH;',
					'&eth;',
					'&Egrave;',
					'&egrave;',
					'&Eacute;',
					'&eacute;',
					'&Ecirc;',
					'&ecirc;',
					'&Euml;',
					'&euml;',
					'&Igrave;',
					'&igrave;',
					'&Iacute;',
					'&iacute',
					'&Icirc;',
					'&icirc;',
					'&Iuml;',
					'&iuml;',
					'&micro;',
					'&Ntilde;',
					'&ntilde;',
					'&Ograve;',
					'&ograve;',
					'&Oacute;',
					'&oacute;',
					'&Ocirc;',
					'&ocirc;',
					'&Otilde;',
					'&otilde;',
					'&Ouml;',
					'&ouml;',
					'&Oslash;',
					'&oslash;',
					'&szlig;',
					'&THORN;',
					'&thorn;',
					'&Ugrave;',
					'&ugrave;',
					'&Uacute;',
					'&uacute;',
					'&Ucirc;',
					'&ucirc;',
					'&Uuml;',
					'&uuml;',
					'&Yacute;',
					'&yacute;',
					'&yuml;',
					'&uml;',
					'&macr;',
					'&acute;',
					'&cedil;',
					'&iexcl;',
					'&iquest;',
					'&middot;',
					'&brvbar;',
					'&laquo;',
					'&raquo;',
					'&para;',
					'&sect;',
					'&copy;',
					'&reg;',
					'&sup1;',
					'&sup2;',
					'&sup3;',
					'&times;',
					'&divide;',
					'&frac14;',
					'&frac12;',
					'&frac34;',
					'&ordf;',
					'&ordm;',
					'&not;',
					'&deg;',
					'&plusmn;',
					'&curren;',
					'&cent;',
					'&pound;',
					'&yen;',
					'&Delta;',
					'&fnof;',
					'&Omega;',
					'&OElig;',
					'&oelig;',
					'&Scaron;',
					'&scaron;',
					'&Yuml;',
					'&circ;',
					'&tilde;',
					'&ndash;',
					'&mdash;',
					'&dagger;',
					'&Dagger;',
					'&bull;',
					'&hellip;',
					'&lsquo;',
					'&rsquo;',
					'&ldquo;',
					'&rdquo;',
					'&lsaquo;',
					'&rsaquo;',
					'&trade;',
					'&radic;',
					'&infin;',
					'&int;',
					'&part;',
					'&ne;',
					'&le;',
					'&ge;',
					'&sum;',
					'&permil;',
					'&prod;',
					'&pi;',
					'&loz;',
					'&shy;',
				);

	$col_limit = 10;


?>
<html style="width: 280px; height: 380px;">
	<head>
		<TITLE>Click a character to Insert:</TITLE>
		<script type="text/javascript" src="../../core/popup.js"></script>
		<script type="text/javascript" src="../../core/dialog.js"></script>
		<script type="text/javascript">

			//var popupWidth = <?php echo $_REQUEST['width'];?>;
			//var popupHeight = <?php echo $_REQUEST['height'];?>;

			self.focus();

			function Init() {
				__dlg_init("insertSpecialChar");
			};

			function onOK(entity) {
				// pass data back to the calling window
				var param = new Object();

				param['entity'] = entity;
				__dlg_close("insertSpecialChar", param);

				return false;
			};

			function onCancel() {
				__dlg_close("insertSpecialChar", null);

				return false;
			};

			function createDivContents(entity) {
				var retVal = '<table>';
				retVal += '<tr><td rowspan=2 class="displaychar" >';
				retVal += entity;
				retVal += '</td></tr>';
				retVal += '<tr><td>' + entity.replace('&', '&amp;');
				retVal += '</td></tr></table>';
				return retVal;
			}


			function viewTD(event, td, entity, entityText) {
				td.className = 'highlighted';
				var floater = document.getElementById('floater');
				floater.style.left = event.clientX < 140 ? parseInt(event.clientX) + 10 + 'px' : parseInt(event.clientX) - 120 + 'px';
				floater.style.top = event.clientY < 250 ? parseInt(event.clientY) + 10 + 'px' : parseInt(event.clientY) - 80 + 'px';
				floater.style.display = "";
				floater.innerHTML = createDivContents(entityText);
				return true;
			}

			function stopView(td) {
				td.className = "character";
				var floater = document.getElementById('floater');
				floater.style.display = "none";
			}
		</script>
		<style type="text/css">

			body {
				background-color: #FCFCFC;
			}

			td.character {
				width: 25px;
				height: 25px;
				text-align: center;
				background-color: #DEDEDE;
				font-weight: bold;
				border: 1px solid #725B7D;
			}

			td.highlighted {
				width: 25px;
				height: 25px;
				text-align: center;
				font-weight: bold;
				border: 1px dashed grey;
				background-color: #725B7D;
			}

			div.floating {
				width: 100px;
				height: 60px;
				position: absolute;
				/*overflow: none;*/
				z-index: 1000;
				font-size: 100%;
				background-color: #FFFFFF;
				filter:alpha(opacity=90);
				-moz-opacity:0.9;
				opacity: 0.9;
			}

			.displaychar {
				font-size: 230%;
				font-weight: 400;
			}
		</style>
	</head>
	<body onload="Init()">
	<div style="margin-left: 15px; margin-top: 10px;">
		<table cellspacing=0 cellpadding=0 style="cursor: pointer; font-weight: 200; border: 1px solid black;" >
			<tr>
			<?php
			foreach ($entities as $id => $entity) {
				if ($id > 0 && ($id % $col_limit == 0)) {
					?>
					</tr><tr>
					<?php
				}?>
				<td class="character" onclick="onOK('<?php echo $entity; ?>');" onMouseOver="viewTD(event, this, '<?php echo $entity; ?>', '<?php echo htmlspecialchars($entity); ?>');" onmouseout="stopView(this);" >
				<?php
					echo $entity;
					//Below ensures firefox prints the td for the cell containing only &shy; entity
					if ($entity == '&shy;') echo '&nbsp';
				?>
				</td>
			<?php
			}?>
			</tr>
		</table>
	</div>
	<div id="floater" class="floating" style="border: 2px solid rgb(64, 47, 160); display:none;" >
		&nbsp;
	</div>
</body>
</html>

