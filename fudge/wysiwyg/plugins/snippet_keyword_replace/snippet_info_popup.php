<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Lth       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: snippet_info_popup.php,v 1.4 2013/04/23 08:09:05 cupreti Exp $
*
*/

/**
* Embed Movie Popup for the WYSIWYG
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix
*/


require_once dirname(__FILE__).'/../../../../core/include/init.inc';
if (empty($GLOBALS['SQ_SYSTEM']->user) || !($GLOBALS['SQ_SYSTEM']->user->canAccessBackend() || $GLOBALS['SQ_SYSTEM']->user->type() == 'simple_edit_user' || (method_exists($GLOBALS['SQ_SYSTEM']->user, 'isShadowSimpleEditUser') && $GLOBALS['SQ_SYSTEM']->user->isShadowSimpleEditUser()))) {
	exit;
}

?>
<html style="height: 500px;">
	<head>
		<title>Global Snippet Keyword Information</title>
		<link rel="stylesheet" type="text/css" href="<?php echo sq_web_path('lib').'/web/css/edit.css' ?>" />
		<link rel="stylesheet" type="text/css" href="<?php echo sq_web_path('root_url')?>/__fudge/wysiwyg/core/popup.css" />

	</head>

	<body>
		<div class="sq-popup-heading-frame title">
		  <h1>Global Snippet Keyword Information</h1>
		</div>

		<div style="padding: 0 10px;" id="main-form">
			<div class="tip">
				<p class="sq-backend-smallprint"><strong>Tip</strong>: The format of a snippet keyword is %globals_snippet_&lt;id&gt;_&lt;name&gt%. </p>
				<p class="sq-backend-smallprint"><strong>Note</strong>: The "name" part is optional.</p>
			</div>
			<?php
			$snippets = $GLOBALS['SQ_SYSTEM']->am->getSnippetKeywords(TRUE);
			?>
			<table class="sq-backend-table" cellpadding="5" cellspacing="0">
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
