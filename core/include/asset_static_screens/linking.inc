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
* $Id: linking.inc,v 1.3.2.2 2004/03/04 16:40:10 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Linking Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/


/**
* Paints the interface for viewing links to/from this asset
*
* @param object	Asset					$owner		the asset whose interface we are painting
* @param object	Backend_Outputter		$o			the outputter class
* @param object	Asset_Edit_Interface	&$ei		the edit interface
* @param boolean						$allow_edit	allow editing of link types if user has access
*
* @return boolean
* @access public
*/
function paintLinking(&$owner, &$o, &$ei, $allow_edit=true)
{
	// If we are allowed to edit, we check the write effective access, minus live edit access
	// We are ignoring live edit access because changes to links are not reliant on
	// status of the asset (because the are not safe edited)
	$created = false;

	$write_access = ($allow_edit) ? $owner->writeAccess('links') :  false;

	// prefix for all the form vars
	$prefix = $owner->getPrefix();

	require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
	$o->openSection('Current Linking');
	$o->openField('&nbsp;');
	echo '<i>NOTE: <b>TYPE_1</b> links are displayed in both the frontend and backend navigation but <b>TYPE_2</b> links are only displayed in the backend navigation.</i><br/><br/>';

	$db = &$GLOBALS['SQ_SYSTEM']->db;
	$asset_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($owner->id, SQ_SC_LINK_ALL, '', true, 'minor');
	$editable_link_types = Array(SQ_LINK_TYPE_1 => link_type_name(SQ_LINK_TYPE_1),
								 SQ_LINK_TYPE_2 => link_type_name(SQ_LINK_TYPE_2)
								);

	$quoted_assetids = Array();
	foreach ($asset_links as $link) $quoted_assetids[] = $db->quote($link['majorid']);

	if (empty($quoted_assetids)) {
		$result = Array();
	} else {

		$concat = ($db->phptype == 'mysql') ? 'CONCAT(pt.treeid, '.$db->quote('%').')' : 'pt.treeid || '.$db->quote('%');
		$sql = 'SELECT ct.treeid as our_treeid, cl.minorid, pt.treeid as parent_treeid, a.name, a.assetid
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link cl
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree ct ON cl.linkid = ct.linkid,
					 '.SQ_TABLE_RUNNING_PREFIX.'asset_link pl
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree pt ON pl.linkid = pt.linkid
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset a ON a.assetid = pl.minorid
				';
		$where = 'cl.minorid IN ('.implode(', ', $quoted_assetids).')
				  AND ct.treeid LIKE '.$concat.'
				  AND pt.treeid <= ct.treeid';
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'cl');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ct');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'pl');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'pt');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

		$sql .= $where.'
				ORDER BY cl.linkid, ct.treeid, pt.treeid';


		$result = $db->getAssoc($sql, false, Array(), DB_FETCHMODE_ASSOC, true);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return Array();
		}
	}

	foreach ($asset_links as $link_data) $link_info[$link_data['majorid']][] = $link_data;

	$output_lineages = Array();
	$editing_linkids = Array();
	foreach ($result as $treeid => $link_tree) {

		// work out some values we are going to need
		$parent_data = $link_tree[count($link_tree) -1];
		$majorid = $parent_data['assetid'];

		foreach ($link_info[$majorid] as $link_data) {

			$link_type = $link_data['link_type'];
			$linkid = $link_data['linkid'];

			// work out if the user can change the link types
			// the arg $allow_edit must be set to TRUE to try the write access test
			$can_edit = ($write_access && in_array($link_type, array_keys($editable_link_types)) && !in_array($linkid, $editing_linkids));

			$asset_lineage = '';
			foreach ($link_tree as $tree_data) {
				if ($tree_data['assetid'] == $majorid) continue;
				$asset_lineage .= $tree_data['name'].' &gt; ';
			}
			$asset_lineage .= '<b>'.$parent_data['name'].'</b>';
			if (isset($output_lineages[$asset_lineage]) && !$can_edit) continue;

			ob_start();
			?>
			<tr>
				<td class="sq-backend-table-cell" style="text-align: center;">
					<?php echo $linkid; ?>
				</td>
				<td class="sq-backend-table-cell" style="text-align: center;">
					<?php
					if (!$can_edit) {
						echo link_type_name($link_type);
					} else {
						combo_box($prefix.'_link_type['.$linkid.']', $editable_link_types, false, $link_type);
						$editing_linkids[] = $linkid;
					}
					?>
				</td>
				<td class="sq-backend-table-cell"><?php echo $asset_lineage; ?></td>
			</tr>
			<?php
			$output_lineages[] = ob_get_contents();
			ob_end_clean();
		}

	}//end foreach result

	if (!count($output_lineages)) {
		echo 'None';
	} else {
		?>
		<table class="sq-backend-table">
		<tr>
			<td class="sq-backend-table-header" style="text-align: center; white-space: nowrap;">Link Id</td>
			<td class="sq-backend-table-header" style="text-align: center;">Link Type</td>
			<td class="sq-backend-table-header" width="100%">Lineage</td>
		</tr>
		<?php
		foreach ($output_lineages as $null => $lineage_data) echo $lineage_data;
		?>
		</table>
		<?php
	}// endif

	$o->closeSection();

	$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_lineage');
	$fl_jobs = &Cron_Job_Future_Lineage::getActiveJobs($owner);

	if (!empty($fl_jobs) || $write_access) {
		$created = true;
		$o->openSection('Future Linking');
		if (!empty($fl_jobs)) {
			$o->openField('', 'new_line');
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header">
						Action
					</td>
					<td class="sq-backend-table-header">
						When
					</td>
			<?php if ($write_access) {
				$created = true;
				?>
					<td class="sq-backend-table-header" style="text-align: center;">
						Delete ?
					</td>
			<?php } // end if ?>
				</tr>
			<?php

			for($i = 0; $i < count($fl_jobs); $i++) {
			?>
				<tr>
					<td class="sq-backend-table-cell">
						<?php echo nl2br($fl_jobs[$i]->readableDescription()); ?>
					</td>
					<td class="sq-backend-table-cell">
						<?php echo $fl_jobs[$i]->readableWhen(); ?>
					</td>
			<?php if ($write_access) {
				$created = true;
				?>
					<td class="sq-backend-table-cell" style="text-align: center;">
					<?php
						if ($fl_jobs[$i]->canDelete()) {
							check_box($prefix.'_remove_fl[]', $fl_jobs[$i]->id);
						} else {
							echo '&nbsp;';
						}// end if
					?>
					</td>
			<?php } // end if ?>
				</tr>
			<?php
			}// end foreach

			?>
			</table>
			<br/>
			<?php

		}// end if

		if ($write_access) {
			$created = true;
			$fl = new Cron_Job_Future_Lineage();
			$fl->setAssetInLink($owner, 'minor');
			$edit_fns = $fl->getEditFns();
			$edit_fns->paintInLineBackend($fl, $o, $prefix.'_new_fl');
		}// end if

		$o->closeSection();
	}// end if

	return $created;

}//end paintLinking()


/**
* Processes the interface for viewing links to/from this asset
*
* @param object	Asset					$owner		the asset whose interface we are painting
* @param object	Backend_Outputter		$o			the outputter class
* @param object	Asset_Edit_Interface	&$ei		the edit interface
*
* @return boolean
* @access public
*/
function processLinking(&$owner, &$o, &$ei)
{
	// if we dont have admin access, piss off
	if ($owner->id && !$owner->writeAccess('')) {
		$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to edit the links of <i>'.$owner->name.'</i>');
		exit();
	}

	// if we dont have effective write access, dont process anything
	if (!$owner->writeAccess('links')) return false;

	// prefix for all the form vars
	$prefix = $owner->getPrefix();

	$done_something = false;
	$asset_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($owner->id, SQ_SC_LINK_ALL, '', true, 'minor');

	foreach ($asset_links as $link) {
		if (!isset($_POST[$prefix.'_link_type'][$link['linkid']])) continue;
		$new_type = $_POST[$prefix.'_link_type'][$link['linkid']];
		if ($link['link_type'] == $new_type) continue;
		if (!$GLOBALS['SQ_SYSTEM']->am->_updateLink($link['linkid'], $new_type)) continue;
		$done_something = true;
	}

	$cron_mgr = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
	if (!is_null($cron_mgr)) {

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_lineage');

		if (!empty($_POST[$prefix.'_remove_fl'])) {
			$fl_jobs = &Cron_Job_Future_Lineage::getActiveJobs($owner);
			if (!empty($fl_jobs)) {

				if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($cron_mgr->id, 'links')) {
					for($i = 0; $i < count($fl_jobs); $i++) {
						if (in_array($fl_jobs[$i]->id, $_POST[$prefix.'_remove_fl'])) {
							if ($cron_mgr->removeJob($fl_jobs[$i])) {
								$is_updated = true;
							}
						}// end if
					}// end for

					$GLOBALS['SQ_SYSTEM']->am->releaseLock($cron_mgr->id, 'links');

				} else {
					trigger_error('Unable to acquire the lock on the Cron Manager, unable to remove jobs', E_USER_NOTICE);

				}// end if

			}// end if
		}// end if

		$fl = new Cron_Job_Future_Lineage();
		$edit_fns = $fl->getEditFns();
		if($fl->setAssetInLink($owner, 'minor') && $edit_fns->processInLineBackend($fl, $o, $prefix.'_new_fl')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			if ($cron_mgr->addJob($fl, $GLOBALS['SQ_SYSTEM']->user)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$done_something = true;
			} else {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			}

		}// end if

	}// end if

	return $done_something;

}//end processLinking()


?>
