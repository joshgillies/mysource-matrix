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
* $Id: metadataSchemas.inc,v 1.3 2004/01/16 11:45:56 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* MetaData Schemas Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/


/**
* Paint the interface for viewing metadata schemas (used in popups etc)
*
* @param object	Asset					$owner		the asset whose interface we are painting
* @param object	Backend_Outputter		$o			the outputter class
* @param object	Asset_Edit_Interface	&$ei		the edit interface
*
* @access public
* @return boolean
*/
function paintInlineMetadataSchemas(&$owner, &$o, &$ei)
{
	$prefix = $owner->getPrefix();

	$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
	$schemas = $mm->getSchemas($owner->id);

	$direct = Array();
	foreach($schemas as $schemaid => $access) {
		if (!isset($direct[$access])) $direct[$access] = Array();
		$direct[$access][] = $schemaid;
	}
	ksort($direct);

	if (empty($direct)) return false;

	$o->openSection('Metadata Schemas');
		$o->openField('&nbsp;');
		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header">
					Metadata Schema
				</td>
				<td class="sq-backend-table-header" align="right">
					Access
				</td>
			</tr>

			<?php
			foreach ($direct as $access => $schemaids) {
				foreach($schemaids as $schemaid) {
					if ($schemaid) {
						$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
						if (!$schema->id || !is_a($schema, 'metadata_schema')) continue;
					}
				?>
					<tr>
						<td class="sq-backend-table-cell">
						<?php
						echo $schema->name.' ['.$schema->type().' : '.$schemaid.']';
						?>
						</td>
						<td class="sq-backend-table-cell" align="right">
							<span style="color: <?php echo (($access == '0') ? 'red' : 'green'); ?>; font-weight: bold; "><?php echo (($access == '0') ? 'Denied' : 'Applied') ?></span>
						</td>
					</tr>
				<?php
				}
			}

		?></table><?php
		$o->closeField();
	$o->closeSection();

	return true;

}//end paintInlineMetadataSchemas()


/**
* Paint the interface for managing metadata schamas
*
* @param object	Asset					$owner		the asset whose interface we are painting
* @param object	Backend_Outputter		$o			the outputter class
* @param object	Asset_Edit_Interface	&$ei		the edit interface
*
* @access public
* @return boolean
*/
function paintMetadataSchemas(&$owner, &$o, &$ei)
{
	$prefix = $owner->getPrefix();

	$admin_access = $owner->adminAccess('metadata');

	$o->openSection('Metadata Schemas');

	$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
	$schemas = $mm->getSchemas($owner->id);

	$direct = Array();
	foreach($schemas as $schemaid => $access) {
		if (!isset($direct[$access])) $direct[$access] = Array();
		$direct[$access][] = $schemaid;
	}
	ksort($direct);

	if (!empty($direct)) {
		$o->openField('&nbsp;');
		foreach ($direct as $access => $schemaids) {
		?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header">
						<span style="color: <?php echo (($access == '0') ? 'red' : 'green'); ?>"><?php echo (($access == '0') ? 'Denied' : 'Applied') ?></span>
					</td>
					<?php
					if ($admin_access) {
						?><td align="center" width="100" class="sq-backend-table-header">Delete ?</td><?php
					}
					?>
				</tr>
			<?php
			foreach($schemaids as $schemaid) {
				if ($schemaid) {
					$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
					if (!$schema->id || !is_a($schema, 'metadata_schema')) continue;
				}
			?>
				<tr>
					<td class="sq-backend-table-cell">
					<?php
					echo $schema->name.' ['.$schema->type().' : '.$schemaid.']';
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

	}// end if

	if ($admin_access) {
		$o->openField('New ?', 'new_line');
			combo_box($prefix.'_metadata[new][access]', Array('' => '', '1' => 'Apply', '0' => 'Deny'), false, '');
			echo ' Metadata schema ';
			asset_finder($prefix.'_metadata[new][schemaid]', 0, Array('metadata_schema' => 'D'));
		$o->closeField();
	}

	$o->closeSection();

	if ($admin_access) {
		$kiddies = $GLOBALS['SQ_SYSTEM']->am->getChildren($owner->id);
		if (count($kiddies)) {
			$o->openSection('Cascade Options');
				$o->openField('Note');
					echo 'These actions will affect '.count($kiddies).' other assets below "'.$owner->name.'"';
				$o->closeField();
				$o->openField('&nbsp;');
					check_box($prefix.'_metadata[cascade_changes]', '1', true);
					echo ' Cascade schema changes';
					echo '<br/>';
					check_box($prefix.'_metadata[manual_cascade]');
					echo ' Manually cascade schemas';
				$o->closeField();
			$o->closeSection();
		}
	}

	return $admin_access;

}//end paintMetadataSchemas()


/**
* Process the interface for managing metadata schemas
*
* @param object	Asset					$owner	the asset whose interface we are painting
* @param object	Backend_Outputter		$o		the outputter class
* @param object	Asset_Edit_Interface	&$ei	the edit interface
*
* @access public
* @return boolean TRUE is there is a change, false on error or no change
*/
function processMetadataSchemas(&$owner, &$o, &$ei)
{
	// if we dont have admin access, piss off
	if ($owner->id && !$owner->adminAccess('')) {
		$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to edit the metadata schemas for <i>'.$owner->name.'</i>');
		exit();
	}

	// if we dont have admin access, dont process anything
	if (!$owner->adminAccess('metadata')) return false;

	$prefix = $owner->getPrefix();
	if (!isset($_POST[$prefix.'_metadata'])) return false;

	$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();

	// are we cascading these schema changes to our children?
	$cascade_changes = false;
	if (isset($_POST[$prefix.'_metadata']['cascade_changes']) && $_POST[$prefix.'_metadata']['cascade_changes']) {
		$cascade_changes = true;
	}

	$schema_vars = Array();
	$post_data = $_POST[$prefix.'_metadata'];

	// apply a schema
	if (!empty($post_data['new']['schemaid']['assetid'])) {
		if (!isset($post_data['new']['access']) || $post_data['new']['access'] == '') {
			trigger_error('You need to select the type of access in order to apply schema #'.$post_data['new']['schemaid']['assetid'], E_USER_NOTICE);
		} else {
			if ($cascade_changes) {
				// cascading access of some kind, so queue it up for the HIPO
				$schema_vars[] = Array('access'          => (int)$post_data['new']['access'],
									   'schemaid'        => (int)$post_data['new']['schemaid']['assetid'],
									   'previous_access' => null,
									   );
			} else {
				// lets do this ourselves
				if (!$mm->setSchema($owner->id, (int)$post_data['new']['schemaid']['assetid'], (int)$post_data['new']['access'])) return false;
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
				$schema_vars[] = Array('access'          => -1,
									   'schemaid'        => $schemaid,
									   'previous_access' => $current_access,
									   );
			} else {
				if (!$mm->deleteSchema($owner->id, $schemaid)) return false;
			}
		}
	}

	if (isset($_POST[$prefix.'_metadata']['manual_cascade']) && $_POST[$prefix.'_metadata']['manual_cascade']) {
		$schemas = $mm->getSchemas($owner->id);
		foreach ($schemas as $schemaid => $access) {
			$schema_vars[] = Array('access'          => $access,
								   'schemaid'        => $schemaid,
								   'previous_access' => null,
								   );
		}
	}


	if (!empty($schema_vars)) {
		$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array('assetid' => $owner->id, 'schema_changes' => $schema_vars);
		$hh->queueHipo('hipo_job_edit_metadata_schemas', $vars);
	}

	return true;

}//end processMetadataSchemas()


?>