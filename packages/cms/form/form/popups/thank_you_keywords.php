<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: thank_you_keywords.php,v 1.11.2.1 2013/06/24 05:26:56 ewang Exp $
*
*/

	require_once dirname(__FILE__).'/../../../../../core/include/init.inc';
	require_once dirname(__FILE__).'/../../../../../core/lib/html_form/html_form.inc';
	if (!isset($_GET['assetid'])) return FALSE;

	assert_valid_assetid($_GET['assetid']);
	$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($_GET['assetid']);
	if (!($asset instanceof Form)) {
		trigger_error('Asset is not a form');
		return FALSE	;
	}

	if ($asset->readAccess()) {
?>

<html>
	<head>
		<title>'<?php echo $asset->attr('name') ?>' Thank You / Emails Keyword Replacements</title>
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
		// $backend = new Backend();
		$o = new Backend_Outputter();

		$o->openSection('Keyword List for \''.$asset->attr('name').'\' (#'.$asset->id.')');
		$o->openField('&nbsp;');

		$questions = $asset->getQuestions();
		$sections  = $asset->getSections();
	?>
				<p>These keywords are available for use in Complex Formatting for insertion into the 'Thank You' bodycopy, if it is enabled, as well as in emails sent from this form. The <b>'Response'</b> keywords (%response_*%) are replaced with the actual response for that question. The <b>'Section Title'</b> keywords (%section_title_*%) will be replaced with the name of the section.</p>

		<p>
		<fieldset>
			<legend><b>Unattached Questions</b></legend>
			<table border="0" width="100%">
				<?php
					foreach ($questions as $q_id => $question) {
						?>							<tr><td valign="top" width="200"><b>%response_<?php echo $asset->id.'_q'.$q_id; ?>%</b></td><td valign="top"><?php echo get_asset_tag_line($asset->id.':q'.$q_id); ?></td></tr><?php
					}
					?>
			</table>
		</fieldset>
		</p>

				<?php
				foreach ($sections as $section) {
				?>
				<p>
					<fieldset>
					<legend><b><?php echo get_asset_tag_line($section->id); ?></b></legend>
						<table border="0" width="100%">
							<tr><td valign="top" width="200"><b>%section_title_<?php echo $section->id ?>%</b></td><td valign="top">Section Title</td></tr>
				<?php
					$replacements['section_title_'.$section->id] = $section->attr('name');
					$questions = $section->getQuestions();
					foreach ($questions as $q_id => $question) {
						?>
						<tr><td valign="top" width="200"><b>%response_<?php echo $section->id.'_q'.$q_id; ?>%</b></td><td valign="top"><?php echo get_asset_tag_line($section->id.':q'.$q_id); ?></td></tr>
					<?php
					}
					?>
						</table>
					</fieldset>
				</p>
				<?php
				}
				?>


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
<?php
	} else {
		echo "<b>You do not have required access to view this page</b>";
	}
?>
