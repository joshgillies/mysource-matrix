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
* $Id: thank_you_keywords.php,v 1.1 2013/09/24 01:10:35 ewang Exp $
*
*/

	require_once dirname(__FILE__).'/../../../../core/include/init.inc';
	require_once dirname(__FILE__).'/../../../../core/lib/html_form/html_form.inc';
	if (!isset($_GET['assetid'])) return FALSE;

	assert_valid_assetid($_GET['assetid']);
	$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($_GET['assetid']);
	if (!($asset instanceof Form)) {
		trigger_error('Asset is not a form');
		return FALSE	;
	}

	require_once dirname(__FILE__).'/../../../../core/include/backend_outputter.inc';
	// $backend = new Backend();
	$o = new Backend_Outputter();

	$o->openSection('\''.$asset->attr('name').'\' Thank You / Emails Keyword Replacements');
	$o->openField('', 'wide_col');

	if ($asset->readAccess()) {

		$questions = $asset->getQuestions();
		$sections  = $asset->getSections();
		?>
		<p>These keywords are available for use in Complex Formatting for insertion into the 'Thank You' bodycopy, if it is enabled, as well as in emails sent from this form. The <b>'Response'</b> keywords (%response_*%) are replaced with the actual response for that question. The <b>'Section Title'</b> keywords (%section_title_*%) will be replaced with the name of the section.</p>

		<table class="sq-backend-table compact">
			<tr><th colspan="2">Unattached Questions</th></tr>
			<?php
				foreach ($questions as $q_id => $question) {
				?>
					<tr>
						<td>%response_<?php echo $asset->id.'_q'.$q_id; ?>%</td>
						<td><?php echo get_asset_tag_line($asset->id.':q'.$q_id); ?></td>
					</tr>
					<?php
				}
			?>
		</table>

		<?php
			foreach ($sections as $section) {
			?>
				<table class="sq-backend-table compact">
					<tr><th colspan="2">Section Questions: <?php echo $section->name ?></th></tr>
					<tr><td>%section_title_<?php echo $section->id ?>%</td><td><?php echo get_asset_tag_line($section->id); ?></td></tr>

					<?php
						$replacements['section_title_'.$section->id] = $section->attr('name');
						$questions = $section->getQuestions();
						foreach ($questions as $q_id => $question) {
							?>
								<tr>
									<td>%response_<?php echo $section->id.'_q'.$q_id; ?>%</td>
									<td><?php echo get_asset_tag_line($section->id.':q'.$q_id); ?></td>
								</tr>
							<?php
						}
					?>
				</table>
			<?php
		}

	} else {
		echo "<p><strong>You do not have required access to view this page.</strong></p>";
	}
	?>

	<?php
	$o->closeField();

	$o->openField('', 'commit');
	normal_button('cancel', translate('close_window'), 'window.close()', '');
	$o->closeSection();
	$o->paint();
	?>

	</body>
</html>
