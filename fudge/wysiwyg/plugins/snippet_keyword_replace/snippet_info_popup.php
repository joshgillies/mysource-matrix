<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Lth       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: snippet_info_popup.php,v 1.1 2007/10/25 23:19:40 rong Exp $
*
*/

/**
* Embed Movie Popup for the WYSIWYG
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix
*/


require_once dirname(__FILE__).'/../../../../core/include/init.inc';

?>
<html style="width: 740px; height: 500px;">
	<head>
		<title>Global Snippet Keyword Information</title>
		<style>
			h1 {font:bold 14pt "Lucida Grande", Tahoma; background: #402F48; color: #FFFFFF; height:30px; text-align:center}
			div.tip {font:bold 10pt "Lucida Grande", Tahoma; background: #FFFFCC; color:#003366; border:solid 1px #003366; padding-left:5px;  padding-right:5px;  padding-top:5px;  padding-bottom:5px; text-align:justify; margin-top:.5cm; margin-bottom:.5cm}
			td {font:10pt "Lucida Grande", Tahoma; color:#000000; background:#FFFFFF;}
			th {font:bold 10pt "Lucida Grande", Tahoma; color:#000000; background:#C0C0C0;}
			table.visible {border:solid 1px #000000;border-collapse:collapse; width:100%}
			table.visible td {border:solid 1px #000000;border-collapse:collapse; height: 40px; text-align:justify}
			table.visible th {border:solid 1px #000000;border-collapse:collapse; height: 30px; text-align:center}
			table.visible td.icon {text-align:center}
		</style>
	</head>

	<body>
		<h1>Global Snippet Keyword Information</h1>
		<div style="padding: 0 10px;">
			<div class="tip">
				TIP: The format of a snippet keyword is %globals_snippet_&lt;id&gt;_&lt;name&gt%. <br />
				Note: The "name" part is optional.
			</div>
			<?php
			$snippets = $GLOBALS['SQ_SYSTEM']->am->getSnippetKeywords(TRUE);
			?>
			<table class="visible" cellpadding="5" cellspacing="0">
				<thead>
					<tr>
						<th>Snippet Name and Keyword</th>
						<th>Description</th>
					</tr>
				</thead>
				<tbody>
				<?php
				foreach ($snippets as $id => $info) {
					$desc = (isset($info['attr']['desc'])) ? $info['attr']['desc'] : '&nbsp;';
					?>
					<tr>
						<td nowrap="nowrap">
							<strong><?php echo $info['name']; ?></strong><br />
							%globals_snippet_<?php echo $id.'_'.$info['safe_name']; ?>%
						</td>
						<td>
							<?php echo $desc ?>
						</td>
					</tr?
					<?php
				}
				?>
				</tbody>
			</table>
		</div>
		&nbsp;
	</body>
</html>