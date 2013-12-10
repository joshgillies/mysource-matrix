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
* $Id: keywords.php,v 1.1 2013/09/24 01:10:35 ewang Exp $
*
*/

	require_once dirname(__FILE__).'/../../../../core/include/init.inc';
	require_once dirname(__FILE__).'/../../../../core/lib/html_form/html_form.inc';

	$assetid = array_get_index($_GET, 'assetid');
	if (is_null($assetid)) return FALSE;
	assert_valid_assetid($assetid);
	$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
	if (is_null($asset) || !$asset->writeAccess()) exit();

	$all = array_get_index($_REQUEST, 'all', FALSE);

	require_once dirname(__FILE__).'/../../../../core/include/backend_outputter.inc';
	$o = new Backend_Outputter();

	$o->openSection('Metadata Keyword Replacements');
	$o->openField('', 'wide_col');

?>

		<p>The following keyword replacements may be used for metadata values. Note that the percentage signs (%) are required.</p>

		<?php
		$o->openSection('Asset Information');
		$o->openField('', 'wide_col');
		?>
		
			<table class="sq-backend-table compact">
			<tr><th style="width: 250px;">Keyword</th><th>Description</th></tr>
			<tr><td><strong>%asset_assetid%</strong></td><td>The ID of the asset</td></tr>
			<tr><td><strong>%asset_name%</strong></td><td>Full name of the asset</td></tr>
			<tr><td><strong>%asset_name_linked%</strong></td><td>Full name of the asset with hyperlink</td></tr>
			<tr><td><strong>%asset_short_name%</strong></td><td>Short name of the asset</td></tr>			
			<tr><td><strong>%asset_short_name_linked%</strong></td><td>Short name of the asset with hyperlink</td></tr>
			<tr><td><strong>%asset_version%</strong></td><td>Version of the asset being displayed</td></tr>
			<tr><td><strong>%asset_url%</strong></td><td>URL of the asset</td></tr>
			<tr><td><strong>%asset_href%</strong></td><td>Relative HREF of the asset</td></tr>
			</table>

		<?php
		$o->closeField();
		$o->closeSection();

		$o->openSection('Creation Details');
		$o->openField('', 'wide_col');
		?>

			<table class="sq-backend-table compact">
			<tr><th style="width: 250px;">Keyword</th><th>Description</th></tr>
			<tr><td><strong>%asset_created%</strong></td><td>Date/Time the asset was created (YYYY-MM-DD HH-MM-SS)</td></tr>
			<tr><td><strong>%asset_created_short%</strong></td><td>Date/Time the asset was created (YYYY-MM-DD)</td></tr>
			<tr><td><strong>%asset_created_readable%</strong></td><td>Date/Time the asset was created (DD M YYYY H:MM[AM\PM])</td></tr>
			<tr><td><strong>%asset_created_by_name%</strong></td><td>Full name of the user that created this asset</td></tr>
			<tr><td><strong>%asset_created_by_first_name%</strong></td><td>First name of the user that created this asset</td></tr>
			<tr><td><strong>%asset_created_by_last_name%</strong></td><td>Surname of the user that created this asset</td></tr>
			<tr><td><strong>%asset_created_by_email%</strong></td><td>Email of the user that created this asset</td></tr>
			</table>

		<?php
		$o->closeField();
		$o->closeSection();
		
		$o->openSection('Last Updated Details');
		$o->openField('', 'wide_col');
		?>
		
			<table class="sq-backend-table compact">
			<tr><th style="width: 250px;">Keyword</th><th>Description</th></tr>
			<tr><td><strong>%asset_updated%</strong></td><td>Date/Time the asset was last updated (YYYY-MM-DD HH-MM-SS)</td></tr>
			<tr><td><strong>%asset_updated_short%</strong></td><td>Date/Time the asset was last updated (YYYY-MM-DD)</td></tr>
			<tr><td><strong>%asset_updated_readable%</strong></td><td>Date/Time the asset was last updated (DD M YYYY H:MM[AM\PM])</td></tr>
			<tr><td><strong>%asset_updated_by_name%</strong></td><td>Full name of the user that last updated this asset</td></tr>
			<tr><td><strong>%asset_updated_by_first_name%</strong></td><td>First name of the user that last updated this asset</td></tr>
			<tr><td><strong>%asset_updated_by_last_name%</strong></td><td>Surname of the user that last updated this asset</td></tr>
			<tr><td><strong>%asset_updated_by_email%</strong></td><td>Email of the user that last updated this asset</td></tr>
			</table>
			
		<?php
		$o->closeField();
		$o->closeSection();
		
		$o->openSection('Last Published Details');
		$o->openField('', 'wide_col');
		?>
		
			<table class="sq-backend-table compact">
			<tr><th style="width: 250px;">Keyword</th><th>Description</th></tr>
			<tr><td><strong>%asset_published%</strong></td><td>Date/Time the asset was last published (YYYY-MM-DD HH-MM-SS)</td></tr>
			<tr><td><strong>%asset_published_short%</strong></td><td>Date/Time the asset was last published (YYYY-MM-DD)</td></tr>
			<tr><td><strong>%asset_published_readable%</strong></td><td>Date/Time the asset was last published (DD M YYYY H:MM[AM\PM])</td></tr>
			<tr><td><strong>%asset_published_by_name%</strong></td><td>Full name of the user that last published this asset</td></tr>
			<tr><td><strong>%asset_published_by_first_name%</strong></td><td>First name of the user that last published this asset</td></tr>
			<tr><td><strong>%asset_published_by_last_name%</strong></td><td>Surname of the user that last published this asset</td></tr>
			<tr><td><strong>%asset_published_by_email%</strong></td><td>Email of the user that last published this asset</td></tr>
			</table>
			
		<?php
		$o->closeField();
		$o->closeSection();
		
		$o->openSection('Permissions');
		$o->openField('', 'wide_col');
		?>
		
			<table class="sq-backend-table compact">
			<tr><th style="width: 250px;">Keyword</th><th>Description</th></tr>
			<tr><td><strong>%asset_read_permission%</strong></td><td>Comma separated list of the full names for users with read access</td></tr>
			<tr><td><strong>%asset_write_permission%</strong></td><td>Comma separated list of the full names for users with write access</td></tr>
			<tr><td><strong>%asset_admin_permission%</strong></td><td>Comma separated list of the full names for users with administrator access</td></tr>
			<tr><td><strong>%asset_read_permission_email%</strong></td><td>Comma separated list of email addresses for users with read access</td></tr>
			<tr><td><strong>%asset_write_permission_email%</strong></td><td>Comma separated list of email addresses for users with write access</td></tr>
			<tr><td><strong>%asset_admin_permission_email%</strong></td><td>Comma separated list of email addresses for users with administrator access</td></tr>
			<tr><td><strong>%asset_read_permission_email_linked%</strong></td><td>Comma separated list of linked email addresses for users with read access</td></tr>
			<tr><td><strong>%asset_write_permission_email_linked%</strong></td><td>Comma separated list of linked email addresses for users with write access</td></tr>
			<tr><td><strong>%asset_admin_permission_email_linked%</strong></td><td>Comma separated list of linked email addresses for users with administrator access</td></tr>
			</table>
			
		<?php
		$o->closeField();
		$o->closeSection();
		
			$roles = Array();
			if ($all) {
				$roles_temp = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('role');
				$users = Array();
				foreach ($roles_temp as $roleid) {
					$role = $GLOBALS['SQ_SYSTEM']->am->getAsset($roleid);
					if ($role->readAccess(Array())) $roles[] = $roleid;
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($role);
				}
			} else {
				$roles = $GLOBALS['SQ_SYSTEM']->am->getRole($assetid);
				$roles = array_keys($roles);
			}

			if (!empty($roles)) {
		
				$o->openSection('Roles');
				$o->openField('', 'wide_col');
				?>
					
					<table class="sq-backend-table compact">
					<?php
					$roleinfo = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($roles, Array(), FALSE, 'name');
					foreach ($roleinfo as $roleid => $name) {
						?>

						<tr><th style="width: 250px;">Keyword</th><th>Description</th></tr>
						<tr>
							<td colspan="2"><strong><?php echo $name; ?></strong></td>
						</tr>
						<tr>
							<td><strong>%asset_role_<?php echo $roleid; ?>%</strong></td>
							<td>Comma separated list of the full names for users/groups who can perform the "<?php echo $name; ?>" role</td>
						</tr>
						<tr>
							<td><strong>%asset_role_<?php echo $roleid; ?>_email%</strong></td>
							<td>Comma separated list of email addresses for users/groups who can perform the "<?php echo $name; ?>" role</td>
						</tr>
						<tr>
							<td><strong>%asset_role_<?php echo $roleid; ?>_email_linked%</strong></td>
							<td>Comma separated list of linked email addresses for users/groups who can perform the "<?php echo $name; ?>" role</td>
						</tr>
						<?php
					}
					?>
					</table>
					
				<?php
				$o->closeField();
				$o->closeSection();
			}//end if

		$o->openSection('All Keywords (dynamically populated)');
		$o->openField('', 'wide_col');
		?>

		<table class="sq-backend-table compact">
			<tr><th style="width: 250px;">Keyword</th><th>Description</th></tr>
		<?php
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		$keywords = $asset->getAvailableKeywords();
		ksort($keywords);

		foreach ($keywords as $keyword => $description) {
			?>
			<tr>
				<td><strong>%<?php echo $keyword; ?>%</strong></td>
				<td><?php echo $description; ?></td>
			</tr>
			<?php
		}

		?>
		</table>

		<?php
		$o->closeField();
		$o->closeSection();

		$o->openField('', 'commit');
		normal_button('cancel', translate('close_window'), 'window.close()', 'style="margin-bottom: 20px;"');
		$o->closeSection();
		$o->paint();
		?>

	</body>
</html>
