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
* $Id: metadataSchemas.inc,v 1.34 2013/03/18 05:28:14 ewang Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* MetaData Schemas Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.34 $
* @package MySource_Matrix
*/


/**
* Paint the interface for viewing metadata schemas (used in popups etc)
*
* @param object	$owner	the asset whose interface we are painting
* @param object	$o		the outputter class
* @param object	$ei		the edit interface
* @param array	$keywords	the list of keywords
*
* @return boolean
* @access public
*/
function paintInlineMetadataSchemas(Asset $owner, Backend_Outputter $o, Asset_Edit_Interface $ei, array $keywords=NULL)
{
	$prefix = $owner->getPrefix();

	$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
	$schemas = $mm->getAssetSchemaInfo($owner->id, NULL, TRUE, FALSE);

	$direct = Array();
	foreach ($schemas as $schemaid => $granted) {
		if (!isset($direct[$granted])) {
			$direct[$granted] = Array();
		}
		$direct[$granted][] = $schemaid;
	}
	ksort($direct);

	if (empty($direct)) return FALSE;

	$o->openSection(translate('Metadata Schemas'));

		$o->openField('&nbsp;');
		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header">
					<?php echo translate('Metadata Schema'); ?>

				</td>
				<td class="sq-backend-table-header" align="right">
					<?php echo translate('Access'); ?>

				</td>
			</tr>

			<?php
			foreach ($direct as $granted => $schemaids) {
				foreach ($schemaids as $schemaid) {
					if ($schemaid) {
						$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
						if (!$schema->id || !($schema instanceof Metadata_Schema)) {
							continue;
						}
					}

				?>
					<tr>
						<td class="sq-backend-table-cell">
						<?php
						echo get_asset_tag_line($schemaid);
						?>
						</td>
						<td class="sq-backend-table-cell" align="right">
							<span style="color: <?php echo (($granted == '0') ? 'red' : 'green'); ?>; font-weight: bold; "><?php echo (($granted == '0') ? translate('Denied') : translate('Applied')) ?></span>


						</td>
					</tr>
				<?php
				}
			}

		?></table><?php
		$o->closeField();
	$o->closeSection();

	return TRUE;

}//end paintInlineMetadataSchemas()


/**
* Paint the interface for managing metadata schamas
*
* @param object	$owner		the asset whose interface we are painting
* @param object	$o			the outputter class
* @param object	$ei			the edit interface
*
* @return boolean
* @access public
*/
function paintMetadataSchemas(Asset $owner, Backend_Outputter $o, Asset_Edit_Interface $ei)
{
	$prefix = $owner->getPrefix();

	$admin_access = $owner->adminAccess('metadata');

	$o->openSection(translate('Metadata Schemas'));


	$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
	$schemas = $mm->getAssetSchemaInfo($owner->id, NULL, NULL, TRUE, TRUE);

	$direct = Array();
	foreach ($schemas as $schemaid => $schema_info) {
		$granted = $schema_info['granted'];
		if (!isset($direct[$granted])) {
			$direct[$granted] = Array();
		}
		$direct[$granted][$schemaid] = $schema_info;
	}
	ksort($direct);



	$o->openSection('Existing Schemas');
		$o->openRaw();
			if (empty($direct)) {
				echo '<p><em>'.translate('There are no metadata schemas applied to this asset.').'</em></p>';

			} else {
				foreach ($direct as $granted => $schema_list) {
				?>
					<table class="sq-backend-table">
						<tr>
							<th class="sq-backend-table-header">
								<span style="color: <?php echo (($granted == '0') ? 'red' : 'green'); ?>"><?php echo (($granted == '0') ? translate('Denied') : translate('Applied')) ?> <?php echo translate('Schemas');?></span>



							</th>
							<th style="text-align: center;" width="100" class="sq-backend-table-header">
								<?php echo translate('Cascade?'); ?>

							</th>
							<?php
							if ($admin_access) {
								?><th style="text-align: center;" width="100" class="sq-backend-table-header"><?php echo translate('Delete?'); ?></th><?php

							}
							?>
						</tr>
					<?php
					foreach ($schema_list as $schemaid => $schema_detail) {
						if ($schemaid) {
							$schema_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($schemaid), 'metadata_schema', FALSE);
							if (empty($schema_info)) continue;
						}
					?>
						<tr>
							<td class="sq-backend-table-cell">
							<?php
							echo get_asset_tag_line($schemaid);
							?>
							</td>
							<td align="center" width="100" class="sq-backend-table-cell">
							<?php
							if ($admin_access) {
								check_box($prefix.'_metadata[cascades]['.$schemaid.']', 1, $schema_detail['cascades']);
							} else {
							?>
								<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo ($schema_detail['cascades'] ? 'tick' : 'cross'); ?>.gif" width="15" height="15" />
							<?php
							}
							?>
							</td>
							<?php
							if ($admin_access) {
								?><td align="center" width="100" class="sq-backend-table-cell"><?php
								check_box($prefix.'_metadata[delete]['.$schemaid.']');
								?></td><?php
							}
							?>
						</tr>
					<?php
					}// end foreach data
				?>
					</table>
				<?php
				}//end foreach inherited
			}//end if
		$o->closeRaw();
		if (!empty($direct)) {
			$o->sectionNote('The "Cascade?" option determines whether new children of this asset will automatically have the schema cascaded to them. If this setting is changed, existing non-dependant children will NOT be affected. If you want to apply a schema to existing children where they have not been before, please do a Manual Cascade after toggling this setting on.');
		}
	$o->closeSection();



	if ($admin_access) {

		$o->openSection('Add New Schema');
			$o->openField('Select Schema', '');
				echo '<span class="sq-inline-fields-wrapper bottom-margin">';
					combo_box($prefix.'_metadata[new][granted]', Array('' => '', '1' => translate('Apply'), '0' => translate('Deny')), FALSE, '');


					echo ' '.translate('Metadata Schema').' &nbsp; ';

					asset_finder($prefix.'_metadata[new][schemaid]', 0, Array('metadata_schema' => 'D'), 'sq_sidenav', FALSE, 'null', Array('clear'));
				echo '</span>';
			$o->closeField();
			$o->openField(translate('Cascade?'), '');

				check_box($prefix.'_metadata[new][cascades]', 1, TRUE);
				label(translate('Cascade this new schema to newly-created children'), $prefix.'_metadata[new][cascades]');

				$o->note('If unchecked, the schema will be applied or denied only to dependants of this asset, regardless of the Cascade Changes settings below. It will also not apply the metadata schema to any asset that becomes a new child of this asset (through creating a new asset, or moving an existing one, or creating a new link from one). This setting can be toggled after being applied.');
			$o->closeField();
		$o->closeSection();
	}

	$o->closeSection();

	if ($admin_access) {
		$kiddies = $GLOBALS['SQ_SYSTEM']->am->getChildren($owner->id);
		if (count($kiddies)) {
			$o->openSection(translate('Cascade Options'));

				$o->openField(translate('Cascade Options'));

					echo '<p><strong>'.translate('Note').':</strong> '.sprintf(translate('These actions will affect <strong>%1$s</strong> other assets below "<strong>%2$s</strong>"'), count($kiddies), htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)).'.</p>';


					check_box($prefix.'_metadata[cascade_changes]', '1', TRUE);
					label(translate('Cascade schema changes'), $prefix.'_metadata[cascade_changes]');

					echo '<br/>';
					check_box($prefix.'_metadata[manual_cascade]');
					label(translate('Manually cascade schemas'), $prefix.'_metadata[manual_cascade]');

				$o->closeField();
			$o->closeSection();
		}
	}

	return $admin_access;

}//end paintMetadataSchemas()


/**
* Process the interface for managing metadata schemas
*
* It returns TRUE is there is a change, FALSE on error or no change
*
* @param object	$owner	the asset whose interface we are painting
* @param object	$o		the outputter class
* @param object	$ei		the edit interface
*
* @return boolean
* @access public
*/
function processMetadataSchemas(Asset $owner, Backend_Outputter $o, Asset_Edit_Interface $ei)
{
	// if we dont have admin access, go away
	if ($owner->id && !$owner->adminAccess('')) {
		$GLOBALS['SQ_SYSTEM']->paintLogin(translate('Login'), sprintf(translate('You do not have permission to access the asset'), htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)));


		exit();
	}

	// if we dont have admin access, dont process anything
	if (!$owner->adminAccess('metadata')) return FALSE;

	$prefix = $owner->getPrefix();
	if (!isset($_POST[$prefix.'_metadata'])) return FALSE;

	$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();

	// are we cascading these schema changes to our children?
	$cascade_changes = FALSE;
	if (isset($_POST[$prefix.'_metadata']['cascade_changes']) && $_POST[$prefix.'_metadata']['cascade_changes']) {
		$cascade_changes = TRUE;
	}

	$schema_vars = Array();
	$post_data = $_POST[$prefix.'_metadata'];

	// apply a schema
	if (!empty($post_data['new']['schemaid']['assetid'])) {
		if (!isset($post_data['new']['granted']) || $post_data['new']['granted'] == '') {
			trigger_localised_error('SYS0268', sprintf(translate('You need to select the type of access in order to apply schema #%s'), $post_data['new']['schemaid']['assetid']), E_USER_NOTICE);
		} else {
			// Only cascade changes to lower assets if "cascade to new assets"
			// is set.
			if ($cascade_changes && isset($post_data['new']['cascades'])) {
				// cascading access of some kind, so queue it up for the HIPO
				$schema_vars[] = Array(
									'assetids'			=> Array($owner->id),
									'granted'			=> (int)$post_data['new']['granted'],
									'schemaid'			=> (int)$post_data['new']['schemaid']['assetid'],
									'cascades'			=> (int)(isset($post_data['new']['cascades']) ? 1 : 0),
									'previous_access'	=> NULL,
								 );
			} else {
				// lets do this ourselves
				if (!$mm->setSchema($owner->id, (int)$post_data['new']['schemaid']['assetid'], (int)$post_data['new']['granted'], (isset($post_data['new']['cascades']) ? 1 : 0))) {
					return FALSE;
				}
			}
		}
	}

	// revoke [deny] schema (apply or deny can be revoked)
	if (!empty($post_data['delete'])) {
		foreach ($post_data['delete'] as $schemaid => $on) {
			$schemaid = (int)$schemaid;
			$schema_info = $mm->getAssetSchemaInfo($owner->id, $schemaid);
			if ($cascade_changes && $schema_info['cascades']) {
				$current_access = $schema_info['granted'];
				$schema_vars[] = Array(
									'assetids'			=> Array($owner->id),
									'granted'			=> -1,
									'schemaid'			=> $schemaid,
									'cascades'			=> $cascade_changes,
									'previous_access'	=> $current_access,
								 );
			} else {
				if (!$mm->deleteSchema($owner->id, $schemaid)) {
					return FALSE;
				}
			}
		}
	}

	// Set cascades here
	$schemas = $mm->getAssetSchemaInfo($owner->id);
	foreach ($schemas as $schemaid => $schema_info) {
		// Prevent new schema from getting applied twice
		if (isset($post_data['new']['schemaid']['assetid']) && $post_data['new']['schemaid']['assetid'] == $schemaid) {
			continue;
		}

		$cascades = isset($post_data['cascades'][$schemaid]);
		if ((boolean)$cascades != (boolean)$schema_info['cascades']) {
			if (!$mm->setSchema($owner->id, (int)$schemaid, (int)$schema_info['granted'], (int)$cascades)) {
				return FALSE;
			}
		}
	}

	if (isset($_POST[$prefix.'_metadata']['manual_cascade']) && $_POST[$prefix.'_metadata']['manual_cascade']) {
		$schemas = $mm->getAssetSchemaInfo($owner->id, NULL, TRUE);
		foreach ($schemas as $schemaid => $schema_info) {
			$schema_vars[] = Array(
								'assetids'			=> Array($owner->id),
								'granted'			=> $schema_info['granted'],
								'cascades'			=> TRUE,
								'schemaid'			=> $schemaid,
								'previous_access'	=> NULL,
							 );
		}
	}


	if (!empty($schema_vars)) {
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array('schema_changes' => $schema_vars);
		$hh->queueHipo('hipo_job_edit_metadata_schemas', $vars);
	}

	return TRUE;

}//end processMetadataSchemas()


?>
