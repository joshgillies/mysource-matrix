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
* $Id: keywords.php,v 1.10 2013/07/25 23:23:48 lwright Exp $
*
*/

	require_once dirname(__FILE__).'/../../../../../core/include/init.inc';
	require_once dirname(__FILE__).'/../../../../../core/lib/html_form/html_form.inc';

	$assetid = array_get_index($_GET, 'assetid');
	if (is_null($assetid)) return FALSE;
	assert_valid_assetid($assetid);
	$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
	if (is_null($asset) || !$asset->writeAccess()) exit();

	$all = array_get_index($_REQUEST, 'all', FALSE);

?>

<html>
	<head>
		<title>Metadata Keyword Replacements</title>
		<style>
			body {
				background-color:	#FFFFFF;
			}

			body, p, td, ul, li, input, select, textarea{
				color:				#000000;
				font-family:		Arial, Verdana Helvetica, sans-serif;
				font-size:			10px;
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
		<p><b><i>The following keyword replacements may be used for metadata values.<br />Note that the percentage signs (%) are required.</i></b></p>

		<p>
		<fieldset>
			<legend><b>Asset Information</b></legend>
			<table border="0" width="100%">
			<tr><td valign="top" width="200"><b>%asset_assetid%</b></td><td valign="top">The ID of the asset</td></tr>
			<tr><td valign="top" width="200"><b>%asset_name%</b></td><td valign="top">Full name of the asset</td></tr>
			<tr><td valign="top" width="200"><b>%asset_name_linked%</b></td><td valign="top">Full name of the asset with hyperlink</td></tr>
			<tr><td valign="top" width="200"><b>%asset_short_name%</b></td><td valign="top">Short name of the asset</td></tr>			
			<tr><td valign="top" width="200"><b>%asset_short_name_linked%</b></td><td valign="top">Short name of the asset with hyperlink</td></tr>
			<tr><td valign="top" width="200"><b>%asset_version%</b></td><td valign="top">Version of the asset being displayed</td></tr>
			<tr><td valign="top" width="200"><b>%asset_url%</b></td><td valign="top">URL of the asset</td></tr>
			<tr><td valign="top" width="200"><b>%asset_href%</b></td><td valign="top">Relative HREF of the asset</td></tr>
			</table>
		</fieldset>
		</p>

		<p>
		<fieldset>
			<legend><b>Creation Details</b></legend>
			<table border="0" width="100%">
			<tr><td valign="top" width="200"><b>%asset_created%</b></td><td valign="top">Date/Time the asset was created (YYYY-MM-DD HH-MM-SS)</td></tr>
			<tr><td valign="top" width="200"><b>%asset_created_short%</b></td><td valign="top">Date/Time the asset was created (YYYY-MM-DD)</td></tr>
			<tr><td valign="top" width="200"><b>%asset_created_readable%</b></td><td valign="top">Date/Time the asset was created (DD M YYYY H:MM[AM\PM])</td></tr>
			<tr><td valign="top" width="200"><b>%asset_created_by_name%</b></td><td valign="top">Full name of the user that created this asset</td></tr>
			<tr><td valign="top" width="200"><b>%asset_created_by_first_name%</b></td><td valign="top">First name of the user that created this asset</td></tr>
			<tr><td valign="top" width="200"><b>%asset_created_by_last_name%</b></td><td valign="top">Surname of the user that created this asset</td></tr>
			<tr><td valign="top" width="200"><b>%asset_created_by_email%</b></td><td valign="top">Email of the user that created this asset</td></tr>
			</table>
		</fieldset>
		</p>

		<p>
		<fieldset>
			<legend><b>Last Updated Details</b></legend>
			<table border="0" width="100%">
			<tr><td valign="top" width="200"><b>%asset_updated%</b></td><td valign="top">Date/Time the asset was last updated (YYYY-MM-DD HH-MM-SS)</td></tr>
			<tr><td valign="top" width="200"><b>%asset_updated_short%</b></td><td valign="top">Date/Time the asset was last updated (YYYY-MM-DD)</td></tr>
			<tr><td valign="top" width="200"><b>%asset_updated_readable%</b></td><td valign="top">Date/Time the asset was last updated (DD M YYYY H:MM[AM\PM])</td></tr>
			<tr><td valign="top" width="200"><b>%asset_updated_by_name%</b></td><td valign="top">Full name of the user that last updated this asset</td></tr>
			<tr><td valign="top" width="200"><b>%asset_updated_by_first_name%</b></td><td valign="top">First name of the user that last updated this asset</td></tr>
			<tr><td valign="top" width="200"><b>%asset_updated_by_last_name%</b></td><td valign="top">Surname of the user that last updated this asset</td></tr>
			<tr><td valign="top" width="200"><b>%asset_updated_by_email%</b></td><td valign="top">Email of the user that last updated this asset</td></tr>
			</table>
		</fieldset>
		</p>

		<p>
		<fieldset>
			<legend><b>Last Published Details</b></legend>
			<table border="0" width="100%">
			<tr><td valign="top" width="200"><b>%asset_published%</b></td><td valign="top">Date/Time the asset was last published (YYYY-MM-DD HH-MM-SS)</td></tr>
			<tr><td valign="top" width="200"><b>%asset_published_short%</b></td><td valign="top">Date/Time the asset was last published (YYYY-MM-DD)</td></tr>
			<tr><td valign="top" width="200"><b>%asset_published_readable%</b></td><td valign="top">Date/Time the asset was last published (DD M YYYY H:MM[AM\PM])</td></tr>
			<tr><td valign="top" width="200"><b>%asset_published_by_name%</b></td><td valign="top">Full name of the user that last published this asset</td></tr>
			<tr><td valign="top" width="200"><b>%asset_published_by_first_name%</b></td><td valign="top">First name of the user that last published this asset</td></tr>
			<tr><td valign="top" width="200"><b>%asset_published_by_last_name%</b></td><td valign="top">Surname of the user that last published this asset</td></tr>
			<tr><td valign="top" width="200"><b>%asset_published_by_email%</b></td><td valign="top">Email of the user that last published this asset</td></tr>
			</table>
		</fieldset>
		</p>

		<p>
		<fieldset>
			<legend><b>Permissions</b></legend>
			<table border="0" width="100%">
			<tr><td valign="top" width="200"><b>%asset_read_permission%</b></td><td valign="top">Comma separated list of the full names for users with read access</td></tr>
			<tr><td valign="top" width="200"><b>%asset_write_permission%</b></td><td valign="top">Comma separated list of the full names for users with write access</td></tr>
			<tr><td valign="top" width="200"><b>%asset_admin_permission%</b></td><td valign="top">Comma separated list of the full names for users with administrator access</td></tr>
			<tr><td valign="top" width="200"><b>%asset_read_permission_email%</b></td><td valign="top">Comma separated list of email addresses for users with read access</td></tr>
			<tr><td valign="top" width="200"><b>%asset_write_permission_email%</b></td><td valign="top">Comma separated list of email addresses for users with write access</td></tr>
			<tr><td valign="top" width="200"><b>%asset_admin_permission_email%</b></td><td valign="top">Comma separated list of email addresses for users with administrator access</td></tr>
			<tr><td valign="top" width="200"><b>%asset_read_permission_email_linked%</b></td><td valign="top">Comma separated list of linked email addresses for users with read access</td></tr>
			<tr><td valign="top" width="200"><b>%asset_write_permission_email_linked%</b></td><td valign="top">Comma separated list of linked email addresses for users with write access</td></tr>
			<tr><td valign="top" width="200"><b>%asset_admin_permission_email_linked%</b></td><td valign="top">Comma separated list of linked email addresses for users with administrator access</td></tr>
			</table>
		</fieldset>
		</p>

		<?php
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
				?>
				<p>
					<fieldset>
						<legend><b>Roles</b></legend>
						<table border="0" width="100%">
						<?php
						$roleinfo = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($roles, Array(), FALSE, 'name');
						foreach ($roleinfo as $roleid => $name) {
							?>
							<tr>
								<td valign="top" width="200" colspan="2"><b><?php echo $name; ?></b></td>
							</tr>
							<tr>
								<td valign="top" width="200"><b>%asset_role_<?php echo $roleid; ?>%</b></td>
								<td valign="top">Comma separated list of the full names for users/groups who can perform the "<?php echo $name; ?>" role</td>
							</tr>
							<tr>
								<td valign="top" width="200"><b>%asset_role_<?php echo $roleid; ?>_email%</b></td>
								<td valign="top">Comma separated list of email addresses for users/groups who can perform the "<?php echo $name; ?>" role</td>
							</tr>
							<tr>
								<td valign="top" width="200"><b>%asset_role_<?php echo $roleid; ?>_email_linked%</b></td>
								<td valign="top">Comma separated list of linked email addresses for users/groups who can perform the "<?php echo $name; ?>" role</td>
							</tr>
							<?php
						}
						?>
						</table>
					</fieldset>
				</p>
				<?php
			}//end if

?>
<p>
<fieldset>
<legend><b>All Keywords</b> (dynamically populated)</legend>
<table border="0" width="100%">
<?php
$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
$keywords = $asset->getAvailableKeywords();
ksort($keywords);

foreach ($keywords as $keyword => $description) {
	?>
	<tr>
		<td valign="top" width="200"><b>%<?php echo $keyword; ?>%</b></td>
		<td valign="top"><?php echo $description; ?></td>
	</tr>
	<?php
}

?>
</table>
</fieldset>
</p>

	</body>
</html>
