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
* $Id: page_contents_keywords.php,v 1.1 2004/09/21 00:41:40 lwright Exp $
* $Name: not supported by cvs2svn $
*/

	require_once dirname(__FILE__).'/../../../../../core/include/init.inc';
	require_once dirname(__FILE__).'/../../../../../core/lib/html_form/html_form.inc';
	if (!isset($_GET['assetid'])) return false;
	
	assert_valid_assetid($_GET['assetid']);
	$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($_GET['assetid']);
	if (!is_a($asset, 'form')) {
		trigger_error('The assetid passed to this popup (# '.$asset.') represents an asset that is not a Custom Form', E_USER_ERROR);
		return false;
	}
?>

<html>
	<head>
		<title>'<?php echo $asset->attr('name') ?>' Page Contents Keyword Replacements</title>
		<style>
			body {
				background-color:	#FFFFFF;
			}

			body, p, td, ul, li, input, select, textarea{
				color:				#000000;
				font-family:		Arial, Verdana Helvetica, sans-serif;
				font-size:			11px;
			}

			fieldset {
				padding:			0px 10px 5px 5px;
				border:				1px solid #E0E0E0;
			}

			legend {
				color:				#2086EA;
			}
		</style>
	</head>

	<body>
	<?php
		require_once dirname(__FILE__).'/../../../../../core/include/backend_outputter.inc';
		//$backend = new Backend();
		$o =& new Backend_Outputter();

		$o->openSection('Keyword List for \''.$asset->attr('name').'\' (#'.$asset->id.')');
		$o->openField('&nbsp;');
		
		$questions = $asset->getQuestions();
		$sections  = $asset->getSections();
	?>
				<p>These keywords are available for use in Complex Formatting for insertion into the 'Page Contents' bodycopy, if it is enabled. The <b>'Question Field'</b> keywords (%question_field_*%) are replaced with the appropriate input field for that question. The <b>'Section Title'</b> keywords (%section_title_*%) will be replaced with the name of the section.</p>
		
		<p>
		<fieldset>
			<legend><b>Unattached Questions</b></legend>
			<table border="0" width="100%">		
				<?php
					foreach ($questions as $q_id => $question) {
						$q_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($asset->id.':q'.$q_id);
						$q_name = $q_asset->attr('name');
						?>							<tr><td valign="top" width="200"><b>%question_field_<?php echo $asset->id.'_q'.$q_id ?>%</b></td><td valign="top"><?php echo $q_name ?></td></tr><?php
					}?>
			</table>
		</fieldset>
		</p>
					
			<?php
				foreach ($sections as $section) { ?>
				<p>
					<fieldset>
					<legend><b>Form Section Asset '<?php echo $section->attr('name') ?>' (Id: #<?php echo $section->id ?>)</b></legend>
						<table border="0" width="100%">
							<tr><td valign="top" width="200"><b>%section_title_<?php echo $section->id ?>%</b></td><td valign="top">Section Title</td></tr>
				<?php
					$replacements['section_title_'.$section->id] = $section->attr('name');
					$questions = &$section->getQuestions();
					foreach ($questions as $q_id => $question) {
						$q_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($section->id.':q'.$q_id);
						$q_name = $section->attr('name').': '.$q_asset->attr('name');
						?>
						<tr><td valign="top" width="200"><b>%question_field_<?php echo $section->id.'_q'.$q_id; ?>%</b></td><td valign="top"><?php echo $q_name; ?></td></tr><?php } ?>
						</table>
					</fieldset>
				</p>
				<?php } ?>		

				
			</table>
		</fieldset>
		</p>

<?php
$o->openField('', 'commit');
normal_button('cancel', 'Close Window', 'window.close()');
$o->closeSection();
$o->paint();
?>
	</body>
</html>