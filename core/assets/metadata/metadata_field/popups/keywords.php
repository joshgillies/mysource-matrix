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
* $Id: keywords.php,v 1.2 2006/05/16 00:41:47 rong Exp $
*
*/

	require_once dirname(__FILE__).'/../../../../../core/include/init.inc';
	require_once dirname(__FILE__).'/../../../../../core/lib/html_form/html_form.inc';
	if (!isset($_GET['assetid'])) return FALSE;
	$all = array_get_index($_REQUEST, 'all', FALSE);
	assert_valid_assetid($_GET['assetid']);
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
			<tr><td valign="top" width="200"><b>%asset_short_name%</b></td><td valign="top">Short name of the asset</td></tr>
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
			<tr><td valign="top" width="200"><b>%asset_read_permission%</b></td><td valign="top">Comma seperated list of the full names for users with read access</td></tr>
			<tr><td valign="top" width="200"><b>%asset_write_permission%</b></td><td valign="top">Comma seperated list of the full names for users with write access</td></tr>
			<tr><td valign="top" width="200"><b>%asset_admin_permission%</b></td><td valign="top">Comma seperated list of the full names for users with administrator access</td></tr>
			<tr><td valign="top" width="200"><b>%asset_read_permission_email%</b></td><td valign="top">Comma seperated list of email addresses for users with read access</td></tr>
			<tr><td valign="top" width="200"><b>%asset_write_permission_email%</b></td><td valign="top">Comma seperated list of email addresses for users with write access</td></tr>
			<tr><td valign="top" width="200"><b>%asset_admin_permission_email%</b></td><td valign="top">Comma seperated list of email addresses for users with administrator access</td></tr>
			<tr><td valign="top" width="200"><b>%asset_read_permission_email_linked%</b></td><td valign="top">Comma seperated list of linked email addresses for users with read access</td></tr>
			<tr><td valign="top" width="200"><b>%asset_write_permission_email_linked%</b></td><td valign="top">Comma seperated list of linked email addresses for users with write access</td></tr>
			<tr><td valign="top" width="200"><b>%asset_admin_permission_email_linked%</b></td><td valign="top">Comma seperated list of linked email addresses for users with administrator access</td></tr>
			</table>
		</fieldset>
		</p>

		<?php
			$roles = Array();
			if ($all) {
				$roles_temp = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('role');
				$users = Array();
				foreach ($roles_temp as $roleid) {
					$role =& $GLOBALS['SQ_SYSTEM']->am->getAsset($roleid);
					if ($role->readAccess(Array())) $roles[] = $roleid;
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($role);
				}
			} else {
				$roles = $GLOBALS['SQ_SYSTEM']->am->getRole($_GET['assetid']);
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
								<td valign="top">Comma seperated list of the full names for users/groups who can perform the "<?php echo $name; ?>" role</td>
							</tr>
							<tr>
								<td valign="top" width="200"><b>%asset_role_<?php echo $roleid; ?>_email%</b></td>
								<td valign="top">Comma seperated list of email addresses for users/groups who can perform the "<?php echo $name; ?>" role</td>
							</tr>
							<tr>
								<td valign="top" width="200"><b>%asset_role_<?php echo $roleid; ?>_email_linked%</b></td>
								<td valign="top">Comma seperated list of linked email addresses for users/groups who can perform the "<?php echo $name; ?>" role</td>
							</tr>
							<?php
						}
						?>
					</fieldset>
				</p>
				<?php
			}//end if
		?>
	</body>
</html>