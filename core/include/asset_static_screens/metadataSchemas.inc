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
* $Id: metadataSchemas.inc,v 1.20 2007/01/30 23:45:57 colivar Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* MetaData Schemas Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.20 $
* @package MySource_Matrix
*/


/**
* Paint the interface for viewing metadata schemas (used in popups etc)
*
* @param object	&$owner	the asset whose interface we are painting
* @param object	&$o		the outputter class
* @param object	&$ei	the edit interface
*
* @return boolean
* @access public
*/
function paintInlineMetadataSchemas(&$owner, &$o, &$ei)
{
	$prefix = $owner->getPrefix();

	$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();
	$schemas = $mm->getSchemas($owner->id);

	$direct = Array();
	foreach ($schemas as $schemaid => $granted) {
		if (!isset($direct[$granted])) {
			$direct[$granted] = Array();
		}
		$direct[$granted][] = $schemaid;
	}
	ksort($direct);

	if (empty($direct)) return FALSE;

	$o->openSection(translate('metadata_schemas'));
		$o->openField('&nbsp;');
		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header">
					<?php echo translate('metadata_schema'); ?>
				</td>
				<td class="sq-backend-table-header" align="right">
					<?php echo translate('access'); ?>
				</td>
			</tr>

			<?php
			foreach ($direct as $granted => $schemaids) {
				foreach ($schemaids as $schemaid) {
					if ($schemaid) {
						$schema =& $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
						if (!$schema->id || !is_a($schema, 'metadata_schema')) {
							continue;
						}
					}
				?>
					<tr>
						<td class="sq-backend-table-cell">
						<?php
						echo $schema->name.' ['.$schema->type().' : '.$schemaid.']';
						?>
						</td>
						<td class="sq-backend-table-cell" align="right">
							<span style="color: <?php echo (($granted == '0') ? 'red' : 'green'); ?>; font-weight: bold; "><?php echo (($granted == '0') ? translate('denied') : translate('applied')) ?></span>
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
* @param object	&$owner		the asset whose interface we are painting
* @param object	&$o				the outputter class
* @param object	&$ei			the edit interface
* @param array	$keywords	the keywords to replace
*
* @return boolean
* @access public
*/
function paintMetadataSchemas(&$owner, &$o, &$ei, $keywords=NULL)
{
	$prefix = $owner->getPrefix();

	$admin_access = $owner->adminAccess('metadata');

	$o->openSection(translate('metadata_schemas'));

	$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();
	$schemas = $mm->getSchemas($owner->id);

	$direct = Array();
	foreach ($schemas as $schemaid => $granted) {
		if (!isset($direct[$granted])) {
			$direct[$granted] = Array();
		}
		$direct[$granted][] = $schemaid;
	}
	ksort($direct);
	

	if (!empty($direct)) {
		$o->openField('&nbsp;');
		foreach ($direct as $granted => $schemaids) {
		?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header">
						<span style="color: <?php echo (($granted == '0') ? 'red' : 'green'); ?>"><?php echo (($granted == '0') ? translate('denied') : translate('applied')) ?></span>
					</td>
					<?php
					if ($admin_access) {
						?><td align="center" width="100" class="sq-backend-table-header"><?php echo translate('delete_question'); ?></td><?php
					}
					?>
				</tr>
			<?php
			foreach ($schemaids as $schemaid) {
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
			<br>
		<?php
		}//end foreach inherited

	}//end if

	if ($admin_access) {
		$o->openField(translate('new_question'), 'new_line');
			combo_box($prefix.'_metadata[new][granted]', Array('' => '', '1' => translate('apply'), '0' => translate('deny')), FALSE, '');
			echo ' '.translate('metadata_schema').' ';
			asset_finder($prefix.'_metadata[new][schemaid]', 0, Array('metadata_schema' => 'D'), 'sq_sidenav', FALSE, 'null', Array('clear'));
		$o->closeField();
	}

	$o->closeSection();

	if ($admin_access) {
		$kiddies = $GLOBALS['SQ_SYSTEM']->am->getChildren($owner->id);
		if (count($kiddies)) {
			$o->openSection(translate('cascade_options'));
				$o->openField('Note');
					echo translate('action_affects_assets_below_this', count($kiddies), $owner->name);
				$o->closeField();
				$o->openField('&nbsp;');
					check_box($prefix.'_metadata[cascade_changes]', '1', TRUE);
					label(translate('cascade_schema_changes'), $prefix.'_metadata[cascade_changes]');
					echo '<br/>';
					check_box($prefix.'_metadata[manual_cascade]');
					label(translate('manually_cascade_schema'), $prefix.'_metadata[manual_cascade]');
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
* @param object	&$owner	the asset whose interface we are painting
* @param object	&$o		the outputter class
* @param object	&$ei	the edit interface
*
* @return boolean
* @access public
*/
function processMetadataSchemas(&$owner, &$o, &$ei)
{
	// if we dont have admin access, go away
	if ($owner->id && !$owner->adminAccess('')) {
		$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset', $owner->name));
		exit();
	}

	// if we dont have admin access, dont process anything
	if (!$owner->adminAccess('metadata')) return FALSE;

	$prefix = $owner->getPrefix();
	if (!isset($_POST[$prefix.'_metadata'])) return FALSE;

	$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();

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
			trigger_localised_error('SYS0268', E_USER_NOTICE, $post_data['new']['schemaid']['assetid']);
		} else {
			if ($cascade_changes) {
				// cascading access of some kind, so queue it up for the HIPO
				$schema_vars[] = Array(
									'granted'			=> (int)$post_data['new']['granted'],
									'schemaid'			=> (int)$post_data['new']['schemaid']['assetid'],
									'previous_access'	=> NULL,
								 );
			} else {
				// lets do this ourselves
				if (!$mm->setSchema($owner->id, (int)$post_data['new']['schemaid']['assetid'], (int)$post_data['new']['granted'])) {
					return FALSE;
				}
			}
		}
	}

	// revoke [deny] schema (apply or deny can be revoked)
	if (!empty($post_data['delete'])) {
		foreach ($post_data['delete'] as $schemaid => $on) {
			$schemaid = (int)$schemaid;
			if ($cascade_changes) {
				$schemas = $mm->getSchemas($owner->id);
				$current_access = $schemas[$schemaid];
				$schema_vars[] = Array(
									'granted'			=> -1,
									'schemaid'			=> $schemaid,
									'previous_access'	=> $current_access,
								 );
			} else {
				if (!$mm->deleteSchema($owner->id, $schemaid)) {
					return FALSE;
				}
			}
		}
	}

	if (isset($_POST[$prefix.'_metadata']['manual_cascade']) && $_POST[$prefix.'_metadata']['manual_cascade']) {
		$schemas = $mm->getSchemas($owner->id);
		foreach ($schemas as $schemaid => $granted) {
			$schema_vars[] = Array(
								'granted'			=> $granted,
								'schemaid'			=> $schemaid,
								'previous_access'	=> NULL,
							 );
		}
	}


	if (!empty($schema_vars)) {
		$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array('assets' => Array($owner->id => Array('type_code' => $owner->type())), 'schema_changes' => $schema_vars);
		$hh->queueHipo('hipo_job_edit_metadata_schemas', $vars);
	}

	return TRUE;

}//end processMetadataSchemas()


?>